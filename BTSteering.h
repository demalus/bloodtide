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


#ifndef BTSteering_h
#define BTSteering_h

#include "C4Vector3D.h"
#include "C4Link.h"

namespace C4
{

	// Forward declaration
	class BTEntityController;
	struct CollisionData;

	/** 
	 * This class represents a mechanism that adjusts the heading 
	 * of some entity and its position. It uses logic to do 
	 * different behaviors so entities can move around in many
	 * different ways.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTSteering
	{
		private:

			// Controller this steering object is attached to.
			BTEntityController *m_pOwner;

			// Target to move to.
			Point3D m_p3dTarget;

			// Intermediate target for avoiding obstacles.
			Point3D m_p3dIntermediateTarget;
			bool m_bIntermediateTarget;
			Link<BTEntityController> m_pAvoidTarget;

			// Impulse used for separation force.
			Vector3D m_v3dSepImpulse;

			// Speed that the unit moves at (1.0F ~= 1m).
			float m_fSpeed;

			// Units should appear to turn instead of just face where they are going.
			// This is the angle that can be turned per tick in radians.
			float m_fMaxTurnRadius;

			// Some units can move in any direction, regardless of whether or not
			// they are facing that direction.
			bool m_bMoveWithoutHeading;

			// Some units need to turn by moving forward in a U shape.
			bool m_bMoveForwardTurn;

			// Whether or not the object can move along the z axis.
			bool m_bMovesIn3D;

			// Various steering behaviors to use:
			bool m_bSeekOn;
			bool m_bAvoidOn;
			bool m_bInterpOn;
			bool m_bSeparationOn;

			bool TrySeek( const Point3D& p3dCurrent, Point3D& p3dTarget, float& fAzimuth, float &fMoveDir, float &fSpeed );
			bool TryAvoid( const Point3D& p3dCurrent, Point3D& p3dTarget, float &fMoveDir );
			bool TrySeparation( Vector3D& sepImpulse );

			bool IsOriginalPathClear( const Point3D& p3dCurrent, Point3D& p3dTarget, float fMoveDir );

		public:

			BTSteering( BTEntityController *pOwner );
			~BTSteering();

			// Set location to move to.
			void SetTarget( Point3D p3dTarget );

			const Point3D GetTarget( void ) const
			{
				return ( m_p3dTarget );
			}

			// Set the speed that this unit moves at (1.0F ~= 1m).
			void SetSpeed( float fSpeed )
			{
				m_fSpeed = fSpeed;
			}

			const float GetSpeed( void ) const
			{
				return ( m_fSpeed );
			}

			// Set max radians that can be turned per tick.
			void SetMaxTurnRadius( float fTurnRadius )
			{
				m_fMaxTurnRadius = fTurnRadius;
			}

			const float GetMaxTurnRadius( void ) const
			{
				return ( m_fMaxTurnRadius );
			}

			// Set whether or not units can move when not facing target.
			void SetMoveWithoutHeading( bool bMoveWithoutHeading )
			{
				m_bMoveWithoutHeading = bMoveWithoutHeading;
			}

			bool CanMoveWithoutHeading( void ) const
			{
				return ( m_bMoveWithoutHeading );
			}

			// Set whether or not this unit should move forward
			// while turning.
			void SetMoveForwardTurn( bool bMoveForwardTurn )
			{
				m_bMoveForwardTurn = bMoveForwardTurn;
			}

			bool CanMoveForwardWhileTurning( void ) const
			{
				return ( m_bMoveForwardTurn );
			}

			// Toggle the seeking behavior on or off.
			void SetSeekOn( bool bSeek )
			{
				m_bSeekOn = bSeek;
			}

			bool GetSeekOn( void ) const
			{
				return ( m_bSeekOn );
			}

			// Toggle the avoidance (of other units) behavior on or off.
			void SetAvoidOn( bool bAvoid )
			{
				m_bAvoidOn = bAvoid;
			}

			bool GetAvoidOn( void ) const
			{
				return ( m_bAvoidOn );
			}

			// Toggle the interpolation behavior on or off.
			void SetInterpOn( bool bAvoid )
			{
				m_bInterpOn = bAvoid;
			}

			bool GetInterpOn( void ) const
			{
				return ( m_bInterpOn );
			}

			// Toggle the separation behavior on or off.
			void SetSeparationOn( bool bSeparation )
			{
				m_bSeparationOn = bSeparation;
			}

			bool GetSeparationOn( void ) const
			{
				return ( m_bSeparationOn );
			}

			// Toggle 3D movement on or off.
			void Set3DMovement( bool b3D )
			{
				m_bMovesIn3D = b3D;
			}

			bool GetMovesIn3D( void ) const
			{
				return ( m_bMovesIn3D );
			}

			// A call to this method will generate a new position and azimuth
			// for this unit based upon the steering behaviors.
			// Returns true if something was updated, false otherwise.
			bool CalculateMove( const Point3D& p3dPosition, float& fAzimuth, Vector3D& v3dVelocity );
	};

}

#endif
