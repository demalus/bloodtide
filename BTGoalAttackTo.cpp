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


#include "BTGoalAttackTo.h"
#include "BTGoalAttack.h"
#include "BTGoalMove.h"
#include "MGGame.h"
#include "BTMath.h"


#define MIN_DIST_TO_ENEMY 15.0F
#define DIST_TO_ENEMY_MULT 1.5F


using namespace C4;


BTGoalAttackTo::BTGoalAttackTo( BTEntityController *pOwner, Point3D p3dTarget ) :
	BTGoal( kGoalTypeAttackTo ),
	m_pOwner( pOwner ),
	m_p3dTarget( p3dTarget ),
	m_pCurEnemy( nullptr )
{

}

BTGoalAttackTo::~BTGoalAttackTo()
{

}

void BTGoalAttackTo::Enter( void )
{
	BTGoal::Enter();
}

void BTGoalAttackTo::Exit( void )
{
	BTGoal::Exit();
}

// Processes only the first subgoal.
long BTGoalAttackTo::Process( void )
{
	BTGoal::Process();

	if( m_lStatus != kGoalInProgress ) {
		return m_lStatus;
	}

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return m_lStatus;
	}

	if( HasSubgoals() ) {
		GoalType gt = m_arrSubGoals[0]->GetGoalType();
		long status = m_arrSubGoals[0]->Process();

		if( status == kGoalFailed || status == kGoalCompleted ) {
			RemoveAllSubgoals();

			if( gt == kGoalTypeMove ) {
				m_lStatus = kGoalCompleted;
				return m_lStatus;
			}
		}
	}

	EvaluateConditions();

	return m_lStatus;
}

void BTGoalAttackTo::EvaluateConditions()
{
	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	// If this unit has reached the attack to position, then this goal is complete.
	if( m_pOwner->CloseToPosition2D( m_p3dTarget ) ) {
		m_lStatus = kGoalCompleted;
		return;
	}

	// If there is an enemy in range, attack it.
	float closestEnemy = K::infinity;
	float minDistToEnemy = Fmax( MIN_DIST_TO_ENEMY, m_pOwner->GetStats()->GetAttackRange() * DIST_TO_ENEMY_MULT );
	int enemyTeam = 3 - m_pOwner->GetTeam();
	Array<long> rIgnoreConts;
	BTCollisionData data;
	TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheUnits, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheCommanders, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	if( data.m_State == kCollisionStateNone ) {
		TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheBuildings, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false ); 
	}
	if( data.m_State == kCollisionStateNone ) {
		TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheBases, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false ); 
	}

	if( data.m_State == kCollisionStateCollider && data.m_pController != nullptr && m_pOwner->CanAttack(data.m_pController) ) {
		if( !HasSubgoals() || m_arrSubGoals[0]->GetGoalType() != kGoalTypeAttack ) {
			RemoveAllSubgoals();
			AddSubgoal( new BTGoalAttack(data.m_pController, m_pOwner) );
			m_pCurEnemy = data.m_pController;
		}
		else if( data.m_pController != m_pCurEnemy ) {
			RemoveAllSubgoals();
			AddSubgoal( new BTGoalAttack(data.m_pController, m_pOwner) );
			m_pCurEnemy = data.m_pController;
		}
	}
	else {
		if( !HasSubgoals() || m_arrSubGoals[0]->GetGoalType() != kGoalTypeMove ) {
			RemoveAllSubgoals();
			AddSubgoal( new BTGoalMove(m_p3dTarget, static_cast<UnitController*>(m_pOwner.GetTarget())) );
		}
	}
}

