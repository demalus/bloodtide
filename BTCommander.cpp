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
// This file was based upon MGFighter.cpp
//=============================================================

#include "BTCommander.h"
#include "BTCommanderUI.h"
#include "BTMath.h"
#include "C4FlashController.h"
#include "BTInfoPane.h"
#include "MGGame.h"

using namespace C4;

#define COMMANDER_SELECTER_RANGE 10.0F

// Crosshair assistance
#define CROSS_ASSIST_RADIUS 0.1F
#define CROSS_ASSIST_DIST 25.0F
#define CROSS_ASSIST_COLOR ColorRGBA(0.6F,0.6F,0.6F,0.1F)

namespace
{
	const float kCommanderRadius			= 0.33F;
	const float kCommanderHeight			= 1.8F;
}


CommanderInteractor::CommanderInteractor(CommanderController *controller)
{
	m_pCommanderController = controller;
}

CommanderInteractor::~CommanderInteractor()
{
}

void CommanderInteractor::HandleInteractionEvent(InteractionEventType type, Node *node, const Point3D *position)
{
	// Only interact with things if not firing a weapon.
	if ((m_pCommanderController->GetCommanderFlags() & kCommanderFiring) == 0)
	{
		// Always call the base class counterpart.
		Interactor::HandleInteractionEvent(type, node, position);
		
		switch (type)
		{
			case kInteractionEventEngage:
				
				// This event is received when the player engages an interactive object.
				
				if (TheMessageMgr->Server())
				{
					Controller *controller = node->GetController();
					if (controller)
					{
						// Send a message to the player's machine indicating that interaction has begun.
						
						GamePlayer *player = m_pCommanderController->GetCommanderPlayer();
						player->SendMessage(CommanderBeginInteractionMessage(m_pCommanderController->GetControllerIndex(), controller->GetControllerIndex(), *position));
					}
				}
				
				break;
			
			case kInteractionEventDisengage:
				
				// This event is received when the player disengages an interactive object.
				
				if (TheMessageMgr->Server())
				{
					Controller *controller = node->GetController();
					if (controller)
					{
						// Send a message to the player's machine indicating that interaction has ended.
						
						GamePlayer *player = m_pCommanderController->GetCommanderPlayer();
						player->SendMessage(CommanderInteractionMessage(CommanderController::kCommanderMessageDisengageInteraction, m_pCommanderController->GetControllerIndex(), controller->GetControllerIndex()));
					}
				}
				
				break;
			
			case kInteractionEventTrack:
				
				// This event is received while the player is engaged with interactive object.
				
				if (m_pCommanderController->GetCommanderPlayer() == TheMessageMgr->GetLocalPlayer())
				{
					// Only inform the interaction target on the client machine.
					
					Controller *controller = node->GetController();
					if (controller) controller->HandleInteractionEvent(kInteractionEventTrack, position);
				}
				
				break;
		}
	}
}


CommanderController::CommanderController() :
	BTCharacterController( kControllerCommander ),
	m_objCommanderInteractor( this ),
	m_objCrossAssist( CROSS_ASSIST_RADIUS, CROSS_ASSIST_DIST, CROSS_ASSIST_COLOR, nullptr)
{
	Initialize( Point3D(0,0,0), 0.0F, 0.0F );
	SetBaseControllerType( kControllerEntity );
}

CommanderController::CommanderController(const Point3D& position, unsigned long ulStringID, TableID lTableID, StatsProtocol *pStats, float fRadius, float fHeight, float fFloatHeight, long lTeam, float fOriMult, long startKills) :
	BTCharacterController( kControllerCommander, kEntityCommander, ulStringID, lTableID, kCharacterGravity, pStats, fFloatHeight, lTeam, fOriMult ),
	m_objCommanderInteractor( this ),
	m_objCrossAssist( CROSS_ASSIST_RADIUS, CROSS_ASSIST_DIST, CROSS_ASSIST_COLOR, nullptr)
{
	commanderFlags = 0;
	
	primaryAzimuth = 0.0F;
	lookAzimuth = 0.0F;
	lookAltitude = 0.0F;
	
	Initialize( position, fRadius, fHeight );
	SetBaseControllerType( kControllerEntity );

	m_lKillCount = startKills;
}

CommanderController::~CommanderController()
{
	// Display message for player who lost commander.
	GamePlayer *localPlayer = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( localPlayer && (GetTeam() == localPlayer->GetPlayerTeam()) ) 
	{
		GameWorld *world = static_cast<GameWorld *>( TheWorldMgr->GetWorld() );
		if( world )
		{
			TheGame->HelpMessage( 'code' );

			if( (world->GetInFieldMode()) || ((world->GetInFieldMode() || world->GetInCommandMode()) == false) )
			{
				TheGame->ToCommandMode();
			} 
			else
			{
				if( TheInfoPaneMgr ) {

					TheInfoPaneMgr->GetCurrentPane().HideIfSelected( GetControllerIndex() );
				}
			}
		}
	}

	// Get the player who is controlling this commander and reset his player controller
	GamePlayer *commPlayer = static_cast<GamePlayer *>( &*commanderPlayer );
	if( commPlayer )
	{
		// Only do this, however, if his player controller is still pointing to this controller
		CommanderController *currentController = commPlayer->GetPlayerController();
		if( currentController == this )
		{
			commPlayer->SetPlayerController(nullptr);
			commPlayer->SetDeathTime( TheTimeMgr->GetAbsoluteTime() );
		}
	}

	// Cleanup cache
	Array<long> *comCache = TheGameCacheMgr->GetGameCache(kCacheCommanders, m_lTeam);

	if( comCache ) {
		long ix = comCache->FindElement( GetControllerIndex() );
		if( ix >= 0 ) comCache->RemoveElement( ix );
	}

	// Cleanup abilities
	if( m_pAbility1 ) {

		delete m_pAbility1;
		m_pAbility1 = nullptr;
	}

	if( m_pAbility2 ) {

		delete m_pAbility2;
		m_pAbility2 = nullptr;
	}

	if( m_pAbility3 ) {

		delete m_pAbility3;
		m_pAbility3 = nullptr;
	}
}

void CommanderController::Initialize( const Point3D& position, float fRadius, float fHeight )
{
	commanderPlayer = nullptr;
	
	deltaLookAzimuth = 0.0F;
	deltaLookAltitude = 0.0F;
	lookInterpolateParam = 0.0F;
	
	movementFlags = 0;
	firingTime = 0;
	targetDistance = 0.0F;

	SetCollider( new BTCylinderCollider( position, this, fRadius, fHeight ) );

	m_lKillCount = 0;
	m_ulCommanderRank = 1;
}

void CommanderController::Pack(Packer& data, unsigned long packFlags) const
{
	BTCharacterController::Pack(data, packFlags);
	
	data << ChunkHeader('flag', 4);
	unsigned long flags = commanderFlags & ~kCommanderFiring;
	data << flags;
	
	data << ChunkHeader('ornt', 12);
	data << primaryAzimuth;
	data << lookAzimuth;
	data << lookAltitude;
	
	data << TerminatorChunk;
}

void CommanderController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	BTCharacterController::Unpack(data, unpackFlags);
	UnpackChunkList<CommanderController>(data, unpackFlags, &CommanderController::UnpackChunk);
}

bool CommanderController::UnpackChunk(const ChunkHeader *chunkHeader, Unpacker& data, unsigned long unpackFlags)
{
	switch (chunkHeader->chunkType)
	{
		case 'flag':
			
			data >> commanderFlags;
			return (true);
		
		case 'ornt':
			
			data >> primaryAzimuth;
			data >> lookAzimuth;
			data >> lookAltitude;
			return (true);
	}
	
	return (false);
}

ControllerMessage *CommanderController::ConstructMessage(ControllerMessageType type) const
{
	switch (type)
	{
		case kCommanderMessageEngageInteraction:
			
			return (new CommanderBeginInteractionMessage(GetControllerIndex()));
		
		case kCommanderMessageDisengageInteraction:
			
			return (new CommanderInteractionMessage(kCommanderMessageDisengageInteraction, GetControllerIndex()));
		
		case kCommanderMessageBeginMovement:
		case kCommanderMessageEndMovement:
		case kCommanderMessageChangeMovement:
			
			return (new CommanderMovementMessage(type, GetControllerIndex()));
		
		case kCommanderMessageTeleport:
			
			return (new CommanderTeleportMessage(GetControllerIndex()));
		
		case kCommanderMessageLaunch:
			
			return (new CharacterStateMessage(kCommanderMessageLaunch, GetControllerIndex()));
		
		case kCommanderMessageDamage:
			
			return (new CommanderDamageMessage(GetControllerIndex()));
	}
	
	return (BTCharacterController::ConstructMessage(type));
}

float CommanderController::GetTargetAltitude( void )
{
	return (m_fTargetAltitude);
}

void CommanderController::ReceiveMessage(const ControllerMessage *message)
{
	switch (message->GetControllerMessageType())
	{
		case kCommanderMessageEngageInteraction:
		{
			const CommanderBeginInteractionMessage *m = static_cast<const CommanderBeginInteractionMessage *>(message);
			
			World *world = GetTargetNode()->GetWorld();
			Controller *controller = world->GetController(m->GetInteractionControllerIndex());
			if (controller)
			{
				controller->HandleInteractionEvent(kInteractionEventEngage, &m->GetInteractionPosition());
				
				Node *node = controller->GetTargetNode();
				if (node->GetNodeType() == kNodeEffect)
				{
					const Effect *effect = static_cast<Effect *>(node);
					if (effect->GetEffectType() == kEffectPanel)
					{

					}
				}
				else
				{
					// If the player is looking at a geometry, show the "Use" message.
					
					TheUsePopupInterface->Engage();
				}
				
				if (!TheMessageMgr->Server())
				{
					m_objCommanderInteractor.SetInteractionNode(node);
					world->AddInteractor(&m_objCommanderInteractor);
				}
			}
			
			break;
		}
		
		case kCommanderMessageDisengageInteraction:
		{
			const CommanderInteractionMessage *m = static_cast<const CommanderInteractionMessage *>(message);
			
			World *world = GetTargetNode()->GetWorld();
			Controller *controller = world->GetController(m->GetInteractionControllerIndex());
			if (controller)
			{
				controller->HandleInteractionEvent(kInteractionEventDisengage, nullptr);
				
				Node *node = controller->GetTargetNode();
				if (node->GetNodeType() == kNodeEffect)
				{
					const Effect *effect = static_cast<Effect *>(node);
					if (effect->GetEffectType() == kEffectPanel)
					{

					}
				}
				else
				{
					// If the player was looking at a geometry, hide the "Use" message.
					
					TheUsePopupInterface->Disengage();
				}
				
				if (!TheMessageMgr->Server())
				{
					m_objCommanderInteractor.SetInteractionNode(nullptr);
					world->RemoveInteractor(&m_objCommanderInteractor);
				}
			}
			
			break;
		}
		
		case kCommanderMessageBeginMovement:
		{
			const CommanderMovementMessage *m = static_cast<const CommanderMovementMessage *>(message);
			
			unsigned long flag = m->GetMovementFlag();
			const Point3D& position = m->GetInitialPosition();

			if (commanderPlayer != TheMessageMgr->GetLocalPlayer()) SetOrientation(m->GetMovementAzimuth(), m->GetMovementAltitude());
			
			movementFlags |= flag;
			break;
		}
		
		case kCommanderMessageEndMovement:
		{
			const CommanderMovementMessage *m = static_cast<const CommanderMovementMessage *>(message);
			
			const Point3D& position = m->GetInitialPosition();
			GetCollider()->GetPosition() = position;

			GetVelocity() = Vector3D(0,0,0);
			
			if (commanderPlayer != TheMessageMgr->GetLocalPlayer()) SetOrientation(m->GetMovementAzimuth(), m->GetMovementAltitude());
			
			movementFlags &= ~m->GetMovementFlag();
			break;
		}
		
		case kCommanderMessageChangeMovement:
		{
			const CommanderMovementMessage *m = static_cast<const CommanderMovementMessage *>(message);
			
			if (commanderPlayer != TheMessageMgr->GetLocalPlayer()) SetOrientation(m->GetMovementAzimuth(), m->GetMovementAltitude());
			
			movementFlags = (movementFlags & ~kMovementPlanarMask) | m->GetMovementFlag();
			break;
		}
		
		case kCommanderMessageTeleport:
		{
			const CommanderTeleportMessage *m = static_cast<const CommanderTeleportMessage *>(message);
			
			const Point3D& position = m->GetInitialPosition();
			
			Node *node = GetTargetNode();
			const Transform4D& inverseTransform = node->GetSuperNode()->GetInverseWorldTransform();
			node->SetNodePosition(inverseTransform * position);
			node->Invalidate();
			
			float azm = m->GetTeleportAzimuth();
			primaryAzimuth = azm;
			m_fAzimuth = azm;
			SetOrientation(azm, m->GetTeleportAltitude());
			
			break;
		}
		
		case kCommanderMessageLaunch:
		{
			const CharacterStateMessage *m = static_cast<const CharacterStateMessage *>(message);
			
			const Point3D& position = m->GetInitialPosition();
			
			Node *node = GetTargetNode();
			node->SetNodePosition(node->GetSuperNode()->GetInverseWorldTransform() * position);

			break;
		}
		
		case kCommanderMessageDamage:
		{
			break;
		}

		case kEntityMessagePlayAnimation:
		{
			const EntityPlayAnimationMessage *m = static_cast<const EntityPlayAnimationMessage *>( message );

			long motion = m->GetMotion();

			// Player(s) on the other team should not be able to see this commander's cooldowns
			GamePlayer *localPlayer = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
			if( localPlayer )
			{
				if( localPlayer->GetPlayerTeam() == GetTeam() )
				{
					if( m_pAbility1 ) m_pAbility1->HandleEvent( kAbilityEventTriggered, motion, nullptr );
					if( m_pAbility2 ) m_pAbility2->HandleEvent( kAbilityEventTriggered, motion, nullptr );
					if( m_pAbility3 ) m_pAbility3->HandleEvent( kAbilityEventTriggered, motion, nullptr );
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

void CommanderController::SendSnapshot(void)
{
	const Point3D& position = GetPosition();
	
	TheMessageMgr->SendMessageClients(EntityUpdatePositionMessage(GetControllerIndex(), position));
	TheMessageMgr->SendMessageClients(EntityUpdateAzimuthMessage(GetControllerIndex(), m_fAzimuth));
}

void CommanderController::Preprocess(void)
{
	BTCharacterController::Preprocess();

	SetAnimation( kMotionNone );
	
	Model *model = static_cast<Model*>(GetTargetNode());
	
	World *world = model->GetWorld();
	
	if (TheMessageMgr->Server())
	{
		TheMessageMgr->AddSnapshotSender(this);
		world->AddInteractor(&m_objCommanderInteractor);
	}
	
	commanderFlags |= kCommanderTeleported;

	// award experience increments kill count, so see if m_lKillCount will unlock any ranks
	m_lKillCount -= 1;
	AwardExperience( *this );

	// Setup abilities
	m_pAbility1 = Ability::Construct( this, kStringTableCommanders, m_ulStringID, 'abi1' );
	m_pAbility2 = Ability::Construct( this, kStringTableCommanders, m_ulStringID, 'abi2' );
	m_pAbility3 = Ability::Construct( this, kStringTableCommanders, m_ulStringID, 'abi3' );

	// Add crosshair assistance to world
	Zone *zone = world->FindZone( m_pCollider->GetPosition() );
	zone->AddNewSubnode( &m_objCrossAssist );
	m_objCrossAssist.Disable();
}

void CommanderController::EnterWorld(Zone *zone, const Point3D& zonePosition)
{
	MaterializeSystem *system = new MaterializeSystem(ColorRGB(0.5F, 1.0F, 0.25F), 0.5F);
	system->SetNodePosition(Point3D(zonePosition.x, zonePosition.y, zonePosition.z + 1.0F));
	zone->AddNewSubnode(system);
}

void CommanderController::Move(void)
{	
	if( GetEntityFlags() & kEntityDead ) {
		if( TheTimeMgr->GetAbsoluteTime() >= GetKillTime() ) {
			BTEntityController::Kill();
			return;
		}

		Model *commander = static_cast<Model*>( GetTargetNode() );
		commander->Animate();
		Travel();
		return;
	}

	if( TheMessageMgr->Server() ) {
		StatsProtocol *stats = GetStats();
		if( stats && stats->GetHealth() <= 0.0F )
		{
			Kill();
			return;
		}
	}

	// Update all abilities with the current time (abilities may need to do extra processing if they happen over a duration).
	unsigned long curTime = TheTimeMgr->GetAbsoluteTime();
	if( m_pAbility1 ) m_pAbility1->Update( curTime );
	if( m_pAbility2 ) m_pAbility2->Update( curTime );
	if( m_pAbility3 ) m_pAbility3->Update( curTime );

	// If the commander is stunned, don't let it move
	if( GetEntityFlags() & kEntityStunned )
	{
		BTEntityController::Move();
		BTEntityController::PostTravel();
		return;
	}


	float dt = TheTimeMgr->GetFloatDeltaTime();

	bool server = TheMessageMgr->Server();
	if (!(commanderFlags & kCommanderDead))
	{
		static const unsigned char movementIndexTable[16] =
		{
			8, 0, 1, 8,
			2, 4, 6, 2,
			3, 5, 7, 3,
			8, 0, 1, 8
		};
		
		// If you are the local player, just spin without alerting the server to save some network resources.
		if (commanderPlayer == TheMessageMgr->GetLocalPlayer())
		{	
			float azm = lookAzimuth + TheInputMgr->GetMouseDeltaX() + TheGame->GetLookSpeedX() * dt;
			if (azm < -K::pi) azm += K::two_pi;
			else if (azm > K::pi) azm -= K::two_pi;
			
			float alt = lookAltitude + TheInputMgr->GetMouseDeltaY() + TheGame->GetLookSpeedY() * dt;
			alt = Fmin(Fmax(alt, -1.125F), 1.125F);
			
			if ((azm != lookAzimuth) || (alt != lookAltitude))
			{
				lookAzimuth = azm;
				lookAltitude = alt;
				
				if (!server) TheMessageMgr->SendMessage(kPlayerServer, ClientOrientationMessage(azm, alt));
			}

			primaryAzimuth = lookAzimuth;
			m_fAzimuth = lookAzimuth;
			
			TheSoundMgr->SetListenerVelocity(commanderVelocity);
		}
		
		// Now, figure out the direction of movement (forward, backward, strafing, etc.)
		EntityMotion motion = m_EntityMotion;
		Vector3D propel(0.0F, 0.0F, 0.0F);
		
		long index = movementIndexTable[movementFlags & kMovementPlanarMask];
		if (index < 8)
		{
			static const float movementDirectionTable[8] =
			{
				0.0F, 4.0F, 2.0F, -2.0F, 1.0F, -1.0F, 3.0F, -3.0F
			};

			//if( m_lCharFlags & kCharacterGround ) {
			Vector3D moveVelocity(1,0,0);
			float direction = movementDirectionTable[index] * K::pi_over_4 + lookAzimuth;
			moveVelocity.RotateAboutZ(direction);
			moveVelocity.Normalize();
			moveVelocity *= m_pStats->GetMovementSpeed();
			moveVelocity *= (dt / 1000.0F);

			GetVelocity() = moveVelocity;

			if( motion < kMotionAbility1 )
			{
				motion = kMotionMove;
			}
			//}
		}
		else
		{
			if (motion < kMotionDeath ) //kMotionAbility1)
			{
				motion = kMotionNone;
			}
		}
		
		lookInterpolateParam = Fmax(lookInterpolateParam - TheTimeMgr->GetSystemFloatDeltaTime() * TheMessageMgr->GetSnapshotFrequency(), 0.0F);
		
		SetAnimation( motion );
	}
	else
	{
		if (server)
		{
			TheMessageMgr->SendMessageAll(ControllerMessage(Controller::kControllerMessageDeleteNode, GetControllerIndex()));
			return;
		}
	}

	SetTargetAltitude(-lookAltitude);
	
	BTCharacterController::Move();

	Model *commander = static_cast<Model*>( GetTargetNode() );
	commander->Animate();

	Travel();
}

void CommanderController::Travel(void)
{
	BTCharacterController::Travel();

	// Only do this locally
	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( (player == nullptr) || (player->GetPlayerController() != this) )
	{
		BTEntityController::PostTravel();
		return;
	}

	bool showSelector = false;

	// Check if we in range of a base, if so show the commander selector interface
	World *world = TheWorldMgr->GetWorld();
	if( world && TheGameCacheMgr )
	{
		Array<long> *bases = TheGameCacheMgr->GetGameCache( kCacheBases, GetTeam() );
		if( bases )
		{
			Point3D curPos = GetPosition();

			long size = bases->GetElementCount();
			for( long x = 0; x < size; x++ )
			{
				BaseController *cont = static_cast<BaseController *>( world->GetController( (*bases)[x] ) );
				if( cont )
				{
					if( Calculate2DDistance( curPos, cont->GetPosition() ) <= COMMANDER_SELECTER_RANGE )
					{
						showSelector = true;
					}
				}
			}
		}
	}

	if( TheCommanderSelecter )
	{
		if( showSelector ) TheCommanderSelecter->Show();
		else TheCommanderSelecter->Hide();
	}

	// Set cross assistance transform
	Model *com = static_cast<Model*>( GetTargetNode() );
	if( com && m_pCollider ) {
		const Point3D caPosition( com->GetSuperNode()->GetInverseWorldTransform() * m_pCollider->GetPosition() );
		m_objCrossAssist.SetNodePosition( caPosition );
		m_objCrossAssist.SetNodeMatrix3D( com->GetSuperNode()->GetInverseWorldTransform() * Matrix3D().SetRotationAboutZ( m_fAzimuth ) * Matrix3D().SetRotationAboutY( K::pi_over_2 - lookAltitude ) );
		
		m_objCrossAssist.Invalidate();
	}

	BTEntityController::PostTravel();
}

void CommanderController::Damage( float fDamage, long lEnemyIndex )
{
	EntityDamaged data;
	data.m_lSource = lEnemyIndex;
	data.m_lDamage = fDamage;

	// TODO: might be useful to also pass in the ability's ID (as it is already in ability)
	if( m_pAbility1 ) m_pAbility1->HandleEvent( kAbilityEventDamaged, 0, &data );
	if( m_pAbility2 ) m_pAbility2->HandleEvent( kAbilityEventDamaged, 0, &data );
	if( m_pAbility3 ) m_pAbility3->HandleEvent( kAbilityEventDamaged, 0, &data);

	BTCharacterController::Damage( fDamage, lEnemyIndex );
}

void CommanderController::Kill( void )
{
	TheGame->AwardExperience( *this );

	BTCharacterController::Kill();
}

void CommanderController::PerformAbility( long lAbilityNumber )
{
	// If the commander is stunned, don't let it perform abilities
	if( GetEntityFlags() & kEntityStunned )
	{
		return;
	}

	Ability *ability = nullptr;

	switch( lAbilityNumber )
	{
		case kMotionAbility1:
		{
			ability = m_pAbility1;
			break;
		}
		case kMotionAbility2:
		{
			if( m_ulCommanderRank >= 2)
				ability = m_pAbility2;
			break;
		}
		case kMotionAbility3:
		{
			if( m_ulCommanderRank >= 3)
				ability = m_pAbility3;
			break;
		}
	}

	if( ability ) {

		if( ability->Execute() == true ) {

			BTCharacterController::PerformAbility( lAbilityNumber );

			if( m_pAbility1 && (m_pAbility1 != ability)) m_pAbility1->HandleEvent( kAbilityEventServerTriggered, lAbilityNumber, nullptr );
			if( m_pAbility2 && (m_pAbility2 != ability)) m_pAbility2->HandleEvent( kAbilityEventServerTriggered, lAbilityNumber, nullptr );
			if( m_pAbility3 && (m_pAbility3 != ability)) m_pAbility3->HandleEvent( kAbilityEventServerTriggered, lAbilityNumber, nullptr );

			// Play sound
			GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
			if( world ) {
				TheMessageMgr->SendMessageAll( WorldSoundMessage(ability->GetSound(), m_pCollider->GetPosition(), 16.0F) );
			}
		}
	}
}

void CommanderController::AwardExperience( BTEntityController &objEntity )
{
	++m_lKillCount;

	long nextRankKills = 0;

	long worldID = TheGame->GetWorldID();
	const StringTable *table = TheStringTableMgr->GetStringTable( kStringTableWorlds );

	CommanderInfoPane *pane = static_cast<CommanderInfoPane *>( &TheInfoPaneMgr->GetInfoPane(kInfoPaneCommander) );

	bool local = false;

	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() ); 
	if( player && player->GetPlayerController() == this ) {

		local = true;
	}
	
	if( m_ulCommanderRank == 1 ) {

		nextRankKills = Text::StringToInteger( table->GetString( StringID( worldID, 'INFO', 'RANK', 'RNK2' ) ) );

		if( m_lKillCount >= nextRankKills ) {

			++m_ulCommanderRank;

			if( local ) {
	
				pane->GetAbilityBar()->UnlockAbility( 1, true );
				pane->GetRankBar()->UnlockRank( 1, true );
			}
		}
	} 

	if( m_ulCommanderRank == 2 ) {

		nextRankKills = Text::StringToInteger( table->GetString( StringID( worldID, 'INFO', 'RANK', 'RNK3' ) ) );

		if( m_lKillCount >= nextRankKills ) {

			++m_ulCommanderRank;

			if( local ) {

				pane->GetAbilityBar()->UnlockAbility( 2, true );
				pane->GetRankBar()->UnlockRank( 2, true );
			}
		}
	}

	if( local ) {

		pane->GetRankBar()->AddToKillCount( 1 );
	}
}

void CommanderController::UpdateOrientation(float azm, float alt)
{
	deltaLookAzimuth = GetInterpolatedLookAzimuth() - azm;
	if (deltaLookAzimuth > K::pi) deltaLookAzimuth -= K::two_pi;
	else if (deltaLookAzimuth < -K::pi) deltaLookAzimuth += K::two_pi;
	
	deltaLookAltitude = GetInterpolatedLookAltitude() - alt;
	
	lookAzimuth = azm;
	m_fAzimuth = azm;
	lookAltitude = alt;
	m_fTargetAltitude = alt;
	lookInterpolateParam = 1.0F;
}

void CommanderController::BeginMovement(unsigned long flag, float azm, float alt)
{
	const Point3D& position = GetPosition();

	Vector3D velocity = Vector3D(1,0,0);	// normalized vector
	velocity.RotateAboutZ(azm);				// rotate to go forwards
	velocity *= m_pStats->GetMovementSpeed();
	
	CommanderMovementMessage message(kCommanderMessageBeginMovement, GetControllerIndex(), position, velocity, azm, alt, flag);
	TheMessageMgr->SendMessageAll(message);
}

void CommanderController::EndMovement(unsigned long flag, float azm, float alt)
{
	const Point3D& position = GetPosition();
	Vector3D velocity(0,0,0);
	
	CommanderMovementMessage message(kCommanderMessageEndMovement, GetControllerIndex(), position, velocity, azm, alt, flag);
	TheMessageMgr->SendMessageAll(message);
}

void CommanderController::ChangeMovement(unsigned long flags, float azm, float alt)
{
	const Point3D& position = GetPosition();
	Vector3D velocity = GetVelocity();
	
	CommanderMovementMessage message(kCommanderMessageChangeMovement, GetControllerIndex(), position, velocity, azm, alt, flags);
	TheMessageMgr->SendMessageAll(message);
}

void CommanderController::BeginFiring(bool primary, float azm, float alt)
{
}

void CommanderController::EndFiring(float azm, float alt)
{
}

void CommanderController::SetAnimation( EntityMotion motion )
{
	BTCharacterController::SetAnimation( motion );
}

void CommanderController::InitializeCommanderInfoPane( CommanderInfoPane& pane ) 
{
	// Make abilities unclickable for every other player other than the one who owns this commander
	bool clickable = false;

	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( player->GetPlayerController() == this )
	{
		clickable = true;
	}

	bool locked = true;

	if( player->GetPlayerTeam() == GetTeam() ) {

		locked = false;
	}

	AbilityBar *abilityBar = pane.GetAbilityBar();

	abilityBar->SetAbility( 0, m_pAbility1, kStringTableCommanders, m_ulStringID, 'abi1', 1, clickable );

	if( locked )
		abilityBar->UnlockAbility( 0, false );
	else
		abilityBar->UnlockAbility( 0, m_ulCommanderRank >= 1 ? true : false );

	abilityBar->SetAbility( 1, m_pAbility2, kStringTableCommanders, m_ulStringID, 'abi2', 2, clickable );

	if( locked )
		abilityBar->UnlockAbility( 1, false );
	else
		abilityBar->UnlockAbility( 1, m_ulCommanderRank >= 2 ? true : false );

	abilityBar->SetAbility( 2, m_pAbility3, kStringTableCommanders, m_ulStringID, 'abi3', 3, clickable );

	if( locked )
		abilityBar->UnlockAbility( 2, false );
	else
		abilityBar->UnlockAbility( 2, m_ulCommanderRank >= 3 ? true : false );

	RankBar *rankBar = pane.GetRankBar();
	
	rankBar->UnlockRank(0, m_ulCommanderRank >= 1 ? true : false );
	rankBar->UnlockRank(1, m_ulCommanderRank >= 2 ? true : false );
	rankBar->UnlockRank(2, m_ulCommanderRank >= 3 ? true : false );

	pane.GetHealthBar()->SetStatsController( GetStats() );

	//AwardExperience( *this );
}

void CommanderController::EnableCrosshairAssistance( void )
{
	m_objCrossAssist.Enable();
}

void CommanderController::DisableCrosshairAssistance( void )
{
	m_objCrossAssist.Disable();
}


// Messages

CreateCommanderMessage::CreateCommanderMessage() : CreateModelMessage(kModelMessageCommander)
{
}

CreateCommanderMessage::CreateCommanderMessage(long index, unsigned long ulEntityID, const Point3D& position, float azm, float alt, const Vector3D& ground, const Vector3D& slide1, const Vector3D& slide2, unsigned long character, unsigned long movement, long key, long  lStartKillCount, unsigned long ulBaseIndex ) : 
	CreateModelMessage(kModelMessageCommander, index, position)
{
	m_ulEntityID = ulEntityID;

	initialAzimuth = azm;
	initialAltitude = alt;
	
	groundNormal = ground;
	slideNormal[0] = slide1;
	slideNormal[1] = slide2;
	
	characterFlags = character;
	movementFlags = movement;
	
	playerKey = key;

	m_lStartKillCount =  lStartKillCount;

	m_ulBaseIndex = ulBaseIndex;
}

CreateCommanderMessage::~CreateCommanderMessage()
{
}
			
void CreateCommanderMessage::Compress(Compressor& data) const
{
	CreateModelMessage::Compress(data);
	
	data << m_ulEntityID;
	data << m_lStartKillCount;

	data << initialAzimuth;
	data << initialAltitude;
	
	data << groundNormal.x;
	data << groundNormal.y;
	data << groundNormal.z;
	
	data << slideNormal[0].x;
	data << slideNormal[0].y;
	data << slideNormal[0].z;
	
	data << slideNormal[1].x;
	data << slideNormal[1].y;
	data << slideNormal[1].z;
	
	data << characterFlags;
	data << (unsigned char) movementFlags;
	
	data << (short) playerKey;

	data << m_ulBaseIndex;
}

bool CreateCommanderMessage::Decompress(Decompressor& data)
{
	if (CreateModelMessage::Decompress(data))
	{
		short			key;
		unsigned char	movement;
		
		data >> m_ulEntityID;
		data >> m_lStartKillCount;

		data >> initialAzimuth;
		data >> initialAltitude;
		
		data >> groundNormal.x;
		data >> groundNormal.y;
		data >> groundNormal.z;
		
		data >> slideNormal[0].x;
		data >> slideNormal[0].y;
		data >> slideNormal[0].z;
		
		data >> slideNormal[1].x;
		data >> slideNormal[1].y;
		data >> slideNormal[1].z;
		
		data >> characterFlags;
		
		data >> movement;
		movementFlags = movement;
		
		data >> key;
		playerKey = key;

		data >> m_ulBaseIndex;
		
		return (true);
	}
	
	return (false);
}


CommanderInteractionMessage::CommanderInteractionMessage(ControllerMessageType type, long controllerIndex) : ControllerMessage(type, controllerIndex)
{
}

CommanderInteractionMessage::CommanderInteractionMessage(ControllerMessageType type, long controllerIndex, long interactionIndex) : ControllerMessage(type, controllerIndex)
{
	interactionControllerIndex = interactionIndex;
}

CommanderInteractionMessage::~CommanderInteractionMessage()
{
}

void CommanderInteractionMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress(data);
	
	data << (unsigned short) interactionControllerIndex;
}

bool CommanderInteractionMessage::Decompress(Decompressor& data)
{
	if (ControllerMessage::Decompress(data))
	{
		unsigned short	index;
		
		data >> index;
		interactionControllerIndex = index;
		
		return (true);
	}
	
	return (false);
}


CommanderBeginInteractionMessage::CommanderBeginInteractionMessage(long controllerIndex) : CommanderInteractionMessage(CommanderController::kCommanderMessageEngageInteraction, controllerIndex)
{
}

CommanderBeginInteractionMessage::CommanderBeginInteractionMessage(long controllerIndex, long interactionIndex, const Point3D& position) : CommanderInteractionMessage(CommanderController::kCommanderMessageEngageInteraction, controllerIndex, interactionIndex)
{
	interactionPosition = position;
}

CommanderBeginInteractionMessage::~CommanderBeginInteractionMessage()
{
}

void CommanderBeginInteractionMessage::Compress(Compressor& data) const
{
	CommanderInteractionMessage::Compress(data);
	
	data << interactionPosition.x;
	data << interactionPosition.y;
	data << interactionPosition.z;
}

bool CommanderBeginInteractionMessage::Decompress(Decompressor& data)
{
	if (CommanderInteractionMessage::Decompress(data))
	{
		data >> interactionPosition.x;
		data >> interactionPosition.y;
		data >> interactionPosition.z;
		
		return (true);
	}
	
	return (false);
}


CommanderMovementMessage::CommanderMovementMessage(ControllerMessageType type, long controllerIndex) : ControllerMessage(type, controllerIndex)
{
}

CommanderMovementMessage::CommanderMovementMessage(ControllerMessageType type, long controllerIndex, const Point3D& position, const Vector3D& velocity, float azimuth, float altitude, unsigned long flag) : ControllerMessage(type, controllerIndex)
{
	initialPosition = position;
	movementAzimuth = azimuth;
	movementAltitude = altitude;
	movementFlag = flag;
}

CommanderMovementMessage::~CommanderMovementMessage()
{
}

void CommanderMovementMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress(data);
	
	data << initialPosition.x;
	data << initialPosition.y;
	data << initialPosition.z;

	data << movementAzimuth;
	data << movementAltitude;
	
	data << (unsigned char) movementFlag;
}

bool CommanderMovementMessage::Decompress(Decompressor& data)
{
	if (ControllerMessage::Decompress(data))
	{
		unsigned char	flag;

		data >> initialPosition.x;
		data >> initialPosition.y;
		data >> initialPosition.z;
		
		data >> movementAzimuth;
		data >> movementAltitude;
		
		data >> flag;
		movementFlag = flag;
		
		return (true);
	}
	
	return (false);
}


CommanderTeleportMessage::CommanderTeleportMessage(long controllerIndex) : CharacterStateMessage(CommanderController::kCommanderMessageTeleport, controllerIndex)
{
}

CommanderTeleportMessage::CommanderTeleportMessage(long controllerIndex, const Point3D& position, const Vector3D& velocity, float azimuth, float altitude, const Point3D& center) : CharacterStateMessage(CommanderController::kCommanderMessageTeleport, controllerIndex, position, velocity)
{
	teleportAzimuth = azimuth;
	teleportAltitude = altitude;
	
	effectCenter = center;
}

CommanderTeleportMessage::~CommanderTeleportMessage()
{
}

void CommanderTeleportMessage::Compress(Compressor& data) const
{
	CharacterStateMessage::Compress(data);
	
	data << teleportAzimuth;
	data << teleportAltitude;
	
	data << effectCenter.x;
	data << effectCenter.y;
	data << effectCenter.z;
}

bool CommanderTeleportMessage::Decompress(Decompressor& data)
{
	if (CharacterStateMessage::Decompress(data))
	{
		data >> teleportAzimuth;
		data >> teleportAltitude;
		
		data >> effectCenter.x;
		data >> effectCenter.y;
		data >> effectCenter.z;
		
		return (true);
	}
	
	return (false);
}

CommanderDamageMessage::CommanderDamageMessage(long controllerIndex) : ControllerMessage(CommanderController::kCommanderMessageDamage, controllerIndex)
{
}

CommanderDamageMessage::CommanderDamageMessage(long controllerIndex, long intensity, const Point3D& center) : ControllerMessage(CommanderController::kCommanderMessageDamage, controllerIndex)
{
	damageIntensity = intensity;
	damageCenter = center;
}

CommanderDamageMessage::~CommanderDamageMessage()
{
}

void CommanderDamageMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress(data);
	
	data << (unsigned short) damageIntensity;
	data << damageCenter.x;
	data << damageCenter.y;
	data << damageCenter.z;
}

bool CommanderDamageMessage::Decompress(Decompressor& data)
{
	if (ControllerMessage::Decompress(data))
	{
		unsigned short	intensity;
		
		data >> intensity;
		damageIntensity = intensity;
		
		data >> damageCenter.x;
		data >> damageCenter.y;
		data >> damageCenter.z;
		
		return (true);
	}
	
	return (false);
}
