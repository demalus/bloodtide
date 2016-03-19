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
// This file was based upon MGFighter.h
//=============================================================


#ifndef BTCommander_h
#define BTCommander_h


#include "C4World.h"
#include "C4ExtraEffects.h"
#include "BTCharacter.h"
#include "MGMultiplayer.h"
#include "BTInfoPane.h"

namespace C4
{
	enum
	{
		kColliderCorpse		= kColliderBaseClass << 0
	};
	
	
	enum CommanderFlag
	{
		kCommanderDead					= 1 << 0,
		kCommanderFiringPrimary			= 1 << 1,
		kCommanderFiringSecondary		= 1 << 2,
		kCommanderTeleported			= 1 << 3,
		
		kCommanderFiring				= kCommanderFiringPrimary | kCommanderFiringSecondary
	};

	// Model type for the commander character
	enum
	{
		kModelCommander			= 'cmmd'
	};
	
	
	class CommanderController;
	
	class CommanderInteractor : public Interactor
	{
		private:
			
			CommanderController	*m_pCommanderController;
		
		public:
			
			CommanderInteractor(CommanderController *pController);
			~CommanderInteractor();
			
			void HandleInteractionEvent(InteractionEventType type, Node *node, const Point3D *position = nullptr);
	};
	
	
	class CommanderController : public BTCharacterController, public SnapshotSender
	{
		private:
			
			unsigned long					commanderFlags;
			Link<Player>					commanderPlayer;
			
			float							primaryAzimuth;
			// float						modelAzimuth;   now BTEntityController::m_fAzimuth
			
			float							lookAzimuth;
			float							lookAltitude;
			float							deltaLookAzimuth;
			float							deltaLookAltitude;
			float							lookInterpolateParam;
			
			Vector3D						commanderVelocity;
			
			unsigned long					movementFlags;
			
			Vector3D						firingDirection;
			float							targetDistance;
			
			long							firingTime;
			
			CommanderInteractor				m_objCommanderInteractor;

			Ability							*m_pAbility1;
			Ability							*m_pAbility2;
			Ability							*m_pAbility3;

			unsigned long					m_ulCommanderRank;
			long							m_lKillCount;

			// Crosshair assistance (visualizes trajectory).
			BeamEffect						m_objCrossAssist;
			
			void SetOrientation(float azm, float alt)
			{
				lookAzimuth = azm;
				lookAltitude = alt;
				lookInterpolateParam = 0.0F;
			}
			
			void Initialize( const Point3D& position, float fRadius, float fHeight );
			
			bool UnpackChunk(const ChunkHeader *chunkHeader, Unpacker& data, unsigned long unpackFlags);
		
		protected:
			
			float GetInterpolatedLookAzimuth(void) const
			{
				return (lookAzimuth + deltaLookAzimuth * lookInterpolateParam);
			}
			
			float GetInterpolatedLookAltitude(void) const
			{
				return (lookAltitude + deltaLookAltitude * lookInterpolateParam);
			}
		
		public:

			CommanderController();
			CommanderController(const Point3D& position, unsigned long ulStringID, TableID lTableID, StatsProtocol *pStats, float fRadius, float fHeight, float fFloatHeight, long lTeam, float fOriMult, long startKills = 0);
			
			enum
			{
				kCommanderMessageEngageInteraction = kCharacterMessageBaseCount,
				kCommanderMessageDisengageInteraction,
				kCommanderMessageBeginMovement,
				kCommanderMessageEndMovement,
				kCommanderMessageChangeMovement,
				kCommanderMessageBeginIcon,
				kCommanderMessageEndIcon,
				kCommanderMessageTeleport,
				kCommanderMessageLaunch,
				kCommanderMessageDamage
			};
			
			~CommanderController();
			
			unsigned long GetCommanderFlags(void) const
			{
				return (commanderFlags);
			}
			
			void SetCommanderFlags(unsigned long flags)
			{
				commanderFlags = flags;
			}
			
			GamePlayer *GetCommanderPlayer(void) const
			{
				return (static_cast<GamePlayer *>(commanderPlayer.GetTarget()));
			}
			
			void SetCommanderPlayer(Player *player)
			{
				commanderPlayer = player;
			}

			long GetKillCount( void )
			{
				return (m_lKillCount);
			}
			
			float GetPrimaryAzimuth(void) const
			{
				return (primaryAzimuth);
			}
			
			void SetPrimaryAzimuth(float azimuth)
			{
				primaryAzimuth = azimuth;
			}
			
			float GetLookAzimuth(void) const
			{
				return (lookAzimuth);
			}
			
			void SetLookAzimuth(float azimuth)
			{
				lookAzimuth = azimuth;
			}
			
			float GetLookAltitude(void) const
			{
				return (lookAltitude);
			}
			
			void SetLookAltitude(float altitude)
			{
				lookAltitude = altitude;
			}

			// for abilities, uses the camera position
			float GetTargetAltitude( void );

			const Vector3D& GetCommanderVelocity(void) const
			{
				return (commanderVelocity);
			}
			
			unsigned long GetMovementFlags(void) const
			{
				return (movementFlags);
			}
			
			void SetMovementFlags(unsigned long flags)
			{
				movementFlags = flags;
			}
			
			const Vector3D& GetFiringDirection(void) const
			{
				return (firingDirection);
			}
			
			float GetTargetDistance(void) const
			{
				return (targetDistance);
			}
			
			void SetTargetDistance(float distance)
			{
				targetDistance = distance;
			}
			
			CommanderInteractor *GetCommanderInteractor(void)
			{
				return (&m_objCommanderInteractor);
			}
			
			const CommanderInteractor *GetCommanderInteractor(void) const
			{
				return (&m_objCommanderInteractor);
			}
			
			void Pack(Packer& data, unsigned long packFlags) const;
			void Unpack(Unpacker& data, unsigned long unpackFlags);
			
			ControllerMessage *ConstructMessage(ControllerMessageType type) const;
			void ReceiveMessage(const ControllerMessage *message);
			void SendSnapshot(void);
			
			void Preprocess(void);
			void EnterWorld(Zone *zone, const Point3D& zonePosition);
			
			void Move(void);
			void Travel(void);

			void Damage( float fDamage, long lEnemyIndex );
			
			void Kill( void );
			void PerformAbility( long lAbilityNumber );

			void AwardExperience( BTEntityController &objEntity );
			
			void UpdateOrientation(float azm, float alt);
			void BeginMovement(unsigned long flag, float azm, float alt);
			void EndMovement(unsigned long flag, float azm, float alt);
			void ChangeMovement(unsigned long flags, float azm, float alt);
			
			void BeginFiring(bool primary, float azm, float alt);
			void EndFiring(float azm, float alt);
					
			virtual void SetAnimation( EntityMotion motion );

			void InitializeCommanderInfoPane( CommanderInfoPane& pane );

			void EnableCrosshairAssistance( void );
			void DisableCrosshairAssistance( void );
	};
	
	
	class CreateCommanderMessage : public CreateModelMessage
	{
		private:
			
			float				initialAzimuth;
			float				initialAltitude;
			
			Vector3D			groundNormal;
			Vector3D			slideNormal[2];
			
			unsigned long		characterFlags;
			unsigned long		movementFlags;
			
			long				playerKey;

			unsigned long		m_ulEntityID;

			long				m_lStartKillCount;

			unsigned long		m_ulBaseIndex;
		
		protected:
		
		public:
			
			CreateCommanderMessage();
			CreateCommanderMessage(long index, unsigned long ulEntityID, const Point3D& position, float azm, float alt, const Vector3D& ground, const Vector3D& slide1, const Vector3D& slide2, unsigned long character, unsigned long movement, long key, long lStartKillCount, unsigned long ulBaseIndex );
			~CreateCommanderMessage();

			float GetInitialAzimuth(void) const
			{
				return (initialAzimuth);
			}
			
			float GetInitialAltitude(void) const
			{
				return (initialAltitude);
			}
			
			const Vector3D& GetGroundNormal(void) const
			{
				return (groundNormal);
			}
			
			const Vector3D& GetSlideNormal(long index) const
			{
				return (slideNormal[index]);
			}
			
			unsigned long GetCharacterFlags(void) const
			{
				return (characterFlags);
			}
			
			unsigned long GetMovementFlags(void) const
			{
				return (movementFlags);
			}
			
			long GetPlayerKey(void) const
			{
				return (playerKey);
			}

			unsigned long GetEntityID(void) const
			{
				return (m_ulEntityID);
			}

			long GetStartKillCount(void) const
			{
				return (m_lStartKillCount);
			}

			unsigned long GetBaseIndex( void ) const
			{
				return (m_ulBaseIndex);
			}
			
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
	
	
	class CommanderInteractionMessage : public ControllerMessage
	{
		friend class CommanderController;
		
		private:
			
			long		interactionControllerIndex;
		
		protected:
			
			CommanderInteractionMessage(ControllerMessageType type, long controllerIndex);
		
		public:
			
			CommanderInteractionMessage(ControllerMessageType type, long controllerIndex, long interactionIndex);
			~CommanderInteractionMessage();
			
			long GetInteractionControllerIndex(void) const
			{
				return (interactionControllerIndex);
			}
			
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
	
	
	class CommanderBeginInteractionMessage : public CommanderInteractionMessage
	{
		friend class CommanderController;
		
		private:
			
			Point3D		interactionPosition;
			
			CommanderBeginInteractionMessage(long controllerIndex);
		
		public:
			
			CommanderBeginInteractionMessage(long controllerIndex, long interactionIndex, const Point3D& position);
			~CommanderBeginInteractionMessage();
			
			const Point3D& GetInteractionPosition(void) const
			{
				return (interactionPosition);
			}
			
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
	
	
	class CommanderMovementMessage : public ControllerMessage
	{
		friend class CommanderController;
		
		private:
			
			Point3D			initialPosition;
			float			movementAzimuth;
			float			movementAltitude;
			unsigned long	movementFlag;
			
			CommanderMovementMessage(ControllerMessageType type, long controllerIndex);
		
		public:
			
			CommanderMovementMessage(ControllerMessageType type, long controllerIndex, const Point3D& position, const Vector3D& velocity, float azimuth, float altitude, unsigned long flag);
			~CommanderMovementMessage();

			const Point3D& GetInitialPosition(void) const
			{
				return (initialPosition);
			}
			
			float GetMovementAzimuth(void) const
			{
				return (movementAzimuth);
			}
			
			float GetMovementAltitude(void) const
			{
				return (movementAltitude);
			}
			
			long GetMovementFlag(void) const
			{
				return (movementFlag);
			}
			
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
	
	
	class CommanderTeleportMessage : public CharacterStateMessage
	{
		friend class CommanderController;
		
		private:
			
			float			teleportAzimuth;
			float			teleportAltitude;
			
			Point3D			effectCenter;
			
			CommanderTeleportMessage(long controllerIndex);
		
		public:
			
			CommanderTeleportMessage(long controllerIndex, const Point3D& position, const Vector3D& velocity, float azimuth, float altitude, const Point3D& center);
			~CommanderTeleportMessage();
			
			float GetTeleportAzimuth(void) const
			{
				return (teleportAzimuth);
			}
			
			float GetTeleportAltitude(void) const
			{
				return (teleportAltitude);
			}
			
			const Point3D& GetEffectCenter(void) const
			{
				return (effectCenter);
			}
			
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
	
	
	class CommanderUpdateMessage : public ControllerMessage
	{
		friend class CommanderController;
		
		private:
			
			Point3D		updatePosition;
			Vector3D	updateVelocity;
			
			float		updateAzimuth;
			float		updateAltitude;
			
			CommanderUpdateMessage(long controllerIndex);
		
		public:
			
			CommanderUpdateMessage(long controllerIndex, const Point3D& position, const Vector3D& velocity, float azimuth, float altitude);
			~CommanderUpdateMessage();
			
			const Point3D& GetUpdatePosition(void) const
			{
				return (updatePosition);
			}
			
			const Vector3D& GetUpdateVelocity(void) const
			{
				return (updateVelocity);
			}
			
			float GetUpdateAzimuth(void) const
			{
				return (updateAzimuth);
			}
			
			float GetUpdateAltitude(void) const
			{
				return (updateAltitude);
			}
			
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
	
	
	class CommanderDamageMessage : public ControllerMessage
	{
		friend class CommanderController;
		
		private:
			
			long		damageIntensity;
			Point3D		damageCenter;
			
			CommanderDamageMessage(long controllerIndex);
		
		public:
			
			CommanderDamageMessage(long controllerIndex, long intensity, const Point3D& center);
			~CommanderDamageMessage();
			
			long GetDamageIntensity(void) const
			{
				return (damageIntensity);
			}
			
			const Point3D& GetDamageCenter(void) const
			{
				return (damageCenter);
			}
			
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
}


#endif
