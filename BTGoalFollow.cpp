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


#include "BTGoalFollow.h"
#include "BTGoalAttack.h"
#include "BTGoalMove.h"
#include "MGGame.h"
#include "BTMath.h"


#define MIN_DIST_TO_ENEMY 10.0F
#define DIST_TO_ENEMY_MULT 1.5F

#define DIST_TO_FOLLOW 10.0F

using namespace C4;


BTGoalFollow::BTGoalFollow( BTEntityController *pOwner, BTEntityController *pFollow ) :
	BTGoal( kGoalTypeFollow ),
	m_pOwner( pOwner ),
	m_pFollow( pFollow ),
	m_pCurEnemy( nullptr )
{

}

BTGoalFollow::~BTGoalFollow()
{

}

void BTGoalFollow::Enter( void )
{
	BTGoal::Enter();
}

void BTGoalFollow::Exit( void )
{
	BTGoal::Exit();
}

// If out of range of commander, move to commander.
// If within range and there is an enemy, attack the enemy.
long BTGoalFollow::Process( void )
{
	BTGoal::Process();

	if( m_lStatus != kGoalInProgress ) {
		return m_lStatus;
	}

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return m_lStatus;
	}

	if( !m_pFollow ) {
		m_lStatus = kGoalCompleted;
		return m_lStatus;
	}

	// If we have strayed too far, go back.
	if( !InRangeOfFollow() ) {		

		// ignore subgoal move (it means this entity is already heading back)
		if( !HasSubgoals() || m_arrSubGoals[0]->GetGoalType() != kGoalTypeMove ) {
			RemoveAllSubgoals();
			m_pCurEnemy = nullptr;
			AddSubgoal( new BTGoalMove(m_pFollow->GetPosition(), static_cast<UnitController*>(m_pOwner.GetTarget())) );
		}
	}
	else {
		// if nothing is active, search for enemies
		if( !HasSubgoals() ) {
			CheckForEnemies();
		}
	}

	// Process subgoals
	if( HasSubgoals() ) {
		long status = m_arrSubGoals[0]->Process();

		if( status == kGoalFailed || status == kGoalCompleted ) {
			RemoveSubgoal( m_arrSubGoals[0] );
		}
	}

	return m_lStatus;
}

bool BTGoalFollow::InRangeOfFollow( void )
{
	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return true;
	}

	if( !m_pFollow ) {
		m_lStatus = kGoalCompleted;
		return true;
	}

	if( Calculate2DDistance(m_pOwner->GetPosition(), m_pFollow->GetPosition()) > DIST_TO_FOLLOW ) {
		return false;
	}

	return true;
}

void BTGoalFollow::CheckForEnemies( void )
{
	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	// If there is an enemy in range, attack it.
	float closestEnemy = K::infinity;
	float minDistToEnemy = Fmax( MIN_DIST_TO_ENEMY, m_pOwner->GetStats()->GetAttackRange() * DIST_TO_ENEMY_MULT );
	int enemyTeam = 3 - m_pOwner->GetTeam();
	Array<long> rIgnoreConts;
	BTCollisionData data;
	TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheUnits, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	TheGame->DetectEntity(m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheCommanders, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	if( data.m_State == kCollisionStateNone ) {
		TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheBases, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	}

	if( data.m_State == kCollisionStateCollider && data.m_pController != nullptr && m_pOwner->CanAttack(data.m_pController) ) {
		if( data.m_pController != m_pCurEnemy ) {
			RemoveAllSubgoals();
			m_pCurEnemy = data.m_pController;
			AddSubgoal( new BTGoalAttack(data.m_pController, m_pOwner) );
		}
	}
}

