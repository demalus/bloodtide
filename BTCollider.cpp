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


#include "BTCollider.h"
#include "BTMath.h"

#define HEADING_DIFF_THRESHOLD K::pi_over_6

using namespace C4;

// -------------- BTCollider ------------------

BTCollider::BTCollider( BTColliderType ulType, Point3D p3dPosition, BTEntityController *pController ) :
	m_ulType( ulType ),
	m_p3dPosition( p3dPosition ),
	m_pOwner( pController )
{
}

// -------------- BTCylinderCollider -------------

BTCylinderCollider::BTCylinderCollider( Point3D p3dPosition, BTEntityController *pController, float fRadius, float fHeight ) :
	BTCollider( kBTColliderCylinder, p3dPosition, pController ),
	m_fRadius( fRadius ),
	m_fHeight( fHeight )
{
}

BTCylinderCollider::~BTCylinderCollider()
{

}

// TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Right now, the 3d collision checks radius as height...change this to be radius for x and y but height for z!

/*
 * Check to see if this collider collides with a given point.
 * @author - Frank Williams (demalus@gmail.com)
 */
bool BTCylinderCollider::DetectCollision( Point3D& p3dToTest, BTCollisionData& stData, float& fSafeDist, bool bIs3D ) 
{
	Vector3D toPoint = p3dToTest - m_p3dPosition;
	float mag = 0.0F;
	if( bIs3D ) {
		mag = Magnitude( toPoint );
		if( mag >= m_fRadius + fSafeDist ) {
			return false;
		}
	}
	else {
		mag = Magnitude2D( toPoint );
		if( mag >= m_fRadius + fSafeDist ) {
			return false;
		}
	}

	stData.m_pController = m_pOwner;
	stData.m_State = kCollisionStateCollider;
	stData.m_Distance = Fabs(mag);
	stData.m_v3dNormal = toPoint;
	if( Magnitude(toPoint) > 0 ) {
		stData.m_v3dNormal = (toPoint.Normalize() * -1.0F);
	}
	return true;
}

/*
 * Check to see if this collider collides with some other collider.
 * The normal of the collision is relative to the given collider.
 * @author - Frank Williams (demalus@gmail.com)
 */
void BTCylinderCollider::DetectCollision( BTCollider *pCollider, BTCollisionData& stData, bool bIs3D ) 
{
	switch( pCollider->GetType() ) {

		case kBTColliderCylinder:
		{
			BTCylinderCollider *col = static_cast<BTCylinderCollider*>(pCollider);
			if( !col ) {
				return;
			}

			// Check to make sure the two are within distance of each other.
			Vector3D toCol = col->GetPosition() - m_p3dPosition;
			float mag = 0.0F;
			
			if( bIs3D ) {
				mag = Magnitude( toCol );
				if( mag >= (m_fRadius + col->GetRadius()) ) {
					break;
				}
			}
			else {
				mag = Magnitude2D( toCol );
				if( mag >= (m_fRadius + col->GetRadius()) ) {
					break;
				}
			}

			stData.m_pController = col->GetController();
			stData.m_State = kCollisionStateCollider;
			stData.m_Distance = Fabs(mag);
			stData.m_v3dNormal = (toCol.Normalize() * -1.0F);
			break;
		}

		default:

			break;
	}
}

/*
 * Check to see if this collider collides with some other collider
 * with some extra space.  This is useful for checking collisions before
 * they occur.
 * The normal of the collision is relative to the given collider.
 * @author - Frank Williams (demalus@gmail.com)
 */
void BTCylinderCollider::DetectCollisionAhead( BTCollider *pCollider, BTCollisionData& stData, float& fDistAhead, float& fHeading, bool bIs3D ) 
{
	switch( pCollider->GetType() ) {

		case kBTColliderCylinder:
		{
			BTCylinderCollider *col = static_cast<BTCylinderCollider*>( pCollider );
			if( !col ) {
				break;
			}

			// Check to make sure the two are within distance of each other.
			Vector3D toCol = col->GetPosition() - m_p3dPosition;
			float mag = 0.0F;

			if( bIs3D ) {
				mag = Magnitude( toCol );
				if( mag >= (m_fRadius + col->GetRadius() + fDistAhead) ) {
					break;
				}
			}
			else {
				mag = Magnitude2D( toCol );
				if( mag >= (m_fRadius + col->GetRadius() + fDistAhead) ) {
					break;
				}
			}

			// Test to see if a unit collides with the source unit in front of the source.
			// To do this, get three points:
			//  1) Center of test unit
			//  2&3) Points perpendicular to the vector from source to test unit out as far as the test unit's collider goes.

			// Get angle from source to dest
			float n_heading = atan2f( toCol.y, toCol.x );
			NormalizeAngle( n_heading );

			// Get perpendicular points (source to dest)
			Vector3D perp = toCol;
			perp.Normalize();
			perp.RotateAboutZ(K::pi_over_2);
			perp *= col->GetRadius();
			Vector3D normal_lp = (col->GetPosition() + perp) - m_p3dPosition;
			perp.RotateAboutZ(K::pi);
			Vector3D normal_rp = (col->GetPosition() + perp) - m_p3dPosition;

			float n_lp_heading = atan2f( normal_lp.y, normal_lp.x );
			NormalizeAngle( n_lp_heading );

			float n_rp_heading = atan2f( normal_rp.y, normal_rp.x );
			NormalizeAngle( n_rp_heading );

			// The difference between the angles needs to be small (the angle to the 
			// test unit needs to be within some distance of the current heading to 
			// be considered in front of the source unit).
			float diff = Fabs(fHeading - n_heading);
			if( diff > K::pi ) {
				diff = Fabs(K::two_pi - diff);
			}

			float diff_lp = Fabs(fHeading - n_lp_heading);
			if( diff_lp > K::pi ) {
				diff_lp = Fabs(K::two_pi - diff_lp);
			}

			float diff_rp = Fabs(fHeading - n_rp_heading);
			if( diff_rp > K::pi ) {
				diff_rp = Fabs(K::two_pi - diff_rp);
			}


			if( diff > HEADING_DIFF_THRESHOLD &&
				diff_lp > HEADING_DIFF_THRESHOLD &&
				diff_rp > HEADING_DIFF_THRESHOLD ) 
			{
				break;
			}

			stData.m_pController = col->GetController();
			stData.m_State = kCollisionStateCollider;
			stData.m_Distance = Fabs(mag);
			stData.m_v3dNormal = (toCol.Normalize() * -1.0F);

			break;
		}
		default:

			break;
	}
}

