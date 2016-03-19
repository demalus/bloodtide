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

#include "BTScript_Spawn.h"
#include "BTControllers.h"
#include "MGGame.h"
#include "PointFinder.h"
#include "BTGoalAttackTo.h"

#define BUFFER_DIST			0.5F
#define MAX_RADIUS			100.0F

using namespace C4;

////// EventScript_Spawn_Attackto_Base_Rand

EventScript_Spawn_Attackto_Base_Rand::EventScript_Spawn_Attackto_Base_Rand( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_Spawn_Attackto_Base_Rand::~EventScript_Spawn_Attackto_Base_Rand()
{
	int nLength = m_arrAttackers.GetElementCount();
	for( int x = 0; x < nLength; x++ ) {
		delete m_arrAttackers[x];
	}
}

// Spawn the specified amount of units and attack a random base the player owns.
void EventScript_Spawn_Attackto_Base_Rand::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	// Temporarily store attacker ids.
	Array<long> attackers;

	// Gather the type and number of units to spawn.
	int numSpawn1 = 0;
	int numSpawn2 = 0;
	int numSpawn3 = 0;
	unsigned long type1 = 0;
	unsigned long type2 = 0;
	unsigned long type3 = 0;

	int team = Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, 'TEAM')) );
	if( team == 1 ) {
		type1 = 'SUBM';
		type2 = 'BSHP';
		type3 = 'SCUB';	
	}
	else {
		type1 = 'MERM';
		type2 = 'TURT';
		type3 = 'MANT';	
	}

	numSpawn1 += Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, type1)) );
	numSpawn2 += Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, type2)) );
	numSpawn3 += Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, type3)) );

	int numTotalSpawn = numSpawn1 + numSpawn2 + numSpawn3;

	float posx =  Text::StringToFloat( m_pScriptTable->GetString(StringID(m_ulID, 'POSX')) );
	float posy =  Text::StringToFloat( m_pScriptTable->GetString(StringID(m_ulID, 'POSY')) );

	Point3D position( posx, posy, 0.0F );

	Array<Point3D> safe_points;
	float radius = BUFFER_DIST;
	PointFinder::FindSafePoint_Spiral( position, numTotalSpawn, safe_points, radius, BUFFER_DIST, K::pi_over_20, MAX_RADIUS, 4.0F, Math::RandomFloat( K::two_pi ), false );
	if( safe_points.GetElementCount() != numTotalSpawn ) {
		return;
	}

	for( int i = 0; i < numSpawn1; i++ ) {
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		Point3D pos = safe_points[i];

		TheMessageMgr->SendMessageAll(CreateUnitMessage(kModelMessageUnit, index, type1, pos, 0.0F, 0.0F,
			K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, team, -1));

		attackers.AddElement( index );
	}

	for( int i = 0; i < numSpawn2; i++ ) {
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		Point3D pos = safe_points[i + numSpawn1];

		TheMessageMgr->SendMessageAll(CreateUnitMessage(kModelMessageUnit, index, type2, pos, 0.0F, 0.0F,
			K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, team, -1));

		attackers.AddElement( index );
	}

	for( int i = 0; i < numSpawn3; i++ ) {
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		Point3D pos = safe_points[i + numSpawn1 + numSpawn2];

		TheMessageMgr->SendMessageAll(CreateUnitMessage(kModelMessageUnit, index, type3, pos, 0.0F, 0.0F,
			K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, team, -1));

		attackers.AddElement( index );
	}

	// Pick a random base and attack to that position.
	int playerTeam = 3 - team;
	Array<long> *bases = TheGameCacheMgr->GetGameCache(kCacheBases, playerTeam);
	if( !bases ) {
		return;
	}

	int randb = Math::Random( bases->GetElementCount() );
	BTEntityController *bc = static_cast<BTEntityController*>( world->GetController( (*bases)[randb] ) );
	if( !bc ) {
		return;
	}

	Point3D basePos = bc->GetPosition();

	for( int i = 0; i < numTotalSpawn; i++ ) {
		UnitController *ent = static_cast<UnitController*>( world->GetController(attackers[i]) );
		if( ent ) {
			m_arrAttackers.AddElement( new Link<BTEntityController>( ent ) );
			ent->AddGoal( new BTGoalAttackTo(ent, basePos) );
		}
	}
}

// If all spawn units are dead, this script is over.
// Return true if this script is done, false otherwise.
bool EventScript_Spawn_Attackto_Base_Rand::CheckConditions( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return true;
	}

	int length = m_arrAttackers.GetElementCount();
	for( int x = 0; x < length; x++ ) {
		// If the controller exists, this script is still active.
		if( *m_arrAttackers[x] ) {
			return false;
		}
	}

	return true;
}


////// EventScript_SpawnUnits

EventScript_SpawnUnits::EventScript_SpawnUnits( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_SpawnUnits::~EventScript_SpawnUnits()
{
}

// Spawn the specified amount of units.
void EventScript_SpawnUnits::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	// Gather the type and number of units to spawn.
	int numSpawn1 = 0;
	int numSpawn2 = 0;
	int numSpawn3 = 0;
	unsigned long type1 = 0;
	unsigned long type2 = 0;
	unsigned long type3 = 0;

	int team = Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, 'TEAM')) );
	if( team == 1 ) {
		type1 = 'SUBM';
		type2 = 'BSHP';
		type3 = 'SCUB';	
	}
	else {
		type1 = 'MERM';
		type2 = 'TURT';
		type3 = 'MANT';	
	}

	numSpawn1 += Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, type1)) );
	numSpawn2 += Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, type2)) );
	numSpawn3 += Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, type3)) );

	int numTotalSpawn = numSpawn1 + numSpawn2 + numSpawn3;

	float posx =  Text::StringToFloat( m_pScriptTable->GetString(StringID(m_ulID, 'POSX')) );
	float posy =  Text::StringToFloat( m_pScriptTable->GetString(StringID(m_ulID, 'POSY')) );

	Point3D position( posx, posy, 0.0F );

	Array<Point3D> safe_points;
	float radius = BUFFER_DIST;
	PointFinder::FindSafePoint_Spiral( position, numTotalSpawn, safe_points, radius, BUFFER_DIST, K::pi_over_20, MAX_RADIUS, 4.0F, Math::RandomFloat( K::two_pi ), false );
	if( safe_points.GetElementCount() != numTotalSpawn ) {
		return;
	}

	for( int i = 0; i < numSpawn1; i++ ) {
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		Point3D pos = safe_points[i];

		TheMessageMgr->SendMessageAll(CreateUnitMessage(kModelMessageUnit, index, type1, pos, 0.0F, 0.0F,
			K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, team, -1));
	}

	for( int i = 0; i < numSpawn2; i++ ) {
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		Point3D pos = safe_points[i + numSpawn1];

		TheMessageMgr->SendMessageAll(CreateUnitMessage(kModelMessageUnit, index, type2, pos, 0.0F, 0.0F,
			K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, team, -1));
	}

	for( int i = 0; i < numSpawn3; i++ ) {
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		Point3D pos = safe_points[i + numSpawn1 + numSpawn2];

		TheMessageMgr->SendMessageAll(CreateUnitMessage(kModelMessageUnit, index, type3, pos, 0.0F, 0.0F,
			K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, team, -1));
	}
}

// This script is complete after Start() was called.
bool EventScript_SpawnUnits::CheckConditions( void )
{
	return true;
}


