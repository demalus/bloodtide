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


#ifndef BTAbility_h
#define BTAbility_h

#define ModDmg_Str(str) str*0.01F	
#define ModDmg_Arm(armr) armr*0.01F	

#include "C4Link.h"
#include "StatEffect.h"
#include "StringTableManager.h"
#include "C4Messages.h"
#include "BTEffects.h"

namespace C4
{
	enum AbilityState
	{
		kAbilityStateNone,
		kAbilityStateCooldown,
		kAbilityStateGlobal
	};

	enum AbilityEvent
	{
		kAbilityEventTriggered, // used for syncing abilities on clients (to set cooldowns on ui elements)
		kAbilityEventServerTriggered,	// an ability was performed (performs additional logic on the server)
		kAbilityEventDamaged // damage was done to the owner of this ability ### passes in a struct entityDamaged
	};

	typedef struct entityDamaged
	{
		long m_lSource;
		long m_lDamage;

	} EntityDamaged;

	// passed to subclasses of Ability so that new abilities can parse the string table for additional information
	typedef struct stringTableInfo
	{
		TableID		m_idStringTable;
		Type		m_ulEntityID;
		Type		m_ulAbilityID;
		const char	*m_pSound;
		
		struct stringTableInfo( TableID stringTableID, Type entityID, Type abilityID, const char *pSound ) :
				m_idStringTable( stringTableID ),
				m_ulEntityID( entityID ),
				m_ulAbilityID( abilityID ),
				m_pSound( pSound )
		{

		}

	} StringTableInfo;


	// forward declerations
	class BTEntityController;
	class StatsProtocol;

	/** 
	 * This class represents an ability that entities can
	 * perform.  Abilities range from melee attacks to 
	 * area of effect heals.
	 * @author - Frank Williams (demalus@gmail.com)
	 * @author - Chris Williams
	 */
	class Ability
	{
		protected:

			BTEntityController			*m_pOwner;

			StringTableInfo				m_tableInfo;

			unsigned long				m_ulDelay; // Time to wait before firing the ability (e.g. wait for an animation).
			unsigned long				m_ulCooldown; // How long to wait before the ability can be performed again
			unsigned long				m_ulLastUsed; // The time that this ability was last used

			AbilityState				m_AbilityState;

			Ability( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown );

		public:

			virtual ~Ability();

			BTEntityController *GetOwner( void ) const
			{
				return (m_pOwner);
			}

			unsigned long GetLastUsed( void ) const
			{
				return (m_ulLastUsed);
			}

			unsigned long GetCooldown( void ) const
			{
				return (m_ulCooldown);
			}

			AbilityState GetState( void ) const
			{
				return (m_AbilityState);
			}

			const char *GetSound( void ) const
			{
				return (m_tableInfo.m_pSound);
			}

			// Returns false if the ability should not be executed (global cooldown hasn't elapsed)
			virtual bool Execute( void );

			virtual void Update( unsigned long ulTime );
			virtual void HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData );

			/*
			 * Returns an ability of the specific type given or nullptr on error or if the ability type does not exist.
			 *
			 * stringTableID must be either kStringTableUnits or kStringTableCommanders
			 * ulEntityID is the Commander or Unit ID within the string table (e.g. 'SCUB')
			 * ulAbilityID is the ability within that Commander or Unit (e.g. 'abi1')
			 */
			static Ability *Construct( BTEntityController *pOwner, TableID stringTableID, Type ulEntityID, Type ulAbilityID );
	};

	/** 
	 * This class represents a melee attack that entities can perform.
	 * @author - Frank Williams (demalus@gmail.com)
	 * @author - Chris Williams
	 */
	class AbilityMelee : public Ability
	{
		friend class Ability;

		private:

			float						m_fDamage;


			AbilityMelee( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, float fDamage );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityMelee();

			bool Execute( void );
	};

	struct BTCollisionData;

	/** 
	 * The ability projectile class's responsibility is to parse the string table for the projectile associated with this ability
	 * and spawn that projectile on the server and all clients. In order for the projectile to have any effect when it collides with
	 * an enemy, the CreateStatEffect() function must be modified to create the specific stat effect for the given projectile.
	 * @author - Chris Williams
	 */
	class AbilityProjectile : public Ability
	{
		friend class Ability;

		private:

			unsigned long				m_ulProjectileID; // 4-digit identifier in the Projectiles string table


			AbilityProjectile( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulProjectileID );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityProjectile();

			bool Execute( void );

			StatEffect *CreateStatEffect( void );		
			static void ImpactEventCallback( BTCollisionData *data, void *selfData ); // Do something on impact
			void CreateInitialEffect( BTEntityController& entity ); // Effect applied on initial firing of projectile
	};

	/** 
	 * Functions similar to ability projectile, but spawns multiple projectiles.
	 * @author - Chris Williams
	 */
	class AbilityMultiShot : public Ability
	{
		friend class Ability;

		private:

			unsigned long				m_ulProjectileID; // 4-digit identifier in the Projectiles string table

			long						m_lNumProjectiles;
			float						m_fAngle; // angle between projectiles	

			AbilityMultiShot( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulProjectileID, long lNumProjectiles, float fAngle );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityMultiShot();

			bool Execute( void );

			StatEffect *CreateStatEffect( void );			
			static void ImpactEventCallback( BTCollisionData *collision, void *selfData );
	};

	/** 
	 * This ability will hide the entity that executes the ability from the enemy team.
	 * @author - Chris Williams
	 */
	class AbilityStealth : public Ability
	{
		friend class Ability;

		private:

			unsigned long			m_ulDuration;

			AbilityStealth( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulDuration );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityStealth();

			void Update( unsigned long ulTime );
			void HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData );

			bool Execute( void );			
	};

	/** 
	 * The ability AOE class's responsibility is to parse the string table for the effect associated with this ability
	 * and spawn that effect on the server and all clients (for all units effected). In order for the AOE to have 
	 * any effect when it triggered, the CreateStatEffect() function must be modified to create the specific stat effect for the given AOE.
	 * @author - Chris Williams
	 */
	class AbilityAOE : public Ability
	{
		friend class Ability;

		private:

			float						m_fRange;

			bool						m_bEffectSelf;
			bool						m_bApplyToEnemies;


			AbilityAOE( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, float fRange );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityAOE();

			bool Execute( void );

			StatEffect *CreateStatEffect( void );	
			void CreateEffect( BTEntityController& entity );
	};

	/** 
	 * AbilityRiptide will make the caster invisible for a duration and will apply a movement speed buff to
	 * units/commanders that are close by at the initial cast of the ability.
	 * @author - Chris Williams
	 */
	class AbilityRiptide : public Ability
	{
		friend class Ability;

		private:

			float						m_fRange;
			unsigned long				m_ulDuration;

			StatEffect					*m_pStatEffect;

			AbilityRiptide( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, float fRange, unsigned long ulDuration );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityRiptide();

			void Update( unsigned long ulTime );
			void HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData );

			bool Execute( void );	
	};

	class AbilityLaser : public Ability
	{
		friend class Ability;

		private:

			AbilityLaser( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityLaser();

			bool Execute( void );

			StatEffect *CreateStatEffect( void );	
			void CreateEffect( Point3D& p3dStart, Point3D& p3dEnd );
	};

	class AbilityBarrier : public Ability
	{
		friend class Ability;

		private:

			unsigned long					m_ulDuration;

			bool							m_bActive;
			long							m_lEffectControllerIndex;


			AbilityBarrier( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulDuration );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityBarrier();

			void Update( unsigned long ulTime );
			bool Execute( void );
	};

	/*
	* Drops a depth charge projectiles that explodes after a given duration and stuns enemy units in a given radius.
	* @author Chris Williams
	*/
	class AbilityDepthCharge : public Ability
	{
		friend class Ability;

		private:

			unsigned long				m_ulProjectileID; // 4-digit identifier in the Projectiles string table

			float						m_fDamage;
			float						m_fRange;
			unsigned long				m_ulDuration;


			AbilityDepthCharge( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulProjectileID, float fDamage, float fRange, unsigned long ulDuration );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityDepthCharge();

			bool Execute( void );	
			static void ImpactEventCallback( BTCollisionData *collision, void *selfData );
	};

	class AbilityBladeCounter : public Ability
	{
		friend class Ability;

		private:

			unsigned long				m_ulDuration;
			float						m_fPercentReturned;

			long						m_lEffectControllerIndex;
			bool						m_bActive;

			AbilityBladeCounter( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulDuration, float fPercentReturned );

			static Ability *Construct( BTEntityController *pOwner, StringTableInfo tableInfo );

		public:

			~AbilityBladeCounter();

			void Update( unsigned long ulTime );
			void HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData );

			bool Execute( void );
	};
}

#endif
