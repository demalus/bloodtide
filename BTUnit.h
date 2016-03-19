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


#ifndef BTUnit_h
#define BTUnit_h


#include "BTCharacter.h"
#include "MGMultiplayer.h"
#include "BTSteering.h"
#include "BTGoalThink.h"
#include "BTInfoPane.h"

#define UNIT_SAFE_DIST_MULT 1.3F


namespace C4
{

	/** 
	 * This class represents a generic unit. A unit is a warrior 
	 * that spawns for a player at various intervals. This unit 
	 * class can be used to represent all sorts of unit types.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class UnitController : public BTCharacterController, public SnapshotSender, public LinkTarget<UnitController>
	{
		private:

			BTSteering			m_objSteering;

			BTGoalThink			m_objBaseGoal;

			// Keeps track of old information so the snapshot sender can see if it needs to send an update.
			float				m_fOldAzimuth;
			Point3D				m_p3dOldPosition;
			Vector3D			m_v3dOldVelocity;

			Ability				*m_pAbility1;

			// Time last snapshot arrived - used to see if interp should be off.
			unsigned long		m_ulLastUpdate;

			void Initialize( const Point3D& position, float fRadius, float fHeight );

		public:

			UnitController();
			UnitController(const Point3D& position, unsigned long ulStringID, TableID lTableID, StatsProtocol *pStats, float fRadius, float fHeight, float fFloatHeight, long lTeam, float fOriMult);
			~UnitController();

			enum
			{
				kUnitMessageBeginMovement = kCharacterMessageBaseCount,
				kUnitMessageEndMovement,
				kUnitMessageChangeMovement
			};

			BTSteering *GetSteering( void )
			{
				return (&m_objSteering);
			}

			void Pack(Packer& data, unsigned long packFlags) const;
			void Unpack(Unpacker& data, unsigned long unpackFlags);

			void Preprocess(void);
			void Move(void);
			void Travel(void);

			//bool Damage(Fixed damage, const Point3D *center, PlayerKey shooterKey);
			void Kill( void );
			void PerformAbility( long lAbilityNumber );

			virtual void SetAnimation( EntityMotion motion );

			void AddGoal( BTGoal *pGoal );
			void ClearGoals( void );

			//void PerformAbility( AbilityInfo *pInfo );

			ControllerMessage *ConstructMessage(ControllerMessageType type) const;
			void ReceiveMessage(const ControllerMessage *message);
			void SendSnapshot(void);

			void BeginMovement( float fAzm );
			void EndMovement( float fAzm );
			void ChangeMovement( float fAzm );

			void InitializeUnitInfoPane( UnitInfoPane& pane );
	};


	/** 
	 * Message for spawning units.
	 */
	class CreateUnitMessage : public CreateModelMessage
	{
		friend class CreateModelMessage;

		private:

			long				m_lTeam;

			float				m_fInitialAzimuth;
			float				m_fInitialAltitude;

			Vector3D			m_v3dGroundNormal;
			Vector3D			m_v3dSlideNormal[2];

			unsigned long		m_lCharacterFlags;
			unsigned long		m_lMovementFlags;

			unsigned long		m_ulEntityID;

			unsigned long		m_ulBaseIndex;

			CreateUnitMessage();

		public:

			CreateUnitMessage(ModelMessageType type, long index, unsigned long ulEntityID, const Point3D& position, float azm, float alt, const Vector3D& ground, const Vector3D& slide1, const Vector3D& slide2, unsigned long character, unsigned long movement, long lTeam, unsigned long ulBaseIndex);
			~CreateUnitMessage();

			float GetInitialAzimuth(void) const
			{
				return (m_fInitialAzimuth);
			}

			float GetInitialAltitude(void) const
			{
				return (m_fInitialAltitude);
			}

			const Vector3D& GetGroundNormal(void) const
			{
				return (m_v3dGroundNormal);
			}

			const Vector3D& GetSlideNormal(long index) const
			{
				return (m_v3dSlideNormal[index]);
			}

			unsigned long GetCharacterFlags(void) const
			{
				return (m_lCharacterFlags);
			}

			unsigned long GetMovementFlags(void) const
			{
				return (m_lMovementFlags);
			}

			unsigned long GetEntityID(void) const
			{
				return (m_ulEntityID);
			}

			long GetTeam( void ) const
			{
				return (m_lTeam);
			}

			unsigned long GetBaseIndex( void ) const
			{
				return (m_ulBaseIndex);
			}

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	class UnitMovementMessage : public ControllerMessage
	{
		friend class UnitController;

		private:

			Point3D			m_p3dPosition;
			float			m_fAzimuth;

			UnitMovementMessage(ControllerMessageType type, long controllerIndex);

		public:

			UnitMovementMessage(ControllerMessageType type, long controllerIndex, const Point3D& position, float azimuth);
			~UnitMovementMessage();

			const Point3D& GetPosition(void) const
			{
				return (m_p3dPosition);
			}

			float GetMovementAzimuth(void) const
			{
				return (m_fAzimuth);
			}

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	enum UnitCommandType {
		kCommandNone	= 0,
		kCommandMove,
		kCommandAttack,
		kCommandAttackTo,
		kCommandStop,
		kCommandFollow
	};

	typedef struct UnitCommand
	{
		long		m_lUnit;		// Controller index of unit carrying out this command.
		Point3D		m_p3dTarget;	// Move-to position
		long		m_lTarget;		// Controller index of target unit

	} UnitCommand;

	/*
	* The UnitCommand message sends a player's command to a set
	* of units to the server for processing.
	* @author - Frank Williams (demalus@gmail.com)
	* @author - Chris Williams
	*/
	class UnitCommandMessage : public Message
	{
		friend class Game;

		private:
			
			long				m_lCommandType;
			Array<UnitCommand>	m_arrCommands;
			
			UnitCommandMessage();
		
		public:
			
			UnitCommandMessage( long lCommandType, Array<UnitCommand> &arrCommands );
			~UnitCommandMessage();
			
			void Compress( Compressor& data ) const;
			bool Decompress( Decompressor& data );

			bool HandleMessage( Player *sender ) const;
	};
}


#endif
