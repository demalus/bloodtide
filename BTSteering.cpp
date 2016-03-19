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

#include "BTSteering.h"
#include "BTMath.h"
#include "C4Constants.h"
#include "BTCollider.h"
#include "MGGame.h"
#include "C4Collision.h"
#include "PointFinder.h"

#define CLOSE_TO_HEADING	0.1F
#define LOOK_AHEAD_MULT		3.0F
#define NUDGE_DISTANCE		2.0F	// 2 meters/sec

using namespace C4;

BTSteering::BTSteering( BTEntityController *pOwner) :
	m_pOwner( pOwner ),
	m_p3dTarget(0,0,0),
	m_fSpeed( 0.0F ),
	m_fMaxTurnRadius( 0.0F ),
	m_bMoveWithoutHeading( false ),
	m_bMoveForwardTurn( false ),
	m_bSeekOn( false ),
	m_bAvoidOn( false ),
	m_p3dIntermediateTarget(0,0,0),
	m_bIntermediateTarget( false ),
	m_pAvoidTarget( nullptr ),
	m_bInterpOn( false ),
	m_bSeparationOn( false ),
	m_v3dSepImpulse(0,0,0),
	m_bMovesIn3D( false )
{
}

BTSteering::~BTSteering()
{
}

// Updates the given position and azimuth for the unit based upon
// the current steering behaviors.
// This will only return false when motion should stop.  If moving or doing some intermediate 
// movement task, it will return true.
bool BTSteering::CalculateMove( const Point3D& p3dPosition, float& fAzimuth, Vector3D& v3dVelocity )
{
	if( !m_pOwner ) {
		return false;
	}

	// Exit early if no behaviors are active.
	if( !m_bSeekOn && !m_bAvoidOn && !m_bInterpOn && !m_bSeparationOn ) {
		return false;
	}

	float moveHeading = fAzimuth;	// Might not be the same as the azimuth of the model.
	float speed = m_fSpeed;			// Speed might be modified by a behavior.

	// Target is either avoid target or goal target.
	Point3D target;
	if( m_bIntermediateTarget ) {
		target = m_p3dIntermediateTarget;

		if( IsOriginalPathClear(p3dPosition, target, moveHeading) ) {
			m_bIntermediateTarget = false;
			m_pAvoidTarget = nullptr;
			target = m_p3dTarget;
		}

	}
	else {
		target = m_p3dTarget;
	}

	bool separate = TrySeparation(m_v3dSepImpulse);
	bool onTarget = m_pOwner->CloseToPosition( target );

	// Reset to normal target if the intermediate target has been reached.
	if( onTarget && m_bIntermediateTarget ) {
		m_bIntermediateTarget = false;
		m_pAvoidTarget = nullptr;
		m_pOwner->SetAnimation( kMotionMove );
		return true;
	}

	// Exit if on target.
	if( onTarget && !separate ) {
		return false;
	}

	// Try each steering behavior if not on the target:
	if( !onTarget ) {
		if( TrySeek(p3dPosition, target, fAzimuth, moveHeading, speed) ) return true;
		if( TryAvoid(p3dPosition, target, moveHeading) ) return true;
	}

	// Don't set the velocity if this entity isn't moving.
	if( !separate && !m_bSeekOn && !m_bAvoidOn && !m_bInterpOn ) {
		return false;
	}

	// Finalization
	NormalizeAngle( moveHeading );
	NormalizeAngle( fAzimuth );

	Vector3D moveOffset(0,0,0);
	if( m_bMovesIn3D ) {
		moveOffset = target - p3dPosition;
		moveOffset.Normalize();
		moveOffset *= speed;
	}
	else {
		moveOffset = Vector3D(1, 0, 0);  // Normalized vector rotated 0 radians.
		moveOffset *= speed;
		moveOffset.RotateAboutZ( moveHeading );
	}
	

	v3dVelocity = moveOffset + m_v3dSepImpulse;
	m_pOwner->SetAnimation( kMotionMove );

	m_v3dSepImpulse = Vector3D(0,0,0);
	return true;
}

// Set a new target and clear any temporary movement stuff.
void BTSteering::SetTarget( Point3D p3dTarget )
{
	m_p3dTarget = p3dTarget;
	m_bIntermediateTarget = false;
	m_pAvoidTarget = nullptr;
}

// If seek behavior set, apply forces.
// Return true if CalculateMove should end early, false otherwise.
bool BTSteering::TrySeek( const Point3D& p3dCurrent, Point3D& p3dTarget, float& fAzimuth, float &fMoveDir, float &fSpeed )
{
	if( !m_bSeekOn ) {
		return false;
	}

	if( !m_pOwner ) {
		return false;
	}

	if( m_pOwner->GetAnimation() == kMotionWaiting ) {
		fSpeed = 0.0F;
		return false;
	}

	// Get direction to target.
	Vector3D desired = (p3dTarget - p3dCurrent);
	desired.Normalize();

	// Figure out amount of rotation needed to turn towards target.
	float angleDesired = atan2f(desired.y, desired.x);
	NormalizeAngle( angleDesired );
	float angleToTurn = 0.0F;

	float calcAng = angleDesired - fAzimuth;
	NormalizeAngle( calcAng );
	if( Fabs(calcAng) <= K::pi ) {
		angleToTurn = calcAng;
	}
	else {
		angleToTurn = -(K::two_pi - calcAng);
	}

	// Turn either the full angle or the max angle per tick.
	if( Fabs(angleToTurn) > m_fMaxTurnRadius ) {
		fAzimuth += (angleToTurn / Fabs(angleToTurn)) * m_fMaxTurnRadius;
	}
	else {
		fAzimuth += angleToTurn;
	}

	// Cleanup the azimuth.
	NormalizeAngle( fAzimuth );

	// If the unit can't move without facing the target and can't move forward while turning,
	// and is not already facing near the target...then just update the azimuth (don't move).
	if( !m_bMoveWithoutHeading && !m_bMoveForwardTurn && Fabs(angleToTurn) > m_fMaxTurnRadius ) {
		m_pOwner->SetAnimation( kMotionMove );
		fSpeed = 0.0F;
		return true;
	}

	// Since the unit can move, update the speed and move direction.
	if( m_bMoveWithoutHeading ) {
		fMoveDir = angleDesired;
	}
	else {
		if( m_bMoveForwardTurn && Fabs(angleDesired - fMoveDir) > CLOSE_TO_HEADING ) {
			fSpeed *= 0.65F;
		}
	}

	return false;
}

// If avoid behavior set, apply forces necessary to avoid obstacles.
// Return true if CalculateMove should end early, false otherwise.
bool BTSteering::TryAvoid( const Point3D& p3dCurrent, Point3D& p3dTarget, float &fMoveDir )
{
	if( !m_bAvoidOn ) {
		return false;
	}

	if( !m_pOwner ) {
		return false;
	}

	BTCylinderCollider *myCol = static_cast<BTCylinderCollider*>( m_pOwner->GetCollider() );
	if( !myCol ) {
		return false;
	}

	// The look ahead distance should be based off of the collider if there is one.
	float radDist = myCol->GetRadius() * LOOK_AHEAD_MULT;
	float distToTarget = Fabs(Calculate2DDistance(p3dCurrent, p3dTarget));

	// If a collision occurs, find an intermediate point to travel to first before 
	// continuing on to the target.
	// This is done by finding the outermost point on the collided object's collider
	// radius perpendicular to our approach vector that would ensure a path around said object.

	BTCollisionData data;
	Array<long> ignoreConts;
	if( m_pAvoidTarget ) {
		ignoreConts.AddElement( m_pAvoidTarget->GetControllerIndex() );
	}
	TheGame->DetectEntityCollision( m_pOwner, data, kCacheUnits | kCacheCommanders, fMoveDir, radDist, ignoreConts );
	if( data.m_State == kCollisionStateCollider ) {
		if( distToTarget < data.m_Distance ) {
			return false;
		}
	
		if( !data.m_pController ) {
			return false;
		}

		BTCylinderCollider *tCol = static_cast<BTCylinderCollider*>( data.m_pController->GetCollider() );
		if( !tCol ) {
			return false;
		}

		// If the other unit is moving, be nice and wait for it to clear the area.
		float sdist = myCol->GetRadius() + tCol->GetRadius() + ENT_AVOID_DIST;
		if( data.m_pController->GetTeam() == m_pOwner->GetTeam() && data.m_pController->GetAnimation() == kMotionMove && data.m_Distance <= sdist ) {
			m_pOwner->SetAnimation( kMotionWaiting );
			return true;
		}

		Array<Point3D> arrSafePoints;
		float safeDist = myCol->GetRadius() + ENT_AVOID_DIST;
		PointFinder::FindSafePoint_Wall( m_pOwner, data.m_pController, arrSafePoints, safeDist, false );
		if( arrSafePoints.GetElementCount() < 1 ) {
			return false;
		}

		m_p3dIntermediateTarget = arrSafePoints[0];
		m_bIntermediateTarget = true;
		m_pAvoidTarget = data.m_pController;
		m_pOwner->SetAnimation( kMotionMove );
	}

	return false;
}

// Check to see if the original heading is now cleared of obstructions.
bool BTSteering::IsOriginalPathClear( const Point3D& p3dCurrent, Point3D& p3dTarget, float fMoveDir )
{
	BTCylinderCollider *myCol = static_cast<BTCylinderCollider*>( m_pOwner->GetCollider() );
	if( !myCol ) {
		return true;
	}

	// The look ahead distance should be based off of the collider if there is one.
	float radDist = myCol->GetRadius() * LOOK_AHEAD_MULT;
	float distToTarget = Fabs(Calculate2DDistance(p3dCurrent, p3dTarget));

	// If a collision occurs, find an intermediate point to travel to first before 
	// continuing on to the target.
	// This is done by finding the outermost point on the collided object's collider
	// radius perpendicular to our approach vector that would ensure a path around said object.

	BTCollisionData data;
	Array<long> ignoreConts;
	TheGame->DetectEntityCollision( m_pOwner, data, kCacheUnits | kCacheCommanders, fMoveDir, radDist, ignoreConts );
	if( data.m_State == kCollisionStateCollider ) {
		if( distToTarget < data.m_Distance ) {
			return true;
		}

		if( !data.m_pController ) {
			return true;
		}

		BTCylinderCollider *tCol = static_cast<BTCylinderCollider*>( data.m_pController->GetCollider() );
		if( !tCol ) {
			return true;
		}

		return false;
	}

	return true;
}

// If avoid separation behavior is set, units will float away from each other if they are too close.
bool BTSteering::TrySeparation( Vector3D& sepImpulse  )
{
	if( !m_bSeparationOn ) {
		return false;
	}

	if( !m_pOwner ) {
		return false;
	}

	BTCylinderCollider *myCol = static_cast<BTCylinderCollider*>( m_pOwner->GetCollider() );
	if( !myCol ) {
		return false;
	}

	BTCollisionData data;
	Array<long> ignoreConts;
	TheGame->DetectEntityCollision( m_pOwner, data, kCacheUnits | kCacheCommanders, 0.0F, 0.0F, ignoreConts );
	if( data.m_State == kCollisionStateCollider ) {
		sepImpulse = data.m_v3dNormal * NUDGE_DISTANCE;
		return true;
	}

	return false;
}

