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


#include "BTControllers.h"
#include "StringTableManager.h"
#include "BTCacheManager.h"
#include "MGGame.h"
#include "BTGoalTowerDefend.h"

using namespace C4;


BuildingController::BuildingController() : 
	BTEntityController( kControllerBuilding ),
	m_pEmitter( nullptr ),
	m_pEffect( nullptr )
{
	Initialize( Point3D(0,0,0), 0.0F, 0.0F );
	SetBaseControllerType( kControllerEntity );
}

BuildingController::BuildingController(const Point3D& position, unsigned long ulStringID, StatsProtocol *pStats, 
									   float fRadius, float fHeight, long lTeam, long lPlot, long lTime) : 
	BTEntityController( kControllerBuilding, kEntityBuilding, pStats, lTeam, ulStringID, kStringTableBuildings, 0.0F ),
	m_ulStringID( ulStringID ),
	m_pEmitter( nullptr ),
	m_pEffect( nullptr )
{
	m_lPlot = lPlot;

	m_bUnderConstruction = true;
	m_lConstructionTime = lTime;
	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();
	m_ulLastUpdate = m_ulStartTime;

	PlotController *plot = static_cast<PlotController*> (TheWorldMgr->GetWorld()->GetController(m_lPlot));
	if( plot && plot->GetTargetNode() ){
		if(plot->GetTargetNode()->GetConnectorCount() > 1){
			m_pEmitter = plot->GetTargetNode()->GetConnectedNode('EMIT');
		}
	}

	SetParticleEffects();

	Initialize( position, fRadius, fHeight );
	SetBaseControllerType( kControllerEntity );

	m_pStats->AddHealth(1);
}

BuildingController::~BuildingController()
{
	if( TheInfoPaneMgr ) {

		TheInfoPaneMgr->GetCurrentPane().HideIfSelected( GetControllerIndex() );
	}

	// Remove from the cache
	Array<long> *buildingCache = TheGameCacheMgr->GetGameCache(kCacheBuildings, m_lTeam);

	if( buildingCache ) {
		long ix = buildingCache->FindElement( GetControllerIndex() );
		if( ix >= 0 ) buildingCache->RemoveElement( ix );
	}

	m_objBaseGoal.Exit();

	if( TheMessageMgr->Server() ){

		PlotController *plot = static_cast<PlotController*> (TheWorldMgr->GetWorld()->GetController(m_lPlot));
		if( plot )
		{
			BaseController *base = static_cast<BaseController*> (plot->GetBase());

			if( base )
			{
				if( m_bUnderConstruction ) {

					TheMessageMgr->SendMessageAll( ControllerMessage(BaseController::kBaseMessageBuildingCanceled, base->GetControllerIndex()) );
				}
			}
		}
	}

	// Make plot visible again
	PlotController *plot = static_cast<PlotController*> (TheWorldMgr->GetWorld()->GetController(m_lPlot));
	if( plot )
	{
		plot->DespawnBuilding();
	}

	//plot->SetPerspectiveExclusionMask(0);

	// Incase the building is destroyed during construction, turn off particle effects
	ToggleConstructionEffect(false);

	RemoveBuildingEffect();

	// Display message for player who lost building.
	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( player ) {
		if( GetTeam() == player->GetPlayerTeam() ) {
			TheGame->HelpMessage( 'bude' );
		}
	}
}

void BuildingController::Initialize( const Point3D& position, float fRadius, float fHeight )
{
	SetCollider( new BTCylinderCollider( position, this, fRadius, fHeight ) );

	// Play creation sound
	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( player && player->GetPlayerTeam() == GetTeam() ) {
		GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
		const StringTable *st = TheStringTableMgr->GetStringTable( m_lTableID );
		if( world && st ) {
			const char *sound = st->GetString( StringID(m_ulStringID, 'CSND') );
			if( sound ) {
				TheMessageMgr->SendMessageAll( WorldSoundMessage(sound, m_pCollider->GetPosition(), 40.0F) );
			}
		}
	}
}

void BuildingController::Pack(Packer& data, unsigned long packFlags) const
{
	BTEntityController::Pack(data, packFlags);
}

void BuildingController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	BTEntityController::Unpack(data, unpackFlags);
}


void BuildingController::Preprocess(void)
{
	BTEntityController::Preprocess();
	SetAnimation( kMotionNone );

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	SetCollider( new BTCylinderCollider( GetTargetNode()->GetNodePosition(), this, 1.5F, 0.5F ) );

	m_objBaseGoal.Enter();

	// Hide plot
	PlotController *plot = static_cast<PlotController*>( world->GetController(m_lPlot) );
	if( plot ){
		plot->SetPerspectiveExclusionMask(kPerspectiveDirect);
	}

	ToggleConstructionEffect(true);
}

void BuildingController::Damage( float fDamage, long lEnemyIndex )
{
	BTEntityController::Damage( fDamage, lEnemyIndex );

	// Display message for player who lost building.
	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( player ) {
		if( GetTeam() == player->GetPlayerTeam() ) {
			TheGame->HelpMessage( 'bude' );
		}
	}
}

void BuildingController::Move(void)
{
	if( m_pEmitter ) {
		m_pEmitter->Invalidate();
		m_pEmitter->Update();
	}

	if( m_pEffect ) {
		m_pEffect->Invalidate();
		m_pEffect->Update();
	}

	/* Under construction
	* Building gains completion and health
	*/
	if (m_bUnderConstruction == true)
	{
		//curtime - lastupdate
		long now = TheTimeMgr->GetAbsoluteTime();
		int test = ((now - m_ulLastUpdate) >= 1000);
		if(test)
		{
			if(m_lConstructionTime > 0)
			{
				float health = (m_pStats->GetBaseMaxHealth()) / (1.0F * m_lConstructionTime);
				m_pStats->AddHealth(health);
				m_ulLastUpdate += 1000;
			}
		}

		// Construction complete
		long timepassed = now - m_ulStartTime;
		if(timepassed >= (m_lConstructionTime * 1000))
		{
			m_bUnderConstruction = false;
			ToggleConstructionEffect(false);

			PlotController *plot = static_cast<PlotController*> (TheWorldMgr->GetWorld()->GetController(m_lPlot));
			if( plot )
			{
				BaseController *base = static_cast<BaseController*> (plot->GetBase());

				if( base )
				{
					// remove under construction animation stuff
					ApplyBuildingEffect();

					if( TheMessageMgr->Server() )
						TheMessageMgr->SendMessageAll( ControllerMessage(BaseController::kBaseMessageConstructionCompleted, base->GetControllerIndex()) );
				}
			}
		}
	}
	else {
		if( TheMessageMgr->Server() ) {
			m_objBaseGoal.Process();
		}
	}

	Model *building = static_cast<Model*>( GetTargetNode() );
	building->SetNodeMatrix3D(building->GetSuperNode()->GetInverseWorldTransform() * Matrix3D().SetRotationAboutZ(m_fAzimuth));
	building->Animate();
	building->Invalidate();

	BTEntityController::Move();
	BTEntityController::Travel();

	BTEntityController::PostTravel();
}

// Get this building's task.
void BuildingController::ApplyBuildingEffect()
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	if( !TheMessageMgr->Server() ) {
		return;
	}

	// Get this building's task.
	const StringTable *bt = TheStringTableMgr->GetStringTable(kStringTableBuildings);
	if( bt ) {
		const char *task = bt->GetString(StringID(m_ulStringID, 'TASK', 'TYPE'));

		if( Text::CompareTextCaseless( task, "GUARD" ) ) {

			float range = 0.0F;
			if( GetStats() ) {
				range = GetStats()->GetAttackRange();
			}
			float attack_speed = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'ATKS')) );
			float attack_dmg = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'ATKD')) );
			const char *sound = bt->GetString(StringID(m_ulStringID, 'TASK', 'SOND'));
			m_objBaseGoal.AddSubgoal( new BTGoalTowerDefend( this, range, attack_speed, attack_dmg, sound ) );

		}
		else if( Text::CompareTextCaseless( task, "BUFF" ) ) {

			PlotController *pl = static_cast<PlotController*>( world->GetController(m_lPlot) );
			if( pl ) {
				BaseController *base = static_cast<BaseController*>( pl->GetBase() );
				if( base ) {

					float maxhp = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'MXHP')) );
					float armr = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'ARMR')) );
					float str = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'STR')) );
					float mspd = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'MSPD')) );

					base->UpdateUpgrade( kStatMaxHealth, maxhp );
					base->UpdateUpgrade( kStatArmor, armr );
					base->UpdateUpgrade( kStatStrength, str );
					base->UpdateUpgrade( kStatMoveSpeed, mspd );
				}
			}

		}
		else if( Text::CompareTextCaseless( task, "SPAWN" ) ) {

			PlotController *plot = static_cast<PlotController*>( world->GetController(m_lPlot) );
			if( plot )
			{
				BaseController *base = static_cast<BaseController*> (plot->GetBase());
				if( base )
				{

					float percent = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'PCNT')) );
					// Add the percentage back onto spawn timer to make units spawn slower
					base->SetSpawnTimerEffect(base->GetSpawnTimerEffect() - percent/100.0F);		
				}
			}

		}
	}
}

// Remove this building's task.
void BuildingController::RemoveBuildingEffect()
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	if( !TheMessageMgr->Server() ) {
		return;
	}

	// Get this building's task.
	const StringTable *bt = TheStringTableMgr->GetStringTable(kStringTableBuildings);
	if( bt ) {
		const char *task = bt->GetString(StringID(m_ulStringID, 'TASK', 'TYPE'));

		if( Text::CompareTextCaseless( task, "BUFF" ) ) {

			PlotController *pl = static_cast<PlotController*>( world->GetController(m_lPlot) );
			if( pl ) {
				BaseController *base = static_cast<BaseController*>( pl->GetBase() );
				if( base ) {

					float maxhp = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'MXHP')) );
					float armr = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'ARMR')) );
					float str = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'STR')) );
					float mspd = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'MSPD')) );

					base->UpdateUpgrade( kStatMaxHealth, -maxhp );
					base->UpdateUpgrade( kStatArmor, -armr );
					base->UpdateUpgrade( kStatStrength, -str );
					base->UpdateUpgrade( kStatMoveSpeed, -mspd );
				}
			}

		}
		else if( Text::CompareTextCaseless( task, "SPAWN" ) ) {

			PlotController *plot = static_cast<PlotController*>( world->GetController(m_lPlot) );
			if( plot )
			{
				BaseController *base = static_cast<BaseController*> (plot->GetBase());
				if( base )
				{

					float percent = Text::StringToFloat( bt->GetString(StringID(m_ulStringID, 'TASK', 'PCNT')) );
					// Add the percentage back onto spawn timer to make units spawn slower
					base->SetSpawnTimerEffect(base->GetSpawnTimerEffect() + percent/100.0F);		
				}
			}

		}
	}
}



void BuildingController::ToggleConstructionEffect (bool on)
{
	if( !m_pEffect ) {
		return;
	}

	if( !m_pEmitter ) {
		return;
	}

	if( on ) {
		m_pEffect->Enable();
		m_pEffect->SetConnectedNode( 'EMIT', m_pEmitter );
	}
	else {
		m_pEffect->Disable();
	}

}

void BuildingController::SetParticleEffects()
{
	Node *root = TheWorldMgr->GetWorld()->GetRootNode();
	Node *node = root;
	const char* name = NULL;
	// Get the particle effect for the correct team
	if(GetTeam() == 1)
		name = "H_Construct";
	else
		name = "P_Construct";

	while(node){
		const char *nodeName = node->GetNodeName();
		if( (node->GetNodeType() == kNodeEffect) && (nodeName) && Text::CompareTextCaseless(name, nodeName) )
		{
			m_pEffect = node;
			break;
		}
		node = root->GetNextNode(node);
	}
}

void BuildingController::SetAnimation( EntityMotion motion )
{
	BTEntityController::SetAnimation( motion );
}

void BuildingController::AttackEffect( BTCollisionData data )
{
	if( !TheStringTableMgr )
		return;

	const StringTable *table = TheStringTableMgr->GetStringTable(kStringTableBuildings);
	if( table )
	{
		const char *effectType = table->GetString( StringID( GetStringID(), 'TASK', 'EFFE' ) );
		
		if( Text::CompareText( effectType, "SMOKE" ) == true )
		{
			Point3D posOffset( 0.0F, 0.0F, 0.0F );

			posOffset.x += Text::StringToFloat(table->GetString( StringID( GetStringID(), 'TASK', 'EFFE', 'X' ) ));
			posOffset.y += Text::StringToFloat(table->GetString( StringID( GetStringID(), 'TASK', 'EFFE', 'Y' ) ));
			posOffset.z += Text::StringToFloat(table->GetString( StringID( GetStringID(), 'TASK', 'EFFE', 'Z' ) ));

			posOffset.RotateAboutZ( GetAzimuth() );

			TheMessageMgr->SendMessageAll( CreatePositionalEffectMessage( kPositionalEffectMessageSmoke, -1, GetPosition() + posOffset ) );
		}

		else if( Text::CompareText( effectType, "BEAM" ) == true )
		{
			if( data.m_pController )
			{
				TheMessageMgr->SendMessageAll( CreateBoltEffectMessage( GetPosition(), data.m_pController->GetControllerIndex() ) );
			}
		}
	}
}

ControllerMessage *BuildingController::ConstructMessage(ControllerMessageType type) const
{
	//switch (type)
	//{
	//	case kUnitMessageUpdate:

	//		return (new UnitUpdateMessage(GetControllerIndex()));
	//}

	// todo, need to add GameCharacter messages to this
	return (BTEntityController::ConstructMessage(type));
}

void BuildingController::ReceiveMessage(const ControllerMessage *message)
{
	//switch (message->GetControllerMessageType())
	//{

	//	default:
			BTEntityController::ReceiveMessage(message);
	//		break;
	//}
}

/* Create Building Message
**
*/
CreateBuildingMessage::CreateBuildingMessage() : 
	CreateModelMessage(kModelMessageBuilding)
{
}

CreateBuildingMessage::CreateBuildingMessage(ModelMessageType type, long index, unsigned long ulEntityID, const Point3D& position, float azm, long lTeam, long lplotIndex, long lTime) : 
	CreateModelMessage(type, index, position)
{
	m_ulEntityID = ulEntityID;

	m_fInitialAzimuth = azm;
	
	m_lTeam = lTeam;

	m_lPlot = lplotIndex;

	m_lTime = lTime;
}

CreateBuildingMessage::~CreateBuildingMessage()
{
}

void CreateBuildingMessage::Compress(Compressor& data) const
{
	CreateModelMessage::Compress(data);

	data << m_ulEntityID;

	data << m_fInitialAzimuth;
	
	data << m_lTeam;

	data << m_lPlot;
}

bool CreateBuildingMessage::Decompress(Decompressor& data)
{
	if(CreateModelMessage::Decompress(data)) {

		data >> m_ulEntityID;

		data >> m_fInitialAzimuth;
		
		data >> m_lTeam;

		data >> m_lPlot;

		return (true);
	}

	return (false);
}
