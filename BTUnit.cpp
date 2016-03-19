//=============================================================
//
// Blood Tide, Version 1.0.0
// Copyright 2009-2010, by Team Blackfire
//
// This file is part of the game Blood Tide. It uses the 
// C4 Engine from Terathon Software LLC. Their license
// can be viewed at:
//
// http://www.terathon.com/c4engine/license.php
//
// Our license agreement, however, is not so kind.
// You are free to use and modify this code, but you 
// cannot sell it and you must give us credit if you use it.
// Violators will be fed to the Gorthax.
//
//=============================================================


#include "BTUnit.h"
#include "BTSteering.h"
#include "StringTableManager.h"
#include "BTCacheManager.h"
#include "BTGoalAttack.h"
#include "BTGoalMove.h"
#include "MGGame.h"
#include "BTMath.h"
#include "BTGoalDefend.h"
#include "BTGoalAttackTo.h"
#include "BTGoalFollow.h"
#include "BTCommanderUI.h"

#define UNIT_INTERP_WAIT 200

using namespace C4;


UnitController::UnitController() : 
	BTCharacterController( kControllerUnit ),
	m_objSteering( this ),
	m_ulLastUpdate( 0 )
{
	Initialize( Point3D(0,0,0), 0.0F, 0.0F );
	SetBaseControllerType( kControllerEntity );
}

UnitController::UnitController(const Point3D& position, unsigned long ulStringID, TableID lTableID, StatsProtocol *pStats, float fRadius, float fHeight, float fFloatHeight, long lTeam, float fOriMult) : 
	BTCharacterController( kControllerUnit, kEntityUnit, ulStringID, lTableID, kCharacterGravity, pStats, fFloatHeight, lTeam, fOriMult ),
	m_objSteering( this ),
	m_ulLastUpdate( 0 )
{
	Initialize( position, fRadius, fHeight );
	SetBaseControllerType( kControllerEntity );
}

UnitController::~UnitController()
{
	// Clear UI
	if( TheInfoPaneMgr ) {

		TheInfoPaneMgr->GetCurrentPane().HideIfSelected( GetControllerIndex() );
	}

	// Remove from cache
	Array<long> *unitCache = TheGameCacheMgr->GetGameCache(kCacheUnits, m_lTeam);

	if( unitCache ) {
		long ix = unitCache->FindElement( GetControllerIndex() );
		if( ix >= 0 ) unitCache->RemoveElement( ix );
	}

	// Cleanup AI
	m_objBaseGoal.Exit();

	// Cleanup networking
	if( TheMessageMgr->Server() ){
		TheMessageMgr->RemoveSnapshotSender( this );
	}

	// Cleanup abilities
	if( m_pAbility1 ) {

		delete m_pAbility1;
		m_pAbility1 = nullptr;
	}
}

void UnitController::Initialize( const Point3D& position, float fRadius, float fHeight )
{
	SetCollider( new BTCylinderCollider( position, this, fRadius, fHeight ) );
}

void UnitController::Pack(Packer& data, unsigned long packFlags) const
{
	BTCharacterController::Pack(data, packFlags);
}

void UnitController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	BTCharacterController::Unpack(data, unpackFlags);
}


void UnitController::Preprocess(void)
{
	BTCharacterController::Preprocess();
	SetAnimation( kMotionNone );
	m_objBaseGoal.Enter();

	if( TheMessageMgr->Server() ){
		TheMessageMgr->AddSnapshotSender(this);
	}

	m_pAbility1 = Ability::Construct( this, kStringTableUnits, m_ulStringID, 'abi1' );

	if( TheMessageMgr->Server() ) {
		m_objSteering.SetMoveWithoutHeading( true );
	}
}

void UnitController::Move(void)
{
	if( GetEntityFlags() & kEntityDead ) {
		if( TheTimeMgr->GetAbsoluteTime() >= GetKillTime() ) {
			BTEntityController::Kill();
			return;
		}

		Model *unit = static_cast<Model*>( GetTargetNode() );
		unit->Animate();
		Travel();
		return;
	}

	// If the unit is below zero health, it is dead
	StatsProtocol *stats = GetStats();
	if( stats && stats->GetHealth() <= 0.0F )
	{
		Kill();
		return;
	}

	// Update all abilities with the current time (abilities may need to do extra processing if they happen over a duration).
	if( m_pAbility1 ) m_pAbility1->Update( TheTimeMgr->GetAbsoluteTime() );

	// If the unit is stunned, don't perform AI or steering
	if( GetEntityFlags() & kEntityStunned )
	{
		if( GetAnimation() == kMotionMove ) {

			EndMovement( GetAzimuth() );
		}

		BTEntityController::Move();
		BTEntityController::PostTravel();

		return;
	}

	float dt = TheTimeMgr->GetFloatDeltaTime();

	// Only use steering when at "ground" level and not stunned
	if ( GetCharFlags() & kCharacterGround ) {

		// If it has been too long since the last update, stop interpolating positions.
		if( !TheMessageMgr->Server() && TheTimeMgr->GetAbsoluteTime() > m_ulLastUpdate + UNIT_INTERP_WAIT && m_objSteering.GetInterpOn() ) {
			m_objSteering.SetInterpOn( false );
			SetAnimation( kMotionNone );
		}

		if( TheMessageMgr->Server() ) {
			if( !m_objBaseGoal.HasSubgoals() ) {
				AddGoal( new BTGoalDefend(this, GetPosition()) );
			}

			// process AI
			m_objBaseGoal.Process();
		}

		if( stats ) {
			GetSteering()->SetSpeed( stats->GetMovementSpeed() );
		}

		long prevAnim = GetAnimation();

		Vector3D newVelocity(0,0,0);
		float unitAzimuth = GetAzimuth();
		bool bMoved = GetSteering()->CalculateMove( GetPosition(), unitAzimuth, newVelocity );

		long newAnim = GetAnimation();

		if( TheMessageMgr->Server() && bMoved ) {

			if( Magnitude2D(newVelocity) == 0 ) {
				if( newAnim == kMotionWaiting ) {
					EndMovement( m_fAzimuth );
				}
				else {
					ChangeMovement( m_fAzimuth );
				}
			}

			if( (prevAnim == kMotionNone || prevAnim == kMotionWaiting) && newAnim != kMotionNone && newAnim != kMotionWaiting ) {
				BeginMovement( m_fAzimuth );
			}
		}

		GetImpulse() = ( newVelocity * (dt/1000.0F) );
		SetAzimuth( unitAzimuth );

		if( TheMessageMgr->Server() && !bMoved && prevAnim != kMotionNone ) {

			if( prevAnim != kMotionAbility1 )
				EndMovement( m_fAzimuth );
		}

		if( bMoved && (prevAnim == kMotionNone) )
		{
			SetAnimation( kMotionMove );
		}
	}

	BTCharacterController::Move();

	Model *unit = static_cast<Model*>( GetTargetNode() );
	unit->Animate();

	Travel();
}

void UnitController::Travel(void)
{
	BTCharacterController::Travel();

	BTEntityController::PostTravel();
}

void UnitController::Kill( void )
{
	TheGame->AwardExperience( *this );

	long index = GetControllerIndex();

	if( TheBTCursor )
		TheBTCursor->RemoveSelectedUnit( index );

	if( TheUnitIconMgr )
		TheUnitIconMgr->RemoveUnit( index );

	BTCharacterController::Kill();
}

void UnitController::PerformAbility( long lAbilityNumber )
{
	// If the unit is stunned, don't let it perform abilities
	if( GetEntityFlags() & kEntityStunned )
	{
		return;
	}

	Ability *ability = nullptr;

	if( lAbilityNumber == kMotionAbility1 ) {
		ability = m_pAbility1;
	}

	if( ability ) {

		if( ability->Execute() == true ) {

			BTCharacterController::PerformAbility( lAbilityNumber );

			GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
			if( world ) {
				TheMessageMgr->SendMessageAll( WorldSoundMessage(ability->GetSound(), m_pCollider->GetPosition(), 16.0F) );
			}
		}
	}
}


void UnitController::SetAnimation( EntityMotion motion )
{
	if( GetAnimation() == kMotionAbility1 )
	{
		if( motion != kMotionMove )
			BTCharacterController::SetAnimation( motion );

		return;
	}

	BTCharacterController::SetAnimation( motion );
}

void UnitController::AddGoal( BTGoal *pGoal )
{
	if( !TheMessageMgr->Server() ) {
		return;
	}

	m_objBaseGoal.AddSubgoal( pGoal );
}

void UnitController::ClearGoals( void )
{
	m_objBaseGoal.RemoveAllSubgoals();
}

ControllerMessage *UnitController::ConstructMessage(ControllerMessageType type) const
{
	switch (type)
	{
		case kUnitMessageBeginMovement:
		case kUnitMessageEndMovement:
		case kUnitMessageChangeMovement:

			return (new UnitMovementMessage(type, GetControllerIndex()));

	}

	return (BTCharacterController::ConstructMessage(type));
}

void UnitController::ReceiveMessage(const ControllerMessage *message)
{
	switch (message->GetControllerMessageType())
	{
		case kEntityMessageUpdatePosition:
		{
			const EntityUpdatePositionMessage *m = static_cast<const EntityUpdatePositionMessage *>(message);
			GetCollider()->GetPosition() = m->GetUpdatePosition();

			m_ulLastUpdate = TheTimeMgr->GetAbsoluteTime();

			if( !TheMessageMgr->Server() ) {
				m_objSteering.SetInterpOn( true );
				SetAnimation( kMotionMove );
			}

			break;
		}

		case kUnitMessageBeginMovement:
		{
			const UnitMovementMessage *m = static_cast<const UnitMovementMessage *>(message);

			GetPosition() = m->GetPosition();

			if( !TheMessageMgr->Server() ) {
				m_objSteering.SetInterpOn( true );
			}

			SetAnimation( kMotionMove );

			break;
		}

		case kUnitMessageEndMovement:
		{
			const UnitMovementMessage *m = static_cast<const UnitMovementMessage *>(message);

			GetPosition() = m->GetPosition();
			GetVelocity() = Vector3D(0,0,0);

			if( !TheMessageMgr->Server() ) {
				m_objSteering.SetInterpOn( false );
			}

			SetAnimation( kMotionNone );

			break;
		}

		case kUnitMessageChangeMovement:
		{
			const UnitMovementMessage *m = static_cast<const UnitMovementMessage *>(message);

			GetPosition() = m->GetPosition();
			m_fAzimuth = m->GetMovementAzimuth();

			SetAnimation( kMotionMove );

			break;
		}

		case kEntityMessagePlayAnimation:
		{
			const EntityPlayAnimationMessage *m = static_cast<const EntityPlayAnimationMessage *>( message );

			long motion = m->GetMotion();

			// Player(s) on the other team should not be able to see this units's cooldowns
			GamePlayer *localPlayer = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
			if( localPlayer )
			{
				if( localPlayer->GetPlayerTeam() == GetTeam() )
				{
					if( m_pAbility1 ) m_pAbility1->HandleEvent( kAbilityEventTriggered, motion, nullptr );
				}
			}

			BTCharacterController::ReceiveMessage(message);
			break;
		}

		default:

			BTCharacterController::ReceiveMessage(message);
			break;
	}
}

// Send an update to clients.
void UnitController::SendSnapshot(void)
{
	const Point3D& position = GetPosition();
	Vector3D velocity = GetVelocity();

	// Only send an update if something is different.
	if( m_fOldAzimuth != m_fAzimuth ||
		m_p3dOldPosition != position ||
		m_v3dOldVelocity != velocity )
	{
		TheMessageMgr->SendMessageClients(EntityUpdatePositionMessage(GetControllerIndex(), position));
		TheMessageMgr->SendMessageClients(EntityUpdateAzimuthMessage(GetControllerIndex(), m_fAzimuth));
	}

	// Keep track of old information.
	m_fOldAzimuth = m_fAzimuth;
	m_p3dOldPosition = position;
	m_v3dOldVelocity = velocity;
}

void UnitController::BeginMovement( float fAzm )
{
	const Point3D& position = GetPosition();

	UnitMovementMessage message(kUnitMessageBeginMovement, GetControllerIndex(), position, fAzm);
	TheMessageMgr->SendMessageAll(message);
}

void UnitController::EndMovement( float fAzm )
{
	const Point3D& position = GetPosition();

	UnitMovementMessage message(kUnitMessageEndMovement, GetControllerIndex(), position, fAzm);
	TheMessageMgr->SendMessageAll(message);
}

void UnitController::ChangeMovement( float fAzm )
{
	const Point3D& position = GetPosition();

	UnitMovementMessage message(kUnitMessageChangeMovement, GetControllerIndex(), position, fAzm);
	TheMessageMgr->SendMessageAll(message);
}

void UnitController::InitializeUnitInfoPane( UnitInfoPane& pane )
{
	AbilityBar *abilityBar = pane.GetAbilityBar();

	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );

	bool unlocked = false;

	if( player->GetPlayerTeam() == GetTeam() ) {

		unlocked = true;
	}

	abilityBar->SetAbility( 0, m_pAbility1, kStringTableCommanders, m_ulStringID, 'abi1', 1, false );
	abilityBar->UnlockAbility( 0, unlocked );

	pane.GetHealthBar()->SetStatsController( GetStats() );
}

/* ---- Create Unit Message ---- */

CreateUnitMessage::CreateUnitMessage() : 
	CreateModelMessage(kModelMessageUnit)
{
}

CreateUnitMessage::CreateUnitMessage(ModelMessageType type, long index, unsigned long ulEntityID, const Point3D& position, float azm, float alt, const Vector3D& ground, const Vector3D& slide1, const Vector3D& slide2, unsigned long character, unsigned long movement, long lTeam, unsigned long ulBaseIndex) : 
	CreateModelMessage(type, index, position)
{
	m_ulEntityID = ulEntityID;

	m_fInitialAzimuth = azm;
	m_fInitialAltitude = alt;

	m_v3dGroundNormal = ground;
	m_v3dSlideNormal[0] = slide1;
	m_v3dSlideNormal[1] = slide2;

	m_lCharacterFlags = character;
	m_lMovementFlags = movement;

	m_lTeam = lTeam;

	m_ulBaseIndex = ulBaseIndex;
}

CreateUnitMessage::~CreateUnitMessage()
{
}

void CreateUnitMessage::Compress(Compressor& data) const
{
	CreateModelMessage::Compress(data);

	data << m_ulEntityID;

	data << m_fInitialAzimuth;
	data << m_fInitialAltitude;

	data << m_v3dGroundNormal.x;
	data << m_v3dGroundNormal.y;
	data << m_v3dGroundNormal.z;

	data << m_v3dSlideNormal[0].x;
	data << m_v3dSlideNormal[0].y;
	data << m_v3dSlideNormal[0].z;

	data << m_v3dSlideNormal[1].x;
	data << m_v3dSlideNormal[1].y;
	data << m_v3dSlideNormal[1].z;

	data << m_lCharacterFlags;
	data << (unsigned char) m_lMovementFlags;

	data << m_lTeam;

	data << m_ulBaseIndex;
}

bool CreateUnitMessage::Decompress(Decompressor& data)
{
	if(CreateModelMessage::Decompress(data)) {
		unsigned char	movement;

		data >> m_ulEntityID;

		data >> m_fInitialAzimuth;
		data >> m_fInitialAltitude;

		data >> m_v3dGroundNormal.x;
		data >> m_v3dGroundNormal.y;
		data >> m_v3dGroundNormal.z;

		data >> m_v3dSlideNormal[0].x;
		data >> m_v3dSlideNormal[0].y;
		data >> m_v3dSlideNormal[0].z;

		data >> m_v3dSlideNormal[1].x;
		data >> m_v3dSlideNormal[1].y;
		data >> m_v3dSlideNormal[1].z;

		data >> m_lCharacterFlags;

		data >> movement;
		m_lMovementFlags = movement;

		data >> m_lTeam;

		data >> m_ulBaseIndex;

		return (true);
	}

	return (false);
}

/*-- Unit Movement Message --*/

UnitMovementMessage::UnitMovementMessage(ControllerMessageType type, long controllerIndex) : ControllerMessage(type, controllerIndex)
{
}

UnitMovementMessage::UnitMovementMessage(ControllerMessageType type, long controllerIndex, const Point3D& position, float azimuth) : ControllerMessage(type, controllerIndex)
{
	m_p3dPosition = position;
	m_fAzimuth = azimuth;
}

UnitMovementMessage::~UnitMovementMessage()
{
}

void UnitMovementMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress(data);

	data << m_fAzimuth;
}

bool UnitMovementMessage::Decompress(Decompressor& data)
{
	if (ControllerMessage::Decompress(data))
	{
		data >> m_fAzimuth;

		return (true);
	}

	return (false);
}

/* ---- Unit Command Message ---- */

UnitCommandMessage::UnitCommandMessage( ) : 
	Message( kMessageUnitCommand )
{

}

UnitCommandMessage::UnitCommandMessage( long lCommandType, Array<UnitCommand> &arrCommands) : 
	Message( kMessageUnitCommand ),
	m_lCommandType( lCommandType )
{
	long size = arrCommands.GetElementCount();
	for( int x = 0; x < size; x++ ) {
		m_arrCommands.AddElement( arrCommands[x] );
	}
}

UnitCommandMessage::~UnitCommandMessage()
{
}

void UnitCommandMessage::Compress( Compressor& data ) const
{
	data << m_lCommandType;

	long numElements;
	long i;

	numElements = m_arrCommands.GetElementCount();
	data << numElements;

	for( i = 0; i < numElements; i++ ) {
		UnitCommand command = m_arrCommands[i];

		data << command.m_lUnit;

		switch( m_lCommandType ) {

			case kCommandMove:
			case kCommandAttackTo:
				
				data << command.m_p3dTarget.x;
				data << command.m_p3dTarget.y;
				data << command.m_p3dTarget.z;
				break;

			case kCommandAttack:
			case kCommandFollow:

				data << command.m_lTarget;
				break;


			default:
				break;
		}
	}
}

bool UnitCommandMessage::Decompress( Decompressor& data )
{
	data >> m_lCommandType;

	long numElements;
	long i;

	data >> numElements;

	for( i = 0; i < numElements; i++ ) {
		UnitCommand command;
		data >> command.m_lUnit;
		
		switch( m_lCommandType ) {

			case kCommandMove:
			case kCommandAttackTo:

				float x, y, z;
				data >> x;
				data >> y;
				data >> z;
				command.m_p3dTarget = Point3D( x, y, z );
				break;

			case kCommandAttack:
			case kCommandFollow:

				data >> command.m_lTarget;
				break;

			default:
				break;
		}

		m_arrCommands.AddElement( command );
	}

	return (true);
}

bool UnitCommandMessage::HandleMessage( Player *sender ) const
{
	if( TheMessageMgr->Server() ) {

		long size = m_arrCommands.GetElementCount();
		for( long  i= 0; i < size; i++ ) {

			UnitCommand command = m_arrCommands[i];

			UnitController *controller = static_cast<UnitController*>(TheWorldMgr->GetWorld()->GetController( command.m_lUnit ));
			if( controller && controller->GetControllerType() == kControllerUnit ) {

				BTCharacterController *pTarget = static_cast<BTCharacterController*>(TheWorldMgr->GetWorld()->GetController( command.m_lTarget ));
				Point3D p3dTarget = command.m_p3dTarget;

				switch( m_lCommandType ) {
					case kCommandAttack:

						if( pTarget && controller->CanAttack(pTarget) ) {
							controller->Wake();
							controller->ClearGoals();
							controller->AddGoal( new BTGoalAttack(pTarget, controller) );
						}

						break;

					case kCommandFollow:

						if( pTarget && !controller->CanAttack(pTarget) ) {
							controller->Wake();
							controller->ClearGoals();
							controller->AddGoal( new BTGoalFollow(controller, pTarget) );
						}

						break;

					case kCommandMove:

						controller->Wake();
						controller->ClearGoals();
						controller->AddGoal( new BTGoalMove(p3dTarget,controller) );

						break;

					case kCommandAttackTo:

						controller->Wake();
						controller->ClearGoals();
						controller->AddGoal( new BTGoalAttackTo(controller,p3dTarget) );

						break;

					case kCommandStop:

						controller->Wake();
						controller->ClearGoals();

						break;

					default:
						break;
				}
			}
		}

	}

	return (true);
}

