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

#ifndef BTStatEffects_h_
#define BTStatEffects_h_

#include "StatEffect.h"
#include "C4Vector3D.h"
#include "C4Resources.h"
#include "C4Array.h"

namespace C4
{

	// forward decelerations
	class BTEntityController;
	class Geometry;
	class MaterialObject;

	enum
	{
		kModelStealthShader = 'stsh'
	};

	/** 
	 * The ProjectileBasicEffect class represents a basic projectile stat effect that only does raw damage.
	 * @author - Chris Williams
	 */
	class ProjectileBasicEffect : public StatEffect
	{
		private:

			float				m_fDamage;

		public:

			ProjectileBasicEffect( BTEntityController *pOwner, long lSource, float fDamage );
			~ProjectileBasicEffect();

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};

	/** 
	 * The ProjectileDOTEffect class represents a projectile stat effect that does damage every second for a duration
	 * @author - Chris Williams
	 */
	class ProjectileDOTEffect : public StatEffect
	{
		private:

			unsigned long				m_ulDamage;
			unsigned long				m_ulDuration;

			unsigned long				m_ulBegin;
			unsigned long				m_ulDamageAlreadyDone;

		public:

			ProjectileDOTEffect( BTEntityController *pOwner, long lSource, unsigned long ulDamage, unsigned long ulDuration );

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};

	/** 
	 * Fades an entity into a given transparency.
	 * @author - Chris Williams
	 */
	class StealthEffect : public StatEffect
	{
		// For each geometry, its geometry object will be changed to point
		// at the stealth shader. This structure will hold its old geometry object.
		typedef struct geometryTuple
		{
			Geometry					*m_pGeometry;
			MaterialObject *const		*m_pOldMaterial;

			struct geometryTuple( Geometry *a, MaterialObject *const *b ) : 
					m_pGeometry(a), m_pOldMaterial(b)
			{
			}

		} GeometryTuple;

		private:

			bool						m_bFullStealth; // the enemy team cannot see this entity

			Array<GeometryTuple *>		m_arrRevertGeometry;

		public:

			StealthEffect( BTEntityController *pOwner, long lSource, bool bFullStealth );
			~StealthEffect();

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			static bool ApplyTransparency( Geometry *pNode, const Point3D& p3dPos, float fRadius, void *data );

			StatEffect *Clone( void );
	};

	/** 
	 * The AOEHealEffect class represents a basic AOE stat effect that applies only an instant heal.
	 * @author - Chris Williams
	 */
	class AOEHealEffect : public StatEffect
	{
		private:

			float				m_fHealAmount;

		public:

			AOEHealEffect( BTEntityController *pOwner, long lSource, float fHealAmount );

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};

	/** 
	 * The BadNameEffect class represents an AOE stat effect that applies an instant heal
	 * and a temporary strength buff.
	 * @author - Chris Williams
	 */
	class BadNameEffect : public StatEffect
	{
		private:

			float				m_fHealAmount;
			float				m_fStrengthBoost;
			unsigned long		m_ulDuration;

			unsigned long		m_ulStartTime;

		public:

			BadNameEffect( BTEntityController *pOwner, long lSource, float fHealAmount, float fStrengthBoost, unsigned long ulDuration );

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};

	/** 
	 * Modifies movement speed for a duration.
	 * @author - Chris Williams
	 */
	class MoveSpeedEffect : public StatEffect
	{
		private:

			float				m_fEffectOnMove;
			unsigned long		m_ulDuration;

			unsigned long		m_ulStartTime;

		public:

			MoveSpeedEffect( BTEntityController *pOwner, long lSource, float fEffectOnMove, unsigned long ulDuration );

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};

	class SpiralHelix;

	/** 
	*  Laser Trident stuns an entity for a duration and applies instant damage.
	 * @author - Chris Williams
	 */
	class LaserTridentEffect : public StatEffect
	{
		private:

			float				m_fDamage;
			unsigned long		m_ulDuration;

			unsigned long		m_ulStartTime;

			long				m_lEffectController;
			
		public:

			LaserTridentEffect( BTEntityController *pOwner, long lSource, float fDamage, unsigned long ulDuration );
			~LaserTridentEffect();

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};

	class InvulnerabilityEffect : public StatEffect
	{
		private:

			unsigned long		m_ulDuration;

			unsigned long		m_ulStartTime;

		public:

			InvulnerabilityEffect( BTEntityController *pOwner, long lSource, unsigned long ulDuration );
			~InvulnerabilityEffect();

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};

	class DepthChargeEffect : public StatEffect
	{
		private:

			float				m_fDamage;
			float				m_fRange;
			unsigned long		m_ulDuration;

		public:

			DepthChargeEffect( BTEntityController *pOwner, long lSource, float fDamage, float fRange, unsigned long ulDuration );

			void OnApplyEffect( void );
			void OnRemoveEffect( void );
			void UpdateEffect( unsigned long ulTime );

			StatEffect *Clone( void );
	};
};


#endif

