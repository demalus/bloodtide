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


#include "BTGoalAttack.h"
#include "BTUnit.h"
#include "MGGame.h"
#include "C4Vector3D.h"
#include "BTMath.h"
#include "BTGoalMove.h"

#define BUFFER_DIST_MULT	1.5F
#define AZI_THRESHOLD		0.002F

using namespace C4;

BTGoalAttack::BTGoalAttack( BTEntityController *pTarget, BTEntityController *pOwner ) :
	BTGoal( kGoalTypeAttack ),
	m_pTarget( pTarget ),
	m_pOwner( pOwner )
{

}

BTGoalAttack::~BTGoalAttack()
{

}

void BTGoalAttack::Enter( void )
{
	BTGoal::Enter();

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	if( !m_pTarget || !m_pOwner->CanAttack( m_pTarget ) ) {
		m_lStatus = kGoalCompleted;
		return;
	}

	if( m_pOwner->GetTeam() == m_pTarget->GetTeam() ) {
		m_lStatus = kGoalFailed;
		return;
	}
}

void BTGoalAttack::Exit( void )
{
	BTGoal::Exit();
}

long BTGoalAttack::Process( void )
{
	BTGoal::Process();

	if( m_lStatus != kGoalInProgress ) {
		return m_lStatus;
	}

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return m_lStatus;
	}

	if( !m_pTarget || !m_pOwner->CanAttack( m_pTarget ) ) {
		m_lStatus = kGoalCompleted;
		return m_lStatus;
	}

	if( m_pOwner->GetTeam() == m_pTarget->GetTeam() ) {
		m_lStatus = kGoalFailed;
		return m_lStatus;
	}

	if( AttackValid() ) {
		RemoveAllSubgoals();

		// Calculate altitude.
		Vector3D toEnemy = m_pTarget->GetPosition() - m_pOwner->GetPosition();
		float azi = atan2f( toEnemy.y, toEnemy.x );
		toEnemy.RotateAboutZ( -azi );
		float alt = atan2f( toEnemy.z, toEnemy.x );
		m_pOwner->SetTargetAltitude( -alt );

		// Perform attack.
		m_pOwner->PerformAbility( kMotionAbility1 );
	}
	else {
		if( !HasSubgoals() ) {

			// Go to a position just in front of the target to "chase".
			Vector3D normal = m_pTarget->GetPosition() - m_pOwner->GetPosition();

			float distFromTarg = 0.0F;
			BTCylinderCollider *col = static_cast<BTCylinderCollider*>( m_pOwner->GetCollider() );
			if( col ) {
				distFromTarg = col->GetRadius() * BUFFER_DIST_MULT;
			}

			normal.Normalize();
			normal *= ( distFromTarg );

			Point3D targetPos = m_pOwner->GetPosition() + normal;
			targetPos.z = m_pOwner->GetPosition().z;

			AddSubgoal( new BTGoalMove( targetPos, static_cast<UnitController*>(m_pOwner.GetTarget()), 1.0F ) );
		}

		long status = m_arrSubGoals[0]->Process();

		if( status == kGoalFailed || status == kGoalCompleted ) {
			RemoveSubgoal( m_arrSubGoals[0] );
		}
	}


	return m_lStatus;
}

// Figure out if we are close enough and facing the right direction to attack the enemy.
bool BTGoalAttack::AttackValid( void )
{
	if( !m_pOwner || !m_pTarget ) {
		return false;
	}

	if( !m_pOwner->CanAttack( m_pTarget ) ) {
		return false;
	}

	float col_dist = 0.0F;
	BTCylinderCollider *col = static_cast<BTCylinderCollider*>( m_pTarget->GetCollider() );
	if( col ) {
		col_dist = col->GetRadius();
	}

	Point3D sPos = m_pOwner->GetPosition();
	Point3D ePos = m_pTarget->GetPosition();
	float dist = Calculate2DDistance( sPos, ePos );
	
	float range = m_pOwner->GetStats()->GetAttackRange() + col_dist;

	if( dist > range || dist <= col_dist ) {
		return false;
	}

	Vector3D toTarget = ePos - sPos;
	float desiredHeading = atan2f(toTarget.y, toTarget.x);
	NormalizeAngle(desiredHeading);

	if( Fabs(m_pOwner->GetAzimuth() - desiredHeading) > AZI_THRESHOLD ) {
		m_pOwner->SetAzimuth( desiredHeading );
		RemoveAllSubgoals();
	}

	return true;
}

