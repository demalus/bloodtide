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


#ifndef BTControllers_h
#define BTControllers_h

#include "C4Controller.h"
#include "C4Properties.h"
#include "C4Time.h"
#include "C4Models.h"
#include "StatsProtocol.h"
#include "BTCollider.h"
#include "BTSteering.h"
#include "MGMultiplayer.h"
#include "StringTableManager.h"
#include "BTGoalThink.h"
#include "StatEffect.h"

#define CLOSE_TO_TARGET		0.05F
#define NUM_BUILDINGS		4

#define GLOBAL_COOLDOWN		1500

namespace C4
{

	// Controller types
	enum
	{
		kControllerMapBounds		= 'mbnd',
		kControllerEntity			= 'enti', // base controller type for entities
		kControllerLesserEntity		= 'less', // an entity that isn't part of the game rules (fish, projectiles, etc.)
		kControllerBase				= 'base',
		kControllerPlot				= 'plot',
		kControllerBuilding			= 'bldg',
		kControllerCommander		= 'cmmd',
		kControllerUnit				= 'unit',
		kControllerBTProjectile		= 'proj',
		kControllerSimpleFish		= 'fish',
		kControllerVictory			= 'vict'
	};

	// Locator types
	enum
	{
		kLocatorCenter				= 'cent',
		kLocatorAxis				= 'axis',
		kLocatorMass				= 'mass',
		kLocatorFire				= 'fire'
	};

	// Entity types
	typedef Type EntityType;
	enum {
		kEntityNone			= 0,
		kEntityUnit,
		kEntityCommander,
		kEntityBase,
		kEntityPlot,
		kEntityBuilding,
		kEntityProjectile
	};

	enum EntityMotion
	{
		kMotionNone,
		kMotionMove,
		kMotionDeath,
		kMotionWaiting,
		kMotionAbility1 = 'abi1',
		kMotionAbility2 = 'abi2',
		kMotionAbility3 = 'abi3'
	};

	enum EntityFlags
	{
		kEntityInvisible				= 1 << 0, // so goals for enemy units will not choose an invisible unit as a target
		kEntityDead						= 1 << 1, // whether or not this entity is dead
		kEntityStunned					= 1 << 2, // entity cannot move or perform actions while stunned
		kEntityInvulnerable				= 1 << 3, // entity does not take damage
		kEntitySelected					= 1 << 4  // draw the selection ring underneath the entity
	};

	// forward declerations
	class SelectionRing;
	class MiniHealthBar;

	/**
	 * This class represents a generic entity that can be in the game world.
	 * An entity has an entity ID, a collider, and a stats controller.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTEntityController : public Controller, public ListElement<BTEntityController>, public LinkTarget<BTEntityController>
	{
		private:

			void CreateSelectionRing( void );
			void UpdateSelectionRing( void );

			void CreateMiniHealthBar( void );
			void UpdateMiniHealthBar( void );

		protected:

			// Entity Type
			EntityType			m_lEntityType;

			// Entity team
			long				m_lTeam;

			// Stats
			StatsProtocol		*m_pStats;

			// Positioning
			BTCollider			*m_pCollider;

			float				m_fOriMult;			// Orientation multiplier (x times pi)
			float				m_fAzimuth;
			float				m_fTargetAltitude;	// Altitude used for abilities.

			// Animation
			EntityMotion		m_EntityMotion;

			// Gameplay
			unsigned long		m_ulEntityFlags;

			// String table
			TableID				m_lTableID;			// String Table ID
			unsigned long		m_ulStringID;		// String ID in the table

			// UI
			SelectionRing		*m_pSelectionRing;
			MiniHealthBar		*m_pMiniHealthBar;

			void Initialize( void );

		public:

			enum {
				kEntityMessageDeath,
				kEntityMessageUpdatePosition,
				kEntityMessageUpdateAzimuth,
				kEntityMessageAbility,
				kEntityMessagePlayAnimation,
				kEntityMessageUpdateStat,
				kEntityMessageStealth,
				kEntityMessageUnstealth,
				kEntityMessageStun,
				kEntityMessageUnstun,
				kEntityMessageBaseCount
			};

			BTEntityController( ControllerType contType );
			BTEntityController( ControllerType contType, EntityType lEntityType );
			BTEntityController( ControllerType contType, EntityType lEntityType, StatsProtocol *pStats, long lTeam, unsigned long ulStringID, TableID lTableID, float fOriMult );
			virtual ~BTEntityController();

			using ListElement<BTEntityController>::Previous;
			using ListElement<BTEntityController>::Next;


			EntityType GetEntityType( void )
			{
				return m_lEntityType;
			}

			StatsProtocol *GetStats( void )
			{
				return m_pStats;
			}

			void SetStats( StatsProtocol *pStats )
			{
				if( m_pStats == pStats ) {
					return;
				}

				if( m_pStats ) {
					delete m_pStats;
				}

				m_pStats = pStats;
			}

			BTCollider *GetCollider( void )
			{
				return m_pCollider;
			}

			void SetCollider( BTCollider *pCollider )
			{
				if( m_pCollider == pCollider ) {
					return;
				}

				if( m_pCollider ) {
					delete m_pCollider;
				}

				m_pCollider = pCollider;
			}

			Point3D GetPosition( void )
			{
				if( m_pCollider ) {
					return m_pCollider->GetPosition();
				}

				return Point3D(0,0,0);
			}

			float GetAzimuth( void ) const
			{
				return (m_fAzimuth);
			}

			void SetAzimuth( float azimuth )
			{
				m_fAzimuth = azimuth;
			}

			long GetTeam( void ) const
			{
				return m_lTeam;
			}

			void SetTeam( long lTeam )
			{
				m_lTeam = lTeam;
			}

			virtual void SetAnimation( EntityMotion motion )
			{
				m_EntityMotion = motion;
			}

			EntityMotion GetAnimation( void ) const
			{
				return (m_EntityMotion);
			}

			virtual float GetTargetAltitude( void ) const
			{
				return (m_fTargetAltitude);
			}

			void SetTargetAltitude( float fAlt )
			{
				m_fTargetAltitude = fAlt;
			}

			unsigned long GetEntityFlags( void )
			{
				return (m_ulEntityFlags);
			}

			void SetEntityFlags( unsigned long flags )
			{
				m_ulEntityFlags = flags;
			}

			unsigned long GetStringID( void )
			{
				return (m_ulStringID);
			}

			void SetStringID( unsigned long ulEntityID )
			{
				m_ulStringID = ulEntityID;
			}

			TableID GetTableID( void )
			{
				return m_lTableID;
			}

			void SetSelected( bool selected );

			virtual void Pack(Packer& data, unsigned long packFlags) const;
			void Unpack(Unpacker& data, unsigned long unpackFlags);

			virtual void Preprocess(void);

			virtual void Move( void );
			virtual void Travel( void );
			virtual void PostTravel( void );

			// Calculate the final damage and apply it.
			// fDamage - Initial damage (before defense considered).
			// lEnemyIndex - Controller index of the attacker.
			virtual void Damage( float fDamage, long lEnemyIndex );

			// Destroy this entity and free up some memory.
			virtual void Kill( void );
			void Destroy( void );

			// Use the selected ability.
			virtual void PerformAbility( long lAbilityNumber );

			virtual ControllerMessage *ConstructMessage(ControllerMessageType type) const;
			virtual void ReceiveMessage(const ControllerMessage *message);

			virtual void SetPerspectiveExclusionMask(unsigned long mask) const ;

			bool CloseToPosition( Point3D& p3dPosition );
			bool CloseToPosition2D( Point3D& p3dPosition );

			bool CanAttack( BTEntityController *pEnemy );
	};


	/**
	 * This sets the rectangular size of the playable area on a map.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class MapBoundsController : public Controller
	{
		private:

			Point2D		m_topRight;

			// Replication for world editor.
			MapBoundsController( const MapBoundsController& pController );
			Controller *Replicate( void ) const;

		public:

			MapBoundsController();
			~MapBoundsController();

			// Which nodes this controller can be placed on.
			static bool ValidNode( const Node *node );

			// Writing/reading to file (a world file).
			void Pack( Packer& data, unsigned long packFlags ) const;
			void Unpack( Unpacker& data, unsigned long unpackFlags );

			// Settings
			long GetSettingCount( void ) const;
			Setting *GetSetting( long index ) const;
			void SetSetting( const Setting *setting );

			Point2D GetTopRightVector( void ) const { return m_topRight; };
	};

	/**
	* BaseController
	* This controller should be attached to Base objects, giving
	* them the functionality to spawn units, connect to plots
	* @author Garret Doe
	* @modified - Frank Williams (demalus@gmail.com)
	*/
	class BaseController : public BTEntityController
	{
		private:

			Type			m_UnitType;
			Type			m_CommanderType;

			long			m_lSpawnTimer;
			long			m_lSpawnNumber;

			bool			m_bSpawn;
			unsigned long	m_ulNextSpawn;

			bool			m_ReadyForBuilding;

			float			m_fSpawnTimerEffect;	// adjusts the spawn timer

			// Base activation/deactivation - this controls whether or not bases will spawn things
			// and whether or not plots for this base can be used.
			bool			m_bIsActive;

			// Store some stats while in world editor.
			// Note: These are not to be used after Preprocess() is called!
			float			m_fTempHeight;
			float			m_fTempRadius;
			float			m_fTempMaxHP;
			float			m_fTempHP;

			// Upgrades from buildings.
			// Note: the values stored here are percentage upgrades.
			StatsProtocol	m_objUpgrades;

			// Replication for world editor.
			BaseController( const BaseController& bController );
			Controller *Replicate( void ) const;

			// Used for unpacking specific (arbitrary) chunks of this controller's data.
			bool UnpackChunk( const ChunkHeader *chunkHeader, Unpacker& data, unsigned long unpackFlags );

		public:

			enum {

				kBaseMessageConstructionCompleted = kEntityMessageBaseCount, // Another base can be built on this team
				kBaseMessageBuildingCanceled,
				kBaseMessageBaseCount
			};


			BaseController();
			~BaseController();

			// Which nodes this controller can be placed on.
			static bool ValidNode( const Node *node );

			// Serialization.
			void Pack( Packer& data, unsigned long packFlags ) const;
			void Unpack( Unpacker& data, unsigned long unpackFlags );

			// Settings
			long GetSettingCount( void ) const;
			Setting *GetSetting( long index ) const;
			void SetSetting( const Setting *setting );

			Type GetUnitType( void ) const
			{
				return m_UnitType;
			}

			Type GetCommanderType( void ) const
			{
				return m_CommanderType;
			}

			long GetSpawnTimer( void ) const
			{
				return m_lSpawnTimer;
			}

			long GetSpawnNumber( void ) const
			{
				return m_lSpawnNumber;
			}

			bool CanSpawnUnits( void ) const
			{
				return m_bSpawn;
			}

			unsigned long GetNextSpawnTime( void ) const
			{
				return m_ulNextSpawn;
			}

			bool IsReadyForBuilding( void ) const
			{
				return m_ReadyForBuilding;
			}

			void SetReadyForBuilding(bool ready)
			{
				m_ReadyForBuilding = ready;
			}

			float GetSpawnTimerEffect()
			{
				return m_fSpawnTimerEffect;
			}

			void SetSpawnTimerEffect(float percent)
			{
				m_fSpawnTimerEffect = percent;
			}

			// Spawning
			bool SpawnUnits( void );
			void StartSpawning( void );
			void StopSpawning( void );

			bool SpawnCommander( PlayerKey nKey, long prevKills  );

			virtual void Preprocess( void );
			virtual void Move( void );

			void Damage( float fDamage, long lEnemyIndex );

			// Base activation/deactivation - this controls whether or not bases will spawn things
			// and whether or not plots for this base can be used.
			void ActivateBase( void )
			{
				m_bIsActive = true;
			}

			void DeactivateBase( void )
			{
				m_bIsActive = false;
			}

			bool IsActiveBase( void )
			{
				return (m_bIsActive);
			}

			ControllerMessage *ConstructMessage(ControllerMessageType type) const;
			void ReceiveMessage(const ControllerMessage *message);

			// Adds the value to the given stat.
			void UpdateUpgrade( long lStatID, float fValue );

			StatsProtocol& GetUpgrades( void )
			{
				return (m_objUpgrades);
			}
	};

	/**
	 * This class represents a generic building the player can use
	 * to customize a base.  Buildings can apply various effects to units or
	 * even have some AI.
	 * @author - Garret Doe
	 * @modified - Frank Williams (demalus@gmail.com)
	 */
	class BuildingController : public BTEntityController
	{
		private:

			long			m_lPlot;		// controller index of plot owner

			BTGoalThink		m_objBaseGoal;	// basic AI

			bool			m_bUnderConstruction;	// building is being constructed
			long			m_lConstructionTime;	// time it will take to build the building
			unsigned long	m_ulStartTime;			// timestamp building started construction
			unsigned long	m_ulLastUpdate;			// timestamp on last building "build second"

			unsigned long	m_ulStringID;			// This building's id in the buildings string table.

			Link<Node>		m_pEmitter;			// Particle effects for construction and explosions
			Link<Node>		m_pEffect;			// Controls the particles for the emitter

			void Initialize( const Point3D& position, float fRadius, float fHeight );

		public:

			BuildingController();
			BuildingController(const Point3D& position, unsigned long ulStringID, StatsProtocol *pStats, float fRadius, float fHeight,
				long lTeam, long lPlot, long lTime);
			~BuildingController();

			void Pack(Packer& data, unsigned long packFlags) const;
			void Unpack(Unpacker& data, unsigned long unpackFlags);

			void Preprocess(void);
			void Move(void);

			void Damage( float fDamage, long lEnemyIndex );

			virtual void SetAnimation( EntityMotion motion );

			ControllerMessage *ConstructMessage(ControllerMessageType type) const;
			void ReceiveMessage(const ControllerMessage *message);

			void SetParticleEffects( void );
			void ToggleConstructionEffect (bool on);

			void ApplyBuildingEffect( void );
			void RemoveBuildingEffect( void );

			void AttackEffect( BTCollisionData data );
	};


	/**
	* PlotController
	* This controller should be attached to a marker node, and connected to a Base
	* object, signifying the location spots for buildings.
	*/
	class PlotController : public BTEntityController
	{
	private:


		Type m_BuildingType;					//type of building built upon this plot

		unsigned long m_ulbuildingTime;			// length of time it takes to construct building

		Type m_arrPossibleBuildings[NUM_BUILDINGS];		// list of possible buildings that can be built

		Link<BTEntityController> m_myBuilding;	// pointer to building on plot

		Link<BTEntityController> m_myBase;		// base that this plot is attached to

		// Replication for world editor.
		PlotController( const PlotController& plotController );
		Controller *Replicate( void ) const;

	public:

		PlotController();
		~PlotController();

		// Which nodes this controller can be placed on.
		static bool ValidNode( const Node *node );

		// Serialization.
		void Pack( Packer& data, unsigned long packFlags ) const;
		void Unpack( Unpacker& data, unsigned long unpackFlags );

		// Settings
		long GetSettingCount( void ) const;
		Setting *GetSetting( long index ) const;
		void SetSetting( const Setting *setting );

		void Preprocess();
		void Move();

		void SpawnBuilding(Type buildingType);
		void DespawnBuilding();

		Type GetBuildingType( void ) const
		{
			return m_BuildingType;
		}

		BTEntityController* GetBase (void) const
		{
			return m_myBase;
		}

		BTEntityController* GetBuilding (void) const
		{
			return m_myBuilding;
		}

		void GetPossibleBuildings( Array<Type>& arrPossible )
		{
			for( int i = 0; i < NUM_BUILDINGS; i++ ) {
				arrPossible.AddElement( m_arrPossibleBuildings[i] );
			}
		}

		void SetBase (BTEntityController *base)
		{
			m_myBase = base;
		}

		void SetBuilding (BTEntityController *building)
		{
			m_myBuilding = building;
		}

	};

	//# \enum	ProjectileFlags

	enum
	{
		kProjectileStopped		= 1 << 0,		//## The projectile should not move.
		kProjectileGravity		= 1 << 1,		//## Gravity is applied to the projectile.
		kProjectileResisted		= 1 << 2,		//## Viscous resistance is applied to the projectile.
		kProjectileNoImpact		= 1 << 3		//## Projectile does not explode on impact, only after a duration expires (see note below!)
	};

	/* Note on the use of kProjectileNoImpact:

		All StatEffects have a notion of an 'owner'. This is set to the unit that the projectile collided with when not using this flag.
		Projectiles that do not impact have no entity to set as the owner of the stat effect.
		Instead, the projectile sets itself as the owner and manually calls StatEffect->OnApplyEffect() and StatEffect->OnRemoveEffect().
		Note that StatEffect->UpdateEffect() is not called.s
		This method will allow the stat effect to do processing at the position of the projectile, but it will need to query the world itself for target(s).
	*/

	class BTProjectileController : public BTEntityController
	{
		public:

			typedef void ProjectileImpactCallback( BTCollisionData *collision, void *selfData );

		private:

			unsigned long					projectileFlags;

			Point3D							m_p3dInitialPosition;
			float							m_fSpeed;
			float							m_fDistance;

			// Duration and start time only used if the kProjectileNoImpact flag is set
			unsigned long					m_ulDuration;
			unsigned long					m_ulStartTime;
			//

			BTSteering						m_objSteering;

			StatEffect						*m_pStatEffect;
			ProjectileImpactCallback		*m_pCallback;
			void							*m_pCallbackData;

			// Sound when projectile hits something.
			const char						*m_pDeathSound;
			bool							m_bDeathSoundOnlyOnImpact;

			bool UnpackChunk(const ChunkHeader *chunkHeader, Unpacker& data, unsigned long unpackFlags);

		public:

			BTProjectileController(ControllerType type, float radius, float height, unsigned long flags, const Point3D& position, float azimuth, float speed, float fDistance, long lTeam, float fAlt, float fOriMult, bool bSoundOnlyOnImpact, const char *pHitSound = nullptr, unsigned long ulDuration = 0 );
			virtual ~BTProjectileController();

			unsigned long GetProjectileFlags(void) const
			{
				return (projectileFlags);
			}

			void SetProjectileFlags(unsigned long flags)
			{
				projectileFlags = flags;
			}

			float GetAzimuth(void) const
			{
				return (m_fAzimuth);
			}

			void SetAzimuth(float fAzimuth)
			{
				m_fAzimuth = fAzimuth;
			}

			void SetDuration(unsigned long ulDuration)
			{
				m_ulDuration = ulDuration;
			}

			// the stat effect applied to the enemy that this projectile collided with
			void SetStatEffect( StatEffect *pStatEffect );

			void SetImpactCallback( ProjectileImpactCallback pCallback, void *selfData );

			virtual void Preprocess(void);
			virtual void Move(void);
			virtual void Travel(void);

			virtual void Kill( void );
	};
}

#endif
