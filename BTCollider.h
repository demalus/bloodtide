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


#ifndef BTCollider_h
#define BTCollider_h

#include "C4Collision.h"


namespace C4
{

	// Forward declaration.
	class BTEntityController;


	// The geometry type of the collider
	enum BTColliderType
	{
		kBTColliderCircle		= 0,
		kBTColliderCylinder
	};


	// Collision information
	typedef struct BTCollisionData
	{
		BTCollisionData() : m_State(kCollisionStateNone), m_v3dNormal(0,0,0), m_pController(nullptr) {};

		CollisionState		m_State;			// Whether or not something collided.
		Vector3D			m_v3dNormal;		// Reflective normal at point of collision (normalized).
		BTEntityController	*m_pController;		// Controller that was detected as part of a collision.
		float				m_Distance;			// Distance of the collision.

	} BTCollisionData;


	/** 
	 * This class represents a shape that acts as a collision
	 * bounds.  This greatly improves performance during 
	 * collision detection.  BTColliders are attached to controllers.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTCollider
	{
		protected:

			Point3D				m_p3dPosition;
			BTEntityController	*m_pOwner;
			BTColliderType		m_ulType;

		public:

			BTCollider( BTColliderType ulType, Point3D p3dPosition, BTEntityController *pController );

			virtual ~BTCollider() {}

			virtual bool DetectCollision( Point3D& p3dToTest, BTCollisionData& stData, float& fSafeDist, bool bIs3D = true ) { return false; }
			virtual void DetectCollision( BTCollider *pCollider, BTCollisionData& stData, bool bIs3D = true ) {}
			virtual void DetectCollisionAhead( BTCollider *pCollider, BTCollisionData& stData, float& fDistAhead, float& fHeading, bool bIs3D = true ) {}

			Point3D& GetPosition( void )
			{
				return (m_p3dPosition);
			}

			BTEntityController *GetController( void )
			{
				return (m_pOwner);
			}	

			BTColliderType GetType( void )
			{
				return (m_ulType);
			}
	};


	/** 
	 * This class represents a cylinder collider.
	 * It only needs a radius to determine collisions.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTCylinderCollider : public BTCollider
	{
		protected:

			float	m_fRadius;
			float	m_fHeight;

		public:

			BTCylinderCollider( Point3D p3dPosition, BTEntityController *pController, float fRadius, float fHeight );
			virtual ~BTCylinderCollider();

			virtual bool DetectCollision( Point3D& p3dToTest, BTCollisionData& stData, float& fSafeDist, bool bIs3D = true );
			virtual void DetectCollision( BTCollider *pCollider, BTCollisionData& stData, bool bIs3D = true );
			virtual void DetectCollisionAhead( BTCollider *pCollider, BTCollisionData& stData, float& fDistAhead, float& fHeading, bool bIs3D = true );

			float GetRadius( void )
			{
				return (m_fRadius);
			}

			void SetRadius( float fRadius )
			{
				m_fRadius = fRadius;
			}

			float GetHeight( void )
			{
				return (m_fHeight);
			}

			void SetHeight( float fHeight ) 
			{
				m_fHeight = fHeight;
			}
	};

}


#endif
