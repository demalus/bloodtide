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

#ifndef StatsProtocol_h
#define StatsProtocol_h

#include "C4Link.h"
#include "C4Array.h"

namespace C4
{
	// Forward declaration
	class StatEffect;
	class BTEntityController;

	enum StatIDs
	{
		kStatMaxHealth				= 1 << 0,
		kStatHealth					= 1 << 1,
		kStatStrength				= 1 << 2,
		kStatArmor					= 1 << 3,
		kStatMoveSpeed				= 1 << 4,
		kStatAttackRange			= 1 << 5,
		kStatMaxHealthEffect		= 1 << 6,
		kStatStrengthEffect			= 1 << 7,
		kStatArmorEffect			= 1 << 8,
		kStatMoveSpeedEffect		= 1 << 9,
		kStatAttackRangeEffect		= 1 << 10
	};

	/**
	 * This interface will be used for any stats controllers.
	 * It guarantees access and mutations to all stats.
	 * It also provides a way to apply effects.
	 * @author - Frank Williams (demalus@gmail.com)
	 * @modified - Garret Doe (gdoe87@gmail.com)
	 * @modified - Chris Williams
	 */
	class StatsProtocol : public LinkTarget<StatsProtocol>
	{
		private:

			BTEntityController					*m_pOwner;

			unsigned long						m_ulDirtyStats;

			float m_fMaxHealth;					// Maximum Health
			float m_fHealth;					// Current Health
			float m_fStrength;					// Determines Damage Dealt
			float m_fArmor;						// Determines Damage Taken
			float m_fMovementSpeed;				// Speed moving from A to B
			float m_fAttackRange;				

			/* Effects granted by StatEffects */
			float m_fEffectOnMaxHealth;
			float m_fEffectOnStrength;
			float m_fEffectOnArmor;
			float m_fEffectOnMovementSpeed;
			float m_fEffectOnAttackRange;

			Array<StatEffect*> m_arrEffects;

		public:

			StatsProtocol(float fMaxHealth = 0.0F, float fHealth = 0.0F, float fStrength = 0.0F, float fArmor = 0.0F, float fMovementSpeed = 0.0F, float fAttackRange = 0.0F);	

			~StatsProtocol();

			void SetOwner( BTEntityController *pOwner )
			{
				m_pOwner = pOwner;
			}
					
			// Access base stats
			virtual float GetBaseMaxHealth( void ) { return m_fMaxHealth; }
			virtual float GetBaseStrength( void ) { return m_fStrength; }
			virtual float GetBaseArmor( void ) { return m_fArmor; }
			virtual float GetBaseMovementSpeed( void ) { return m_fMovementSpeed; }
			virtual float GetBaseAttackRange( void ) { return m_fAttackRange; }

			// Access effects
			virtual float GetEffectOnMaxHealth( void ) { return m_fEffectOnMaxHealth; }
			virtual float GetEffectOnStrength( void ) { return m_fEffectOnStrength; }
			virtual float GetEffectOnArmor( void ) { return m_fEffectOnArmor; }
			virtual float GetEffectOnMovementSpeed( void ) { return m_fEffectOnMovementSpeed; }
			virtual float GetEffectOnAttackRange( void ) { return m_fEffectOnAttackRange; }

			// Access total (base + effects)
			virtual float GetMaxHealth( void ) { return m_fMaxHealth + m_fEffectOnMaxHealth; }
			virtual float GetHealth( void ) { return m_fHealth; }
			virtual float GetStrength( void ) { return m_fStrength + m_fEffectOnStrength; }
			virtual float GetArmor( void ) { return m_fArmor + m_fEffectOnArmor; }
			virtual float GetMovementSpeed( void ) { return m_fMovementSpeed + m_fEffectOnMovementSpeed; }
			virtual float GetAttackRange( void ) { return m_fAttackRange + m_fEffectOnAttackRange; }

			// Mutate
			virtual void SetBaseMaxHealth( float nNewMaxHealth );
			virtual void SetHealth( float fNewHealth );
			virtual void AddHealth( float fAmount );
			virtual void RemoveHealth( float fAmount );
			virtual void SetBaseStrength( float fNewStrength );
			virtual void SetBaseArmor( float fNewArmor);
			virtual void SetBaseMovementSpeed( float fNewMovementSpeed );
			virtual void SetBaseAttackRange( float fNewAttackRange );

			// Mutate Effects
			virtual void SetEffectOnMaxHealth( float fNewEffect );
			virtual void SetEffectOnStrength( float fNewStrength );
			virtual void SetEffectOnArmor( float fNewArmor );
			virtual void SetEffectOnMovementSpeed( float fNewMovementSpeed );
			virtual void SetEffectOnAttackRange( float fNewAttackRange );

			// Effects
			virtual bool ApplyEffect( StatEffect* pEffect );
			virtual bool RemoveEffect( StatEffect* pEffect );
			virtual void UpdateEffects( unsigned long ulTime );

			// Client-side Synchronization
			virtual void Synchronize( long lControllerIndex, bool bFullSync ); // sends an update of the units dirty stats to clients
			virtual void UpdateStats( unsigned long ulDirtyStats, const Array<float>& arrStats );
			virtual void UpdateStat( long lStatID, float fValue );
	};
}

#endif