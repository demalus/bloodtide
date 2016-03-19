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


#include "BTGoalMove.h"
#include "C4Vector3D.h"
#include "BTUnit.h"
#include "MGGame.h"
#include "PointFinder.h"

#include "BTGoalAttack.h"

using namespace C4;

BTGoalMove::BTGoalMove( Point3D& p3dTarget, UnitController *pOwner, float fMaxRadiusMult ) :
	BTGoal( kGoalTypeMove ),
	m_p3dTarget( p3dTarget ),
	m_pOwner( pOwner ),
	m_p3dInitialTarget( p3dTarget ),
	m_fMaxRadiusMult( fMaxRadiusMult )
{

}

BTGoalMove::~BTGoalMove()
{

}

void BTGoalMove::Enter( void )
{
	BTGoal::Enter();

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	m_pOwner->GetSteering()->SetAvoidOn( true );
	m_pOwner->GetSteering()->SetSeekOn( true );
	m_pOwner->GetSteering()->SetTarget( m_p3dTarget );
}

void BTGoalMove::Exit( void )
{
	BTGoal::Exit();

	if( !m_pOwner ) {
		return;
	}

	m_pOwner->GetSteering()->SetAvoidOn( false );
	m_pOwner->GetSteering()->SetSeekOn( false );
}

long BTGoalMove::Process( void )
{
	BTGoal::Process();

	if( m_lStatus != kGoalInProgress ) {
		return m_lStatus;
	}

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
	}

	EvaluateTarget();

	return m_lStatus;
}

void BTGoalMove::EvaluateTarget( void )
{
	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	if( m_pOwner->CloseToPosition2D( m_p3dTarget ) ) {
		m_lStatus = kGoalCompleted;
		return;
	}

	BTCylinderCollider *col = static_cast<BTCylinderCollider*>( m_pOwner->GetCollider() );
	if( !col ) {
		return;
	}

	// Distance from any unit that won't result in a collision.
	float safe_dist = col->GetRadius() * UNIT_SAFE_DIST_MULT;

	// See if there is a unit on the spot we want to go.
	BTCollisionData data;
	Array<long> ignoreConts;
	ignoreConts.AddElement( m_pOwner->GetControllerIndex() );
	TheGame->DetectEntity( m_p3dTarget, data, kCacheUnits | kCacheCommanders, safe_dist, ignoreConts, false );
	if( data.m_State == kCollisionStateNone ) {
		return;
	}

	if( !data.m_pController ) {
		return;
	}

	// If it's moving, then the spot is still OK.
	if( data.m_pController->GetAnimation() == kMotionMove ) {
		return;
	}

	// Try to find a new target.

	BTCylinderCollider *collider = static_cast<BTCylinderCollider*>( data.m_pController->GetCollider() );
	if( !collider ) {
		m_lStatus = kGoalFailed;
		return;
	}
	
	Vector3D toCol = data.m_pController->GetPosition() - m_pOwner->GetPosition();
	float preferredAngle = atan2f( toCol.y, toCol.x );
	NormalizeAngle( preferredAngle );

	float radius = collider->GetRadius();

	Array<Point3D> safe_points;
	PointFinder::FindSafePoint_Spiral( m_p3dInitialTarget, 1, safe_points, radius + safe_dist + ENT_AVOID_DIST, radius, K::pi_over_6, m_fMaxRadiusMult * radius, safe_dist, preferredAngle, false );

	if( safe_points.GetElementCount() < 1 ) {
		m_lStatus = kGoalFailed;
		return;
	}

	m_p3dTarget = safe_points[0];
	m_pOwner->GetSteering()->SetTarget( m_p3dTarget );
}
