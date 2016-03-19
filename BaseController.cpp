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

#include "MGGame.h"
#include "PointFinder.h"
#include "StatsProtocol.h"

#define BUFFER_DIST			0.5F
#define MAX_RADIUS_MULT		4.0F

using namespace C4;

BaseController::BaseController() : 
	BTEntityController( kControllerBase ),
	m_CommanderType( 0 ),
	m_lSpawnTimer( 0 ),
	m_lSpawnNumber( 0 ),
	m_UnitType( 0 ),
	m_bSpawn( false ),
	m_ulNextSpawn( 0 ),
	m_fSpawnTimerEffect (1.0F),
	m_fTempHeight( 0.0F ),
	m_fTempRadius( 0.0F ),
	m_fTempMaxHP( 0.0F ),
	m_fTempHP( 0.0F ),
	m_bIsActive( true )
{
	SetBaseControllerType( kControllerEntity );
}

BaseController::~BaseController()
{
	if( TheInfoPaneMgr ) {

		TheInfoPaneMgr->GetCurrentPane().HideIfSelected( GetControllerIndex() );
	}

	if( TheGameCacheMgr == nullptr )
		return;

	// Find all plots associated with this base and delete them
	if( TheMessageMgr->Server() )
	{
		Array<long> *plotCache = TheGameCacheMgr->GetGameCache(kCachePlots, m_lTeam);
		if( plotCache )
		{
			World *world = TheWorldMgr->GetWorld();
			if( world )
			{
				long size = plotCache->GetElementCount();
				for( long x = 0; x < size; x++ )
				{
					long index = (*plotCache)[x];		
					PlotController *plot = static_cast<PlotController *>( TheWorldMgr->GetWorld()->GetController( index ) );

					if( plot && plot->GetBase() == this )
					{
						TheMessageMgr->SendMessageAll( ControllerMessage( kEntityMessageDeath, plot->GetControllerIndex() ) );

						long newSize = plotCache->GetElementCount();
						if( size != newSize )
						{
							size = newSize;
							x--;
						}
					}
				}
			}
		}
	}

	Array<long> *baseCache = TheGameCacheMgr->GetGameCache(kCacheBases, m_lTeam);

	// When the base is destroyed, remove it from the stored list of bases
	if( baseCache ) {
		long ix = baseCache->FindElement( GetControllerIndex() );
		if( ix >= 0 ) baseCache->RemoveElement( ix );
	}

	// Make destructible model invisible.
	Node *node = GetTargetNode();
	if( node ) {
		Node *dkt = node->GetConnectedNode('dskt');
		if( dkt ) {
			dkt->Enable();
		}
	}

	// Display message for player who lost base.
	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( player ) {
		if( GetTeam() == player->GetPlayerTeam() ) {
			TheGame->HelpMessage( 'bade' );
		}
	}
}

// Needed as part of replicate to work in the world editor.
BaseController::BaseController(const BaseController& bController) : 
	BTEntityController( kControllerBase, kEntityBase )
{
	m_lTeam = bController.m_lTeam;
	m_UnitType = bController.m_UnitType;
	m_CommanderType = bController.m_CommanderType;
	m_lSpawnNumber = bController.m_lSpawnNumber;
	m_lSpawnTimer = bController.m_lSpawnTimer;
	m_bSpawn = bController.m_bSpawn;
	m_fSpawnTimerEffect = bController.m_fSpawnTimerEffect;
	m_ulNextSpawn = bController.m_ulNextSpawn;
	m_fTempHeight = bController.m_fTempHeight;
	m_fTempRadius = bController.m_fTempRadius;
	m_fTempMaxHP = bController.m_fTempMaxHP;
	m_fTempHP = bController.m_fTempHP;
	m_bIsActive = bController.m_bIsActive;

	// TODO: Replicate Entity as well
	m_pStats = nullptr;
	m_pCollider = nullptr;

	SetBaseControllerType( kControllerEntity );
}

// Needed to work in world editor.
Controller *BaseController::Replicate(void) const
{
	return (new BaseController(*this));
}

void BaseController::Damage( float fDamage, long lEnemyIndex )
{
	BTEntityController::Damage( fDamage, lEnemyIndex );
}

// This controller can only be placed on a Marker node.
bool BaseController::ValidNode( const Node *node )
{
	// temporarily can be used on geometry and model nodes
	if (node->GetNodeType() == kNodeModel )
		return (true);
	else if (node->GetNodeType() == kNodeGeometry)
		return (true);
	else if( node->GetNodeType() == kNodeMarker )
		return (true);
	
	return (false);
}

// Adds the value to the given stat (doesn't override).
// Uses the base base stats in the StatsProtocol.
void BaseController::UpdateUpgrade( long lStatID, float fValue )
{
	if( !TheMessageMgr->Server() ) {
		return;
	}

	if( lStatID & kStatMaxHealth ) {
		float val = m_objUpgrades.GetBaseMaxHealth();
		m_objUpgrades.SetBaseMaxHealth( val + fValue );
	}
	else if( lStatID & kStatMoveSpeed ) {
		float val = m_objUpgrades.GetBaseMovementSpeed();
		m_objUpgrades.SetBaseMovementSpeed( val + fValue );
	}
	else if( lStatID & kStatStrength ) {
		float val = m_objUpgrades.GetBaseStrength();
		m_objUpgrades.SetBaseStrength( val + fValue );
	}
	else if( lStatID & kStatArmor ) {
		float val = m_objUpgrades.GetBaseArmor();
		m_objUpgrades.SetBaseArmor( val + fValue );
	}
}

// Save to world file.
void BaseController::Pack(Packer& data, unsigned long packFlags) const
{
	// Pack up the base-controller data.

	BTEntityController::Pack(data, packFlags);

	data << m_lTeam;
	data << m_UnitType;
	data << m_CommanderType;
	data << m_lSpawnTimer;
	data << m_lSpawnNumber;
	data << m_fTempHeight;
	data << m_fTempRadius;
	data << m_fTempMaxHP;
	data << m_fTempHP;
}

// Load from world file.
void BaseController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	BTEntityController::Unpack(data, unpackFlags);

	data >> m_lTeam;
	data >> m_UnitType;
	data >> m_CommanderType;
	data >> m_lSpawnTimer;
	data >> m_lSpawnNumber;
	data >> m_fTempHeight;
	data >> m_fTempRadius;
	data >> m_fTempMaxHP;
	data >> m_fTempHP;
}

// The number of settings this controller will display in the world editor.
long BaseController::GetSettingCount(void) const
{
	return (9);
}

// Get the value of the setting (from what was entered in the world editor).
Setting *BaseController::GetSetting(long index) const
{
	const StringTable *table = TheGame->GetStringTable();

	// Player team (1 or 2)
	if (index == 0)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'TEAM'));
		return (new IntegerSetting('TEAM', m_lTeam, title, 1, 2, 1));
	}

	// Type of unit to spawn (4 char identifier), see Stringtable
	if (index == 1)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'UNIT'));
		return (new TextSetting('UNIT', Text::TypeToString(m_UnitType), title, 4, NULL));
	}

	// Type of commander to spawn (4 char identifier), see Stringtable
	if (index == 2)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'COMM'));
		return (new TextSetting('COMM', Text::TypeToString(m_CommanderType), title, 4, NULL));
	}

	// Number of seconds between when unit waves spawn
	if (index == 3)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'TIME'));
		return (new IntegerSetting('TIME', m_lSpawnTimer, title, 0, 100, 1));
	}

	// Number of units that spawn in each wave
	if (index == 4)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'NUMB'));
		return (new IntegerSetting('NUMB', m_lSpawnNumber, title, 0, 25, 1));
	}

	// Height of the base
	if (index == 5)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'HITE'));
		return (new FloatSetting('HITE', m_fTempHeight, title, 0.0, 100.0, 1.0));
	}

	// Radius of the base
	if (index == 6)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'RADN'));
		return (new FloatSetting('RADN', m_fTempRadius, title, 0.0, 100.0, 1.0));
	}

	// Maximum hit points
	if (index == 7)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'MXHP'));
		return (new FloatSetting('MXHP', m_fTempMaxHP, title, 0.0, 10000.0, 5.0));
	}

	// Current hit points
	if (index == 8)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerBase, 'CRHP'));
		return (new FloatSetting('CRHP', m_fTempHP, title, 0.0, 10000.0, 5.0));
	}

	return (nullptr);
}

// World editor uses this to go from text field in editor to actually stored by this controller.
// See GetSetting() for list of descriptors for each setting
void BaseController::SetSetting(const Setting *setting)
{
	Type identifier = setting->GetSettingIdentifier();

	if( identifier == 'TEAM' )
	{
		m_lTeam = static_cast<const IntegerSetting *>(setting)->GetIntegerValue();
	}
	if( identifier == 'UNIT' ) 
	{
		m_UnitType = Text::StringToType((static_cast<const TextSetting *>(setting))->GetText());
	}
	if( identifier == 'COMM' ) 
	{
		m_CommanderType = Text::StringToType((static_cast<const TextSetting *>(setting))->GetText());
	}
	if( identifier == 'TIME' ) 
	{
		m_lSpawnTimer = static_cast<const IntegerSetting *>(setting)->GetIntegerValue();
	}
	if( identifier == 'NUMB' )
	{
		m_lSpawnNumber = static_cast<const IntegerSetting *>(setting)->GetIntegerValue();
	}
	if( identifier == 'HITE' )
	{
		m_fTempHeight = static_cast<const FloatSetting *>(setting)->GetFloatValue();
	}
	if( identifier == 'RADN' )
	{
		m_fTempRadius = static_cast<const FloatSetting *>(setting)->GetFloatValue();
	}
	if( identifier == 'MXHP' )
	{
		m_fTempMaxHP = static_cast<const FloatSetting *>(setting)->GetFloatValue();
	}
	if( identifier == 'CRHP' )
	{
		m_fTempHP = static_cast<const FloatSetting *>(setting)->GetFloatValue();
	}
}

/* Start spawning units */
void BaseController::StartSpawning()
{
	m_bSpawn = true;

	m_ulNextSpawn = TheTimeMgr->GetAbsoluteTime();
	m_ulNextSpawn += (GetSpawnTimer() * GetSpawnTimerEffect()) * 1000;
}

/* Stop spawning units */
void BaseController::StopSpawning()
{
	m_bSpawn = false;
}

/* Create a wave of units around this base */
bool BaseController::SpawnUnits()
{
	if( !TheMessageMgr->Server() ) {
		return false;
	}

	if( !m_bIsActive ) {
		return false;
	}

	if( m_lSpawnNumber <= 0 ) {
		return false;
	}

	if(m_bSpawn == false) { 
		return false;
	}

	Array<long> *unitCache = TheGameCacheMgr->GetGameCache(kCacheUnits, GetTeam());

	if( unitCache == nullptr ) {
		return false;
	}

	Node *node = GetTargetNode();
	Point3D position = node->GetWorldPosition();
	
	Array<Point3D> safe_points;
	
	float radius = m_fTempRadius;
	BTCylinderCollider *col = static_cast<BTCylinderCollider*>( GetCollider() );
	if( col ) {
		radius = col->GetRadius();
	}

	float safe_dist = 3.0F;
	//float safe_dist = radius * UNIT_SAFE_DIST_MULT;

	// Find some safe areas to spawn units around the base
	PointFinder::FindSafePoint_Spiral( position, m_lSpawnNumber, safe_points, radius + BUFFER_DIST, BUFFER_DIST, K::pi_over_20, MAX_RADIUS_MULT * radius, safe_dist, Math::RandomFloat( K::two_pi ), false );
	if( safe_points.GetElementCount() != m_lSpawnNumber ) {
		return false;
	}

	// Spawn the correct number of units in each of the safe spots
	for( int i = 0; i < m_lSpawnNumber; i++ ) {
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		Point3D pos = safe_points[i];

		if( unitCache->GetElementCount() < MAX_UNITS ) {
			TheMessageMgr->SendMessageAll(CreateUnitMessage(kModelMessageUnit, index, m_UnitType, pos, 0.0F, 0.0F,
				K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, m_lTeam, GetControllerIndex()));
		}
		else {
			if( TheGame->IsHelpActive() && TheTimeMgr->GetAbsoluteTime() >= TheGame->GetNextNoSpawnTime() ) {
				TheGame->HelpMessage('nspn');
				TheGame->SetNextNoSpawnTime( TheTimeMgr->GetAbsoluteTime() + NO_SPAWN_INTERVAL );
			}
		}

	}
	
	return true;
}

// Spawn commander at some valid position around the base. - FW
bool BaseController::SpawnCommander( PlayerKey nKey, long prevKills )
{
	if( !TheMessageMgr->Server() ) {
		return false;
	}

	if( !m_bIsActive ) {
		return false;
	}

	Node *node = GetTargetNode();
	Point3D position = node->GetWorldPosition();

	Array<Point3D> safe_points;

	float radius = m_fTempRadius;
	BTCylinderCollider *col = static_cast<BTCylinderCollider*>( GetCollider() );
	if( col ) {
		radius = col->GetRadius();
	}

	float safe_dist = radius * UNIT_SAFE_DIST_MULT;

	PointFinder::FindSafePoint_Spiral( position, 1, safe_points, radius + BUFFER_DIST, BUFFER_DIST, K::pi_over_20, MAX_RADIUS_MULT * radius, safe_dist, Math::RandomFloat( K::two_pi ), false );
	if( safe_points.GetElementCount() != 1 ) {
		return false;
	}

	long index = TheWorldMgr->GetWorld()->NewControllerIndex();

	TheMessageMgr->SendMessageAll(CreateCommanderMessage(index, GetCommanderType(), safe_points[0], 0.0F, 0.0F, K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, nKey, prevKills, GetControllerIndex() ));

	return true;
}

void BaseController::Preprocess()
{
	BTEntityController::Preprocess();	

	m_pStats = new StatsProtocol( m_fTempMaxHP, m_fTempHP );
	m_pStats->SetOwner( this );

	SetCollider( new BTCylinderCollider(GetTargetNode()->GetNodePosition(), this, m_fTempRadius, m_fTempHeight) );

	// Make destructible model invisible.
	Node *node = GetTargetNode();
	if( node ) {
		Node *dkt = node->GetConnectedNode('dskt');
		if( dkt ) {
			dkt->Disable();
		}
	}
}

/* Check to see if the wave is ready to be spawned */
void BaseController::Move()
{
	BTEntityController::Move();

	if( !m_bIsActive ) {
		return;
	}

	// if the spawner is not stopped
	if(m_bSpawn) {
		unsigned long now = TheTimeMgr->GetAbsoluteTime();

		// if the wave is ready to be spawned
		if( now >= m_ulNextSpawn ) {
			SpawnUnits();

			//reset the timer
			long x = GetSpawnTimer();
			float y = GetSpawnTimerEffect();
			m_ulNextSpawn = now + ((GetSpawnTimer() * GetSpawnTimerEffect()) * 1000);
		}
	}

	BTEntityController::Travel();

	BTEntityController::PostTravel();
}

ControllerMessage *BaseController::ConstructMessage(ControllerMessageType type) const
{
	switch (type)
	{
		case kBaseMessageConstructionCompleted:
		case kBaseMessageBuildingCanceled:

			return (new ControllerMessage(type, GetControllerIndex()));

	}

	return (BTEntityController::ConstructMessage(type));
}

void BaseController::ReceiveMessage(const ControllerMessage *message)
{
	switch (message->GetControllerMessageType())
	{
		case kBaseMessageConstructionCompleted:
		{
			SetReadyForBuilding( true );

			// Display message for player who built this.
			GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
			if( player ) {
				if( GetTeam() == player->GetPlayerTeam() ) {
					TheGame->HelpMessage( 'buco' );
				}
			}

			break;
		}
		case kBaseMessageBuildingCanceled:
		{
			SetReadyForBuilding( true );

			break;
		}

		default:

			BTEntityController::ReceiveMessage(message);
			break;
	}
}

/* ---- Request Commander Message ---- */

bool RequestCommanderMessage::HandleMessage(Player *sender) const
{
	if( TheMessageMgr->Server() ) {

		GamePlayer *player = static_cast<GamePlayer *>( sender );
		if( player ) {

			Controller *controller = TheWorldMgr->GetWorld()->GetController( m_lBaseControllerID );
			if( controller && controller->GetControllerType() == kControllerBase ) {

				BaseController *base = static_cast<BaseController *>( controller );
				if( base->GetTeam() == player->GetPlayerTeam() ) {

					long prevKills = 0;

					CommanderController *cont = player->GetPlayerController();
					base->SpawnCommander( sender->GetPlayerKey(), prevKills );

					if( cont && !(cont->GetEntityFlags() & kEntityDead) )
					{
						prevKills = cont->GetKillCount();
						cont->BTEntityController::Kill();
					}
				}
			}

		}
	}

	return (true);
}
