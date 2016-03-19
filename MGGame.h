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


#ifndef MGGame_h
#define MGGame_h


#include "C4Application.h"
#include "C4Configuration.h"
#include "C4World.h"
#include "MGInput.h"
#include "MGScripts.h"
#include "MGCameras.h"
#include "MGProperties.h"
#include "MGInterface.h"
#include "MGCrosshairs.h"
#include "BTCommander.h"
#include "MGEffects.h"
#include "BTControllers.h"
#include "StringTableManager.h"
#include "BTCursor.h"
#include "BTUnit.h"
#include "BTCacheManager.h"
#include "BTFish.h"
#include "BTVictoryController.h"
#include "BTScriptManager.h"
#include "BTEffects.h"

extern "C"
{
	module_export C4::Application *ConstructApplication(void);
}


// Blood Tide key bindings
#define FIELDCOMMANDER_BINDING "cfg/FieldCommander"
#define COMMANDER_BINDING "cfg/Commander"

// Distance entities should be spread apart
#define ENT_AVOID_DIST	0.05F

// Respawn time for commanders
#define COMMANDER_RESPAWN_TIME 10000 // in milliseconds

// Number of commands to send per message.
#define COMMANDS_PER_BATCH 10

// Player help message
#define COMMANDER_ATTACK_INTERVAL	10000
#define BASE_ATTACK_INTERVAL		10000
#define BUILDING_ATTACK_INTERVAL	10000
#define NO_SPAWN_INTERVAL			10000
#define BUILD_REMINDER_INTERVAL		240000
#define COMM_REMINDER_INTERVAL		240000
#define UNITS_REMINDER_INTERVAL		240000
#define NO_BUILD_INTERVAL			120000

// Number of possible units on each team.
#define MAX_UNITS	30

namespace C4
{
	enum
	{
		kInputXInverted			= 1 << 0,
		kInputYInverted			= 1 << 1
	};


	enum
	{
		kPlayerColorCount		= 11
	};


	// Locator types
	enum
	{
		kLocatorTeam1Spawn		= 'tea1',
		kLocatorTeam2Spawn		= 'tea2',
		kLocatorSpectator		= 'spec',
		kLocatorMapBounds		= 'mbnd',
		kLocatorVictory			= 'vict'
	};


	// Sound groups
	enum
	{
		kSoundGroupEffects		= 'efct',
		kSoundGroupMusic		= 'musi',
		kSoundGroupVoice		= 'voic'
	};


	// This is the subclass of World that this Game Module uses

	class GameWorld : public World
	{
		private:

			Marker					*teamSpawn1Locator;
			Marker					*teamSpawn2Locator;
			Marker					*spectatorLocator;
			Marker					*m_pMapBounds;
			Marker					*m_pVictory;

			SpectatorCamera			spectatorCamera;
			FirstPersonCamera		firstPersonCamera;
			ChaseCamera				chaseCamera;
			RTSCamera				commanderCamera;
			ModelCamera				*playerCamera; // the current field commander camera, either firstPersonCamera or chaseCamera
			CameraBlender			m_objCameraBlender;

			AmbientSource			*m_pBGMusic;	// Ambient source node that plays the background music.

			bool					m_bInCommandMode;
			bool					m_bInFieldMode;

			void CollectZoneMarkers(Zone *zone);

			// Cleanup any remaining characters. - FW
			void ClearCharacters( void );
			void ClearCharacterControllers( Array<long> *arrCharacters );

		public:

			GameWorld(const char *name);
			~GameWorld();

			bool GetInCommandMode( void )
			{
				return (m_bInCommandMode);
			}

			void SetInCommandMode( bool val )
			{
				m_bInCommandMode = val;
			}

			bool GetInFieldMode( void )
			{
				return (m_bInFieldMode);
			}

			void SetInFieldMode( bool val )
			{
				m_bInFieldMode = val;
			}

			Marker *GetTeamSpawnLocator(long lTeamID) const;

			Marker *GetSpectatorLocator( void ) const
			{
				return (spectatorLocator);
			}

			SpectatorCamera *GetSpectatorCamera(void)
			{
				return (&spectatorCamera);
			}

			RTSCamera& GetCommanderCamera(void)
			{
				return (commanderCamera);
			}

			ModelCamera *GetPlayerCamera(void)
			{
				return (playerCamera);
			}

			CameraBlender& GetCameraBlender(void)
			{
				return (m_objCameraBlender);
			}

			bool UsingFirstPersonCamera(void) const
			{
				return (playerCamera == &firstPersonCamera);
			}

			Marker* GetMapBounds( void ) const
			{
				return (m_pMapBounds);
			}

			Marker* GetVictoryLocator( void ) const
			{
				return (m_pVictory);
			}

			ResourceResult Preprocess(void);
			void Interact(void);

			void BeginRendering(void);
			void EndRendering(void);

			void SetCameraTargetModel(Model *model);
			void SetSpectatorCamera(const Point3D& position, float azm, float alt);

			void SetLocalPlayerVisibility(void);
			void ChangePlayerCamera(void);
			void SetFocalLength(float focal);

			MaterialObject *GetGameMaterial(long index) const;

			// Traverses the entire scene graph, extracting information - CW
			void TraverseSceneGraph( void );
			void InspectNode( Node *pNode );

			// Start background music based on team. - FW
			void StartMusic( void );

			// Create a sound in the world itself.
			void CreateWorldSound( const char *pSound, Zone *pZone, const Point3D& p3dLocation, float fRange );

			// Check to see if a point is within map bounds. - FW
			bool PointWithinMapBounds( Point2D& p2dToTest );
			bool PointWithinMapBounds( Point3D& p3dToTest );
	};


	// This is the Game class that must be constructed when the
	// ConstructApplication() function is called

	class Game : public Application, public Singleton<Game>
	{
		private:

			unsigned long							m_ulWorldID; // The world's string table ID

			const StringTable						stringTable;

			CacheManager							m_objGameCache; // fw

			Constructor<Property>					propertyConstructor;
			DisplayEventHandler						displayEventHandler;

			ControllerReg<CommanderController>		commanderControllerReg;
			ControllerReg<UnitController>			m_UnitControllerReg; // fw

			PropertyReg<JumpProperty>				jumpPropertyReg;

			MethodReg<DisplayMessageMethod>			displayMessageReg;

			ParticleSystemReg<WaterfallMist>		waterfallMistReg;
			ParticleSystemReg<WaterfallSplash>		waterfallSplashReg;
			ParticleSystemReg<WaterfallShimmer>		waterfallShimmerReg;

			Array<ModelRegistration*>				m_arrUnitModelReg; // fw

			LocatorRegistration						team1SpawnLocatorReg;
			LocatorRegistration						team2SpawnLocatorReg;
			LocatorRegistration						mbLocatorReg; // fw
			LocatorRegistration						spectatorLocatorReg;
			LocatorRegistration						victLocatorReg; // fw

			Variable								*fovVariable;
			Variable								*crossSizeVariable;
			Variable								*effectsVolumeVariable;
			Variable								*musicVolumeVariable;
			Variable								*voiceVolumeVariable;
			Variable								*voiceReceiveVariable;
			Variable								*voiceSendVariable;
			Variable								*firstLoad;

			SoundGroup								effectsSoundGroup;
			SoundGroup								musicSoundGroup;
			SoundGroup								voiceSoundGroup;

			MovementAction							forwardAction;
			MovementAction							backwardAction;
			MovementAction							leftAction;
			MovementAction							rightAction;
			MovementAction							upAction;
			MovementAction							downAction;
			MovementAction							movementAction;
			LookAction								horizontalAction;
			LookAction								verticalAction;
			FireAction								primaryFireAction;
			FireAction								secondaryFireAction;
			SwitchAction							cameraViewAction;
			SwitchAction							scoreboardAction;
			ChatAction								chatAction;

			// World editor controllers
			ControllerReg<MapBoundsController>		mbControllerReg; // fw
			ControllerReg<BaseController>			baseControllerReg;	 // gd
			ControllerReg<PlotController>			plotControllerReg;	 // gd
			ControllerReg<SimpleFishController>		fishControllerReg; // fw
			ControllerReg<BasesDestroyed_VictoryController>		victControllerReg; // fw

			// Blood Tide Actions - CW
			NavigationAction						navUpAction;
			NavigationAction						navLeftAction;
			NavigationAction						navDownAction;
			NavigationAction						navRightAction;

			UnitOrderAction							orderAttackToAction;
			UnitOrderAction							orderStopAction;

			FieldCommanderAction					fieldCommanderAbility1Action;
			FieldCommanderAction					fieldCommanderAbility2Action;
			FieldCommanderAction					fieldCommanderAbility3Action;

			SwitchBTModesAction						switchBTModesAction;

			ShowCursorAction						showCursorAction;

			SelectAllUnitsAction					selectAllUnitsAction;
			SendAllUnitToComAction					sendAllUnitsToComAction;

			GroupUnitsAction						groupUnits1;
			GroupUnitsAction						groupUnits2;
			GroupUnitsAction						groupUnits3;

			BTCursor								m_objCursor;

			ImageElement							crosshair;

			List<GameInterface>						interfaceList;
			List<GameWindow>						windowList;

			InputMgr::KeyProc						*prevEscapeProc;
			void									*prevEscapeData;

			unsigned long							inputFlags;
			unsigned long							multiplayerFlags;

			long									lookSensitivity;
			float									lookMultiplierX;
			float									lookMultiplierY;

			float									lookSpeedX;
			float									lookSpeedY;

			// Sound - FW
			Sound									*m_pUISound;
			Sound									*m_pHelpSound;

			MaterialObject							*m_pStealthShader; // TODO: find a better place for this

			Action									*m_pConsoleAction;

			static World *ConstructWorld(const char *name, void *data);
			static Player *ConstructPlayer(PlayerKey key, void *data);

			static Property *ConstructProperty(Unpacker& data, unsigned long unpackFlags);

			static void HandleDisplayEvent(EventType eventType, long param, void *data);
			static void EscapeProc(void *data);

			static void SensitivityProc(Variable *variable, void *data);
			static void InvertXProc(Variable *variable, void *data);
			static void InvertYProc(Variable *variable, void *data);

			static void FovProc(Variable *variable, void *data);
			static void EffectsVolumeProc(Variable *variable, void *data);
			static void MusicVolumeProc(Variable *variable, void *data);
			static void VoiceVolumeProc(Variable *variable, void *data);
			static void VoiceReceiveProc(Variable *variable, void *data);
			static void VoiceSendProc(Variable *variable, void *data);

			static void InitializeModel(const CreateModelMessage *message, GameWorld *world, Model *model, Controller *controller);
			static void InitializeEffect(const CreateEffectMessage& message, Effect& effect, Controller& effectController, BTEntityController& target);

			// Opens the game lobby and initializes input - CW
			void StartBloodTideLobby(unsigned long ulWorldID);

			// Cleanup lazily registered models. - FW
			void ClearModelRegList( void );

			// Player help messages
			bool			m_bIsHelpActivte;
			unsigned long	m_ulNextCommAttackedMsg;	// next time commander attacked message can be displayed
			unsigned long	m_ulNextBaseAttackedMsg;	// next time base attacked message can be displayed
			unsigned long	m_ulNextBuildAttackedMsg;	// next time building attacked message can be displayed
			unsigned long	m_ulNextBuildMsg;			// next time building reminder can be displayed
			unsigned long	m_ulNextCommMsg;			// next time commander reminder message can be displayed
			unsigned long	m_ulNextUnitsMsg;			// next time units reminder message can be displayed
			unsigned long	m_ulNextNoSpawnMsg;			// next time no spawn message can be displayed
			unsigned long	m_ulNextNoBuildMsg;			// next time can not build message can be displayed

			// Popup screens
			PopupScreen		m_objPlayTutorial;
			PopupScreen		m_objVictory;
			PopupScreen		m_objDefeat;
			PopupScreen		m_objCredits;
			PopupScreen		m_objControls;

		public:

			// Called after the game world is loaded to initialize a Blood Tide game session - CW
			void InitializeBloodTideSession(void);

			Game();
			~Game();

			const StringTable *GetStringTable(void) const
			{
				return (&stringTable);
			}

			SoundGroup *GetVoiceSoundGroup(void)
			{
				return (&voiceSoundGroup);
			}

			BTCursor *GetCursor(void)
			{
				return (&m_objCursor);
			}

			Element *GetCrosshairs(void)
			{
				return (&crosshair);
			}

			void AddInterface(GameInterface *interface)
			{
				interfaceList.Append(interface);
				TheInterfaceMgr->AddElement(interface);
			}

			void AddWindow(GameWindow *window)
			{
				windowList.Append(window);
				TheInterfaceMgr->AddElement(window);
			}

			unsigned long GetInputFlags(void) const
			{
				return (inputFlags);
			}

			unsigned long GetMultiplayerFlags(void) const
			{
				return (multiplayerFlags);
			}

			long GetLookSensitivity(void) const
			{
				return (lookSensitivity);
			}

			float GetLookSpeedX(void)
			{
				return (lookSpeedX * lookMultiplierX);
			}

			float GetLookSpeedY(void)
			{
				return (lookSpeedY * lookMultiplierY);
			}

			void UpdateLookSpeedX(float value)
			{
				lookSpeedX = value;
			}

			void UpdateLookSpeedY(float value)
			{
				lookSpeedY = value;
			}

			float GetCameraFocalLength(void) const;

			void ClearInterface(void);
			WorldResult StartSinglePlayerGame(const char *name);
			void HostMultiplayerGame(const char *name, unsigned long ulWorldID, unsigned long flags);
			void JoinMultiplayerGame(const char *name, unsigned long ulWorldID, unsigned long flags);
			void ExitCurrentGame(bool unload = true, bool end = false);

			void HandleConnectionEvent(ConnectionEvent event, const NetworkAddress& address, const void *param);
			void HandlePlayerEvent(PlayerEvent event, Player *player, const void *param);
			void HandleGameEvent(GameEvent event, const void *param);

			Message *ConstructMessage(MessageType type, Decompressor& data) const;
			void ReceiveMessage(Player *sender, const NetworkAddress& address, const Message *message);

			void CreateModel(const CreateModelMessage *message);
			void CreateEffect(const CreateEffectMessage *message);
			//void SpawnPlayer(Player *player);

			static void ProcessGeometryProperties(const Geometry *geometry, const Point3D& position, const Vector3D& impulse);

			void RefreshScoreboard(const RefreshScoreboardMessage *message);

			EngineResult LoadWorld(const char *name);
			void UnloadWorld(void);

			void ApplicationTask(void);
			void InterfaceRenderTask(void);

			void DetectEntityCollision( BTEntityController *pController, BTCollisionData& rstData, unsigned long ulCacheFlags, float fHeading, float fLookAheadDist, Array<long>& rIgnoreConts, bool bIs3D = true );
			void DetectEntityCollision( BTEntityController *pController, BTCollisionData& rstData, float fHeading, float fLookAheadDist, Array<long> *arrColliders, float& fClosestSoFar, Array<long>& rIgnoreConts, bool bIs3D = true );

			void DetectEntity( Point3D& p3dPosition, BTCollisionData& rstData, unsigned long ulCacheFlags, float fSafeDist, Array<long>& rIgnoreConts, bool bIs3D = true );
			void DetectEntity( Point3D& p3dPosition, BTCollisionData& rstData, Array<long> *arrColliders, float& fClosestSoFar, float fSafeDist, Array<long>& rIgnoreConts, bool bIs3D = true );

			void DetectEntity( Ray& cameraRay, BTCollisionData& rstData, unsigned long ulCacheFlags, Array<long>& rIgnoreConts, bool bIs3D = true );
			void DetectEntity( Ray& cameraRay, BTCollisionData& rstData, Array<long> *arrColliders, float& fClosestSoFar, Array<long>& rIgnoreConts, bool bIs3D = true );

			void DetectEntity( Point3D& p3dStart, Point3D& p3dEnd, float fRadius, BTCollisionData& rstData, Array<long> *arrColliders, float& fClosestSoFar, Array<long>& rIgnoreConts );

			void DetectEntities( Point3D& p3dPosition, float fRadius, Array<long>& arrData, Array<long> *arrColliders, Array<long>& rIgnoreConts, bool bIs3D = true );


			// Remaps the game's key bindings to the configuration found in the given file path - CW
			// TODO: Make a default key binding if the inputfile is bad?
			void ChangeKeyBindings(const char *strInputFile);

			// Changes the game mode to command mode if players has a Commander in play - CW
 			void ToCommandMode();

			// Changes the player to field mode - CW
			void ToFieldMode( bool instant = false );

			// Called by the transition camera when it is finished - CW
			static void FinishFieldModeTransition( void );

			// When an entity that can give experience dies (e.g. a unit or commander),
			// it should call this function to give the other team's commanders experience - CW
			void AwardExperience( BTEntityController &objEntity );

			// UI Sounds - FW
			void PlayUISound( const char *pSound, float fVol, long lLoopAmt );
			void StopUISound( void );

			// Help Sounds - FW
			void PlayHelpSound( const char *pSound, float fVol, long lLoopAmt );
			void StopHelpSound( void );

			bool IsReadyForBuilding(long team);

			/**
			 * Since models are loaded from a string table and not hardcoded,
			 * they need to be registered upon use.
			 * @author - Frank Williams (demalus@gmail.com)
			 */
			void LazyModelRegistration(ModelType type, TableID nStringTableID);

			// Messages sent to the MessageInterface (and optionally play a sound).
			void HelpMessage( unsigned long ulMsgID );
			void HelpMessage( const char *msg, const char *snd );

			// The currently loaded game world.
			unsigned long GetWorldID( void ) const
			{
				return (m_ulWorldID);
			}

			void SetWorldID( unsigned long ulWorldID )
			{
				m_ulWorldID = ulWorldID;
			}

			// Scripts
			void CheckVictoryConditions( void );

			MaterialObject *const *GetStealthShader( void );

			// Player help messages

			void UnderAttackMessage( long team, ControllerType type );

			bool IsHelpActive( void )
			{
				return (m_bIsHelpActivte);
			}

			void SetHelpActive( bool bHelp )
			{
				m_bIsHelpActivte = bHelp;
			}

			unsigned long GetNextCommAttackTime( void )	// alert commander under attack
			{
				return (m_ulNextCommAttackedMsg);
			}

			unsigned long GetNextBuildAttackTime( void )	// alert building under attack
			{
				return (m_ulNextBuildAttackedMsg);
			}

			unsigned long GetNextBaseAttackTime( void )	// alert base under attack
			{
				return (m_ulNextBaseAttackedMsg);
			}

			unsigned long GetNextBuildRemTime( void ) // reminder to build a building
			{
				return (m_ulNextBuildMsg);
			}

			unsigned long GetNextCommRemTime( void )	// reminder to use commander
			{
				return (m_ulNextCommMsg);
			}

			unsigned long GetNextUnitsRemTime( void )	// reminder to use units
			{
				return (m_ulNextUnitsMsg);
			}

			unsigned long GetNextNoSpawnTime( void )	// no more units can spawn
			{
				return (m_ulNextNoSpawnMsg);
			}

			unsigned long GetNextNoBuildTime( void )	// can only build one building at a time
			{
				return (m_ulNextNoBuildMsg);
			}

			void SetNextCommAttackTime( unsigned long ulTime )
			{
				m_ulNextCommAttackedMsg = ulTime;
			}

			void SetNextBaseAttackTime( unsigned long ulTime )
			{
				m_ulNextBaseAttackedMsg = ulTime;
			}

			void SetNextBuildAttackTime( unsigned long ulTime )
			{
				m_ulNextBuildAttackedMsg = ulTime;
			}

			void SetNextBuildRemTime( unsigned long ulTime )
			{
				m_ulNextBuildMsg = ulTime;
			}

			void SetNextCommRemTime( unsigned long ulTime )
			{
				m_ulNextCommMsg = ulTime;
			}

			void SetNextUnitsRemTime( unsigned long ulTime )
			{
				m_ulNextUnitsMsg = ulTime;
			}

			void SetNextNoSpawnTime( unsigned long ulTime )
			{
				m_ulNextNoSpawnMsg = ulTime;
			}

			void SetNextNoBuildTime( unsigned long ulTime )
			{
				m_ulNextNoBuildMsg = ulTime;
			}

			// Get the popup screens

			PopupScreen *GetPopScreen_PlayTutorial( void )
			{
				return (&m_objPlayTutorial);
			}

			PopupScreen *GetPopScreen_Victory( void )
			{
				return (&m_objVictory);
			}

			PopupScreen *GetPopScreen_Defeat( void )
			{
				return (&m_objDefeat);
			}

			PopupScreen *GetPopScreen_Credits( void )
			{
				return (&m_objCredits);
			}

			PopupScreen *GetPopScreen_Controls( void )
			{
				return (&m_objControls);
			}
	};


	extern Game *TheGame;
}


#endif
