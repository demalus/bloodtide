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


#ifndef BTCharacter_h
#define BTCharacter_h


#include "MGCharacter.h"
#include "MGMultiplayer.h"
#include "StringTableManager.h"
#include "BTControllers.h"
#include "BTAbility.h"


namespace C4
{
	/** 
	 * This class represents a generic character. 
	 * Character's will have information taken from
	 * string tables.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTCharacterController : public BTEntityController, public ListElement<BTCharacterController>, public LinkTarget<BTCharacterController>
	{
		protected:

			unsigned long		m_lCharFlags;		// Character action flags

			// Animation
			FrameAnimator		*m_pFrameAnimator;

			// Positioning/Motion
			Vector3D			m_v3dVelocity;		// Current velocity
			float				m_fCharTime;		// Time falling
			Vector3D			m_v3dImpulse;		// Each step of movement is an impulse
			float				m_fFloatHeight;		// Distance in "air" which a unit floats
			Vector3D			m_v3dInitialGrav;	// Starting gravity

			unsigned long		m_ulKillTime;		// Time to finally delete a unit (after its death).

			void Initialize( void );

			FrameAnimator *GetFrameAnimator( void ) const
			{
				return (m_pFrameAnimator);
			}

			bool TestGround(const Point3D& p1, const Point3D& p2, float fRadius, CollisionData *data) const;

			void Premove();
			void Postmove();

		public:

			enum
			{
				kCharacterMessageBeginGround = kEntityMessageBaseCount,
				kCharacterMessageBeginSlide,
				kCharacterMessageEndGround,
				kCharacterMessageEndSlide1,
				kCharacterMessageEndSlide2,
				kCharacterMessageBeginMovement,
				kCharacterMessageEndMovement,
				kCharacterMessageBaseCount
			};

			BTCharacterController(ControllerType contType);
			BTCharacterController(ControllerType contType, EntityType lEntityType, unsigned long ulStringID, TableID lTableID, unsigned long flags, StatsProtocol *pStats, float fFloatHeight, long lTeam, float fOriMult);
			virtual ~BTCharacterController();

			using ListElement<BTCharacterController>::Previous;
			using ListElement<BTCharacterController>::Next;

			unsigned long GetCharFlags( void ) const
			{
				return (m_lCharFlags);
			}

			void SetCharFlags( unsigned long flags )
			{
				m_lCharFlags = flags;
			}

			Vector3D& GetVelocity( void )
			{
				return m_v3dVelocity;
			}

			float GetCharTime( void )
			{
				return m_fCharTime;
			}

			Vector3D& GetImpulse( void )
			{
				return m_v3dImpulse;
			}

			float GetFloatHeight( void )
			{
				return m_fFloatHeight;
			}

			virtual void Pack(Packer& data, unsigned long packFlags) const;
			void Unpack(Unpacker& data, unsigned long unpackFlags);

			virtual void Preprocess(void);
			virtual void Move(void);
			virtual void Travel(void);

			virtual void Kill( void );

			virtual void SetAnimation( EntityMotion motion );

			void PerformAbility( long lAbilityNumber );

			ControllerMessage *ConstructMessage(ControllerMessageType type) const;
			void ReceiveMessage(const ControllerMessage *message);

			// Get the time at which the entity can finally be deleted (so the death animation can play for a bit).
			unsigned long GetKillTime( void ) 
			{
				return (m_ulKillTime);
			}

			static void MotionComplete(Interpolator *interpolator, void *data);
	};
}


#endif
