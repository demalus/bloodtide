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


#include "BTGoalDefend.h"
#include "BTGoalAttack.h"
#include "BTGoalMove.h"
#include "MGGame.h"
#include "BTMath.h"


#define MIN_DIST_TO_ENEMY 10.0F
#define DIST_TO_ENEMY_MULT 1.5F

using namespace C4;


BTGoalDefend::BTGoalDefend( BTEntityController *pOwner, Point3D p3dInitPosition ) :
	BTGoal( kGoalTypeDefend ),
	m_pOwner( pOwner ),
	m_p3dInitPosition( p3dInitPosition ),
	m_pCurEnemy( nullptr )
{

}

BTGoalDefend::~BTGoalDefend()
{

}

void BTGoalDefend::Enter( void )
{
	BTGoal::Enter();
}

void BTGoalDefend::Exit( void )
{
	BTGoal::Exit();
}

// Processes only the first subgoal.
long BTGoalDefend::Process( void )
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

		long status = m_arrSubGoals[0]->Process();

		if( status == kGoalFailed || status == kGoalCompleted ) {
			RemoveSubgoal( m_arrSubGoals[0] );
			m_pCurEnemy = nullptr;
		}
	}
	else {
		m_p3dInitPosition = m_pOwner->GetPosition();
	}

	EvaluateConditions();

	return m_lStatus;
}

void BTGoalDefend::EvaluateConditions()
{
	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	if( HasSubgoals() ) {

		// If we have strayed too far, go back.
		if( m_arrSubGoals[0]->GetGoalType() != kGoalTypeMove ) {	// ignore subgoal move (it means this entity is headed back)
			if( Calculate2DDistance(m_pOwner->GetPosition(), m_p3dInitPosition) > Fmax( MIN_DIST_TO_ENEMY, m_pOwner->GetStats()->GetAttackRange() * DIST_TO_ENEMY_MULT ) ) {
				RemoveAllSubgoals();
				m_pCurEnemy = nullptr;
				AddSubgoal( new BTGoalMove(m_p3dInitPosition, static_cast<UnitController*>(m_pOwner.GetTarget())) );
			}
		}

		return;
	}

	// If there is an enemy in range, attack it.
	float closestEnemy = K::infinity;
	float minDistToEnemy = Fmax( MIN_DIST_TO_ENEMY, m_pOwner->GetStats()->GetAttackRange() * DIST_TO_ENEMY_MULT );
	int enemyTeam = 3 - m_pOwner->GetTeam();
	Array<long> rIgnoreConts;
	BTCollisionData data;
	TheGame->DetectEntity( m_p3dInitPosition, data, TheGameCacheMgr->GetGameCache(kCacheUnits, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	TheGame->DetectEntity( m_p3dInitPosition, data, TheGameCacheMgr->GetGameCache(kCacheCommanders, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	if( data.m_State == kCollisionStateNone ) {
		TheGame->DetectEntity( m_p3dInitPosition, data, TheGameCacheMgr->GetGameCache(kCacheBases, enemyTeam), closestEnemy, minDistToEnemy, rIgnoreConts, false );
	}

	if( data.m_State == kCollisionStateCollider && data.m_pController != nullptr && m_pOwner->CanAttack(data.m_pController) ) {
		if( data.m_pController != m_pCurEnemy ) {
			RemoveAllSubgoals();
			m_pCurEnemy = data.m_pController;
			AddSubgoal( new BTGoalAttack(data.m_pController, m_pOwner) );
		}
	}
}

