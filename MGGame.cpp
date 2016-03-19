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


#include "C4ToolWindows.h"
#include "C4AudioCapture.h"
#include "MGGame.h"
#include "StringTableManager.h"
#include "BTCursor.h"
#include "BTLobby.h"
#include "BTHUD.h"
#include "BTCommanderUI.h"
#include "BTInfoPane.h"
#include "BTBuildingUI.h"
#include "MGMultiplayer.h"
#include "BTMath.h"
#include "BTLoadingTip.h"
#include "BTStatEffects.h"
#include "MGConfiguration.h"

using namespace C4;

// Zone generation constants - FW
#define ZONEGEN_X 15.0F
#define ZONEGEN_Y 15.0F
#define ZONEGEN_Z 80.0F

namespace
{
	// This is how close the camera needs to be to an interactive object in order to use it.
	const float kInteractionDistance = 2.0F;
}


Game *C4::TheGame = nullptr;

C4::Application *ConstructApplication(void)
{
	return (new Game);
}


GameWorld::GameWorld(const char *name) :
		World( name ),
		m_bInCommandMode( false ),
		m_bInFieldMode( false ),
		spectatorCamera( TheGame->GetCameraFocalLength(), 1.0F, 0.3F ),
		teamSpawn1Locator( nullptr ),
		teamSpawn2Locator( nullptr ),
		m_pMapBounds( nullptr ),
		m_pBGMusic( nullptr ),
		spectatorLocator( nullptr ),
		m_pVictory( nullptr )
{
}

GameWorld::~GameWorld()
{
	ClearCharacters();
	TheGameCacheMgr->Purge();
}

void GameWorld::ClearCharacters( void )
{
	if( !TheGameCacheMgr ) {
		return;
	}

	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheUnits, 1));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheUnits, 2));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheCommanders, 1));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheCommanders, 2));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheBases, 1));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheBases, 2));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCachePlots, 1));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCachePlots, 2));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheBuildings, 1));
	ClearCharacterControllers( TheGameCacheMgr->GetGameCache(kCacheBuildings, 2));
}

void GameWorld::ClearCharacterControllers( Array<long> *arrCharacters )
{
	if( !arrCharacters ) {
		return;
	}

	World *world = TheWorldMgr->GetWorld(); // TODO: Why would this be the case?
	if( world == nullptr ) return;

	int nLength = arrCharacters->GetElementCount();
	for( int x = 0; x < nLength; x++ ) {
		long cIndex = (*arrCharacters)[x];
		Controller *cont = world->GetController(cIndex);
		if( cont ) {
			if( cont->GetTargetNode() )
				delete cont->GetTargetNode();
			else
				delete cont;
		}
	}
}

ResourceResult GameWorld::Preprocess(void)
{
	ResourceResult result = World::Preprocess();
	if (result != kResourceOkay) return (result);

	SetCamera(&spectatorCamera);
	playerCamera = &chaseCamera;

	CollectZoneMarkers(GetRootNode());

	const Marker *marker = GetSpectatorLocator();
	if (marker)
	{
		const Vector3D direction = marker->GetWorldTransform()[0];
		float azimuth = Atan(direction.y, direction.x);
		float altitude = Atan(direction.z, Sqrt(direction.x * direction.x + direction.y * direction.y));

		spectatorCamera.SetCameraAzimuth(azimuth);
		spectatorCamera.SetCameraAltitude(altitude);
		spectatorCamera.SetNodePosition(marker->GetWorldPosition());
	}
	else
	{
		spectatorCamera.SetNodePosition(Point3D(0.0F, 0.0F, 1.0F));
	}

	TraverseSceneGraph();

	return (kResourceOkay);
}

Marker *GameWorld::GetTeamSpawnLocator(long lTeamID) const
{
	if( lTeamID == 1 )
		return teamSpawn1Locator;

	if( lTeamID == 2 )
		return teamSpawn2Locator;

	return nullptr;
}

void GameWorld::TraverseSceneGraph( void )
{
	Node *root = GetRootNode();
	Node *node = root;

	while( node ) {
		InspectNode( node );
		node = root->GetNextNode(node);
	}
}

// Inspect node to see if something should be done with it.
void GameWorld::InspectNode( Node *pNode ) {
	if( !pNode ) {
		return;
	}

	// Check for ambient music source.
	// The source plays for any given player -> the song comes from the Music string table according to which team the player is on.
	if( pNode->GetNodeName() && Text::CompareText(pNode->GetNodeName(), "bg_music") ) {
		m_pBGMusic = static_cast<AmbientSource*>( pNode );
		return;
	}


	// Check controller.
	Controller *controller = pNode->GetController();
	if( controller ) {

		switch( controller->GetControllerType() )
		{
			case kControllerBase:
			{
				BaseController *baseController = static_cast<BaseController *>(controller);
				if( !baseController ) {
					return;
				}

				long team = baseController->GetTeam();

				Array<long> *baseCache = TheGameCacheMgr->GetGameCache(kCacheBases, team);
				if( baseCache ) baseCache->AddElement(baseController->GetControllerIndex());

				//TheGame->LazyModelRegistration( baseController->GetCommanderType(), kStringTableCommanders );
				//TheGame->LazyModelRegistration( baseController->GetUnitType(), kStringTableUnits );

				return;
			}

			case kControllerPlot:
			{
				PlotController *plotController = static_cast<PlotController *>(controller);
				if( !plotController ) {
					return;
				}

				long team = plotController->GetTeam();

				Array<long> *plotCache = TheGameCacheMgr->GetGameCache(kCachePlots, team);
				if( plotCache ) plotCache->AddElement(plotController->GetControllerIndex());

				return;
			}
		}
	}

}

// Once the source has been found, start playing the factional music.
void GameWorld::StartMusic( void )
{
	if( !m_pBGMusic ) {
		return;
	}

	if( !m_pBGMusic->GetObject() ) {
		return;
	}

	if( !TheStringTableMgr ) {
		return;
	}

	const StringTable *music_table = TheStringTableMgr->GetStringTable( kStringTableMusic );
	if( !music_table ) {
		return;
	}

	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( !player ) {
		return;
	}

	m_pBGMusic->Stop();

	if( player->GetPlayerTeam() == 1 ) {
		m_pBGMusic->GetObject()->AddSound( music_table->GetString(StringID('TEA1', 'sng1')) );
		m_pBGMusic->SetSourceVolume( Text::StringToFloat(music_table->GetString(StringID('TEA1', 'sng1', 'vol'))) );
	}
	else {
		m_pBGMusic->GetObject()->AddSound( music_table->GetString(StringID('TEA2', 'sng1')) );
		m_pBGMusic->SetSourceVolume( Text::StringToFloat(music_table->GetString(StringID('TEA2', 'sng1', 'vol'))) );
	}

	m_pBGMusic->Play();
}

void GameWorld::CreateWorldSound( const char *pSound, Zone *pZone, const Point3D& p3dLocation, float fRange )
{
	if( !pSound ) {
		return;
	}

	if( !pZone ) {
		return;
	}

	OmniSource *source = new OmniSource( pSound, fRange );
	source->SetNodePosition( pZone->GetInverseWorldTransform() * p3dLocation );
	pZone->AddNewSubnode( source );
}

void GameWorld::CollectZoneMarkers(Zone *zone)
{
	Marker *marker = zone->GetFirstMarker();
	while (marker)
	{
		Marker *next = marker->Next();

		if ((marker->GetMarkerType() == kMarkerLocator) && (!(marker->GetNodeFlags() & kNodeDisabled)))
		{
			LocatorMarker *locator = static_cast<LocatorMarker *>(marker);
			switch (locator->GetLocatorType())
			{
				case kLocatorTeam1Spawn:

					teamSpawn1Locator = locator;
					break;

				case kLocatorTeam2Spawn:

					teamSpawn2Locator = locator;
					break;


				case kLocatorSpectator:

					spectatorLocator = locator;
					break;

				case kLocatorMapBounds: // fw

					m_pMapBounds = locator;
					break;

				case kLocatorVictory: // fw

					m_pVictory = locator;
					break;
			}
		}

		marker = next;
	}

	Zone *subzone = zone->GetFirstSubzone();
	while (subzone)
	{
		CollectZoneMarkers(subzone);
		subzone = subzone->Next();
	}
}

void GameWorld::Interact(void)
{
	//bool server = TheMessageMgr->Server();
	//const Player *localPlayer = TheMessageMgr->GetLocalPlayer();
	//
	//const Player *player = TheMessageMgr->GetFirstPlayer();
	//while (player)
	//{
	//	CommanderController *controller = static_cast<const GamePlayer *>(player)->GetPlayerController();
	//	if (controller)
	//	{
	//		if ((server) || (player == localPlayer))
	//		{
	//			CollisionData	data;
	//
	//			const Point3D& p = controller->GetTargetNode()->GetWorldPosition();
	//			Point3D position(p.x, p.y, p.z + kCameraPositionHeight);
	//
	//			const Vector3D& direction = controller->GetFiringDirection();
	//			controller->GetCommanderInteractor()->SetInteractionProbe(position, position + direction * kInteractionDistance);
	//
	//			Player *sightPlayer = nullptr;
	//			float targetDistance = 0.0F;
	//
	//			// Fire a 100-meter ray into the world in the direction that the player is looking.
	//
	//			CollisionState state = QueryWorld(position, position + direction * 100.0F, 0.0F, kColliderSightPath, &data);
	//			if (state != kCollisionStateNone)
	//			{
	//				if (state == kCollisionStateGeometry)
	//				{
	//					// The ray hit a geometry node. Set the target distance to be the distance to the geometry,
	//					// but only if the player is not looking through a teleporting portal.
	//
	//					const Geometry *geometry = data.geometry;
	//					const Controller *geometryController = geometry->GetController();
	//					if (!geometryController) targetDistance = data.param * 100.0F;
	//				}
	//				else if (state == kCollisionStateCollider)
	//				{
	//					// The ray hit a collider. Set the target distance to be the distance to the collider.
	//
	//					targetDistance = data.param * 100.0F;
	//
	//					const Collider *collider = data.collider;
	//					if (collider->GetControllerType() == kControllerCommander)
	//					{
	//						// If the collider is another player, then remember who it is so we can display the name.
	//
	//						sightPlayer = static_cast<const CommanderController *>(collider)->GetCommanderPlayer();
	//					}
	//				}
	//			}
	//
	//			controller->SetTargetDistance(targetDistance);
	//
	//			if ((player == localPlayer) && (TheMessageMgr->Multiplayer())) TheNamePopupInterface->SetPlayer(sightPlayer);
	//		}
	//	}
	//
	//	player = player->Next();
	//}

	//World::Interact();
}

void GameWorld::BeginRendering(void)
{
	// If you want to set a post-processing color matrix,
	// make the call to SetFinalColorTransform() here before
	// calling World::BeginRendering().

	World::BeginRendering();
}

void GameWorld::EndRendering(void)
{
	World::EndRendering();

  	const Player *player = TheMessageMgr->GetLocalPlayer();
	if (player)
	{
		if( m_bInCommandMode )
		{

		}
		else if( m_bInFieldMode )
		{
			//const CommanderController *controller = static_cast<const GamePlayer *>(player)->GetPlayerController();
			//if (controller)
			//{
			//	const Node *node = controller->GetCommanderInteractor()->GetInteractionNode();
			//	if ((!node) || (node->GetNodeType() != kNodeEffect))
			//	{
			//		Element *crosshair = TheGame->GetCrosshairs();

			//		if( crosshair )
			//		{
			//			crosshair->SetElementPosition( Point3D( TheDisplayMgr->GetDisplayWidth() / 2.0F - 32.0F, TheDisplayMgr->GetDisplayHeight() / 2.0F - 32.0F, 0.0f));
			//			crosshair->SetActiveUpdateFlags( crosshair->GetActiveUpdateFlags() | Node::kUpdateTransform );
			//			crosshair->Update();

			//			TheInterfaceMgr->RemoveElement( crosshair );
			//			TheInterfaceMgr->AddElement( crosshair );
			//		}
			//	}
			//}
		}
	}
}

void GameWorld::SetCameraTargetModel(Model *model)
{
	firstPersonCamera.SetTargetModel(model);
	chaseCamera.SetTargetModel(model);
	SetCamera(playerCamera);
}

void GameWorld::SetSpectatorCamera(const Point3D& position, float azm, float alt)
{
	firstPersonCamera.SetTargetModel(nullptr);
	chaseCamera.SetTargetModel(nullptr);

	SetCamera(&spectatorCamera);
	spectatorCamera.SetNodePosition(position);
	spectatorCamera.SetCameraAzimuth(azm);
	spectatorCamera.SetCameraAltitude(alt);
}

void GameWorld::SetLocalPlayerVisibility(void)
{
	const Player *player = TheMessageMgr->GetLocalPlayer();
	if (player)
	{
		const CommanderController *controller = static_cast<const GamePlayer *>(player)->GetPlayerController();
		if (controller)
		{
			unsigned long mask = (playerCamera == &firstPersonCamera) ? kPerspectiveDirect | kPerspectiveRefraction : 0;
			controller->SetPerspectiveExclusionMask(mask);
		}
	}
}

void GameWorld::ChangePlayerCamera(void)
{
	const Player *player = TheMessageMgr->GetLocalPlayer();
	if ((player) && (static_cast<const GamePlayer *>(player)->GetPlayerController()))
	{
		if (playerCamera == &firstPersonCamera) playerCamera = &chaseCamera;
		else playerCamera = &firstPersonCamera;

		SetCamera(playerCamera);
		SetLocalPlayerVisibility();
	}
}

void GameWorld::SetFocalLength(float focal)
{
	spectatorCamera.GetObject()->SetFocalLength(focal);
	firstPersonCamera.GetObject()->SetFocalLength(focal);
	chaseCamera.GetObject()->SetFocalLength(focal);
	commanderCamera.GetObject()->SetFocalLength(focal);
	m_objCameraBlender.GetObject()->SetFocalLength(focal);
}

MaterialObject *GameWorld::GetGameMaterial(long index) const
{
	Node *node = GetRootNode();

	PropertyType type = kPropertyGameMaterial + index;
	Property *property = node->GetProperty(type);
	if (property) return (static_cast<MaterialProperty *>(property)->GetMaterialObject());

	MaterialObject *object = new MaterialObject;
	node->AddProperty(new MaterialProperty(type, object));
	object->Release();

	switch (index)
	{
		case kGameMaterialSplatter1:
		case kGameMaterialSplatter2:
		case kGameMaterialSplatter3:
		case kGameMaterialSplatter4:
		case kGameMaterialSplatter5:
		{
			static char textureName[18] = "texture/Splatter1";
			textureName[16] = index - kGameMaterialSplatter1 + '1';

			object->AddAttribute(new TextureMapAttribute(textureName));
			object->SetMaterialFlags(kMaterialAlphaTest);
			break;
		}
	}

	return (object);
}

// Check if a point is within map bounds.
bool GameWorld::PointWithinMapBounds( Point2D& p2dToTest )
{
	if( !m_pMapBounds ) {
		return true;
	}

	MapBoundsController *cont = static_cast<MapBoundsController*>( m_pMapBounds->GetController() );
	if( !cont ) {
		return true;
	}

	if( p2dToTest.x < m_pMapBounds->GetNodePosition().x ) return false;
	if( p2dToTest.x > m_pMapBounds->GetNodePosition().x + cont->GetTopRightVector().x ) return false;
	if( p2dToTest.y < m_pMapBounds->GetNodePosition().y ) return false;
	if( p2dToTest.y > m_pMapBounds->GetNodePosition().y + cont->GetTopRightVector().y ) return false;

	return true;
}

bool GameWorld::PointWithinMapBounds( Point3D& p3dToTest )
{
	return PointWithinMapBounds( p3dToTest.GetPoint2D() );
}


Game::Game() :
		Singleton<Game>(TheGame),
		stringTable("string_table/Game"),

		propertyConstructor(&ConstructProperty),
		displayEventHandler(&HandleDisplayEvent),

		commanderControllerReg(kControllerCommander, nullptr),
		m_UnitControllerReg(kControllerUnit, nullptr),

		jumpPropertyReg(kPropertyJump, stringTable.GetString(StringID('PROP', kPropertyJump))),

		displayMessageReg(kMethodDisplayMessage, stringTable.GetString(StringID('MTHD', kMethodDisplayMessage)), kMethodNoTarget),

		waterfallMistReg(kParticleSystemWaterfallMist, stringTable.GetString(StringID('PART', kParticleSystemWaterfallMist))),
		waterfallSplashReg(kParticleSystemWaterfallSplash, stringTable.GetString(StringID('PART', kParticleSystemWaterfallSplash))),
		waterfallShimmerReg(kParticleSystemWaterfallShimmer, stringTable.GetString(StringID('PART', kParticleSystemWaterfallShimmer))),

		team1SpawnLocatorReg(kLocatorTeam1Spawn, stringTable.GetString(StringID('LOCA', kLocatorTeam1Spawn))),
		team2SpawnLocatorReg(kLocatorTeam2Spawn, stringTable.GetString(StringID('LOCA', kLocatorTeam2Spawn))),
		mbLocatorReg(kLocatorMapBounds, stringTable.GetString(StringID('LOCA', kLocatorMapBounds))),
		spectatorLocatorReg(kLocatorSpectator, stringTable.GetString(StringID('LOCA', kLocatorSpectator))),
		victLocatorReg(kLocatorVictory, stringTable.GetString(StringID('LOCA', kLocatorVictory))),

		// World editor controllers.
		mbControllerReg(kControllerMapBounds, stringTable.GetString(StringID('CTRL', kControllerMapBounds))),
		baseControllerReg(kControllerBase, stringTable.GetString(StringID('CTRL', kControllerBase))),
		plotControllerReg(kControllerPlot, stringTable.GetString(StringID('CTRL', kControllerPlot))),
		fishControllerReg(kControllerSimpleFish, stringTable.GetString(StringID('CTRL', kControllerSimpleFish))),
		victControllerReg(kControllerVictory, stringTable.GetString(StringID('CTRL', kControllerVictory))),

		effectsSoundGroup(kSoundGroupEffects, "Effects"),
		musicSoundGroup(kSoundGroupMusic, "Music"),
		voiceSoundGroup(kSoundGroupVoice, "Voice"),

		// Movement actions.

		forwardAction(kActionForward, kMovementForward, kSpectatorMoveForward),
		backwardAction(kActionBackward, kMovementBackward, kSpectatorMoveBackward),
		leftAction(kActionLeft, kMovementLeft, kSpectatorMoveLeft),
		rightAction(kActionRight, kMovementRight, kSpectatorMoveRight),
		upAction(kActionUp, kMovementUp, kSpectatorMoveUp),
		downAction(kActionDown, kMovementDown, kSpectatorMoveDown),
		movementAction(kActionMovement, 0, 0),
		horizontalAction(kActionHorizontal),
		verticalAction(kActionVertical),

		// Weapon firing actions.

		primaryFireAction(kActionFirePrimary),
		secondaryFireAction(kActionFireSecondary),

		// Miscellaneous actions.

		cameraViewAction(kActionCameraView),
		scoreboardAction(kActionScoreboard),

		// Blood Tide Actions - CW
		navUpAction(kActionNavUp, kNavUp),
		navLeftAction(kActionNavLeft, kNavLeft),
		navDownAction(kActionNavDown, kNavDown),
		navRightAction(kActionNavRight, kNavRight),

		orderAttackToAction(kActionAttackToOrder, kOrderAttackTo),
		orderStopAction(kActionStopOrder, kOrderStop),

		fieldCommanderAbility1Action(kActionFCAbility1, kMotionAbility1),
		fieldCommanderAbility2Action(kActionFCAbility2, kMotionAbility2),
		fieldCommanderAbility3Action(kActionFCAbility3, kMotionAbility3),

		groupUnits1(kActionGroupUnits1, kUnitGroup1),
		groupUnits2(kActionGroupUnits2, kUnitGroup2),
		groupUnits3(kActionGroupUnits3, kUnitGroup3),

		// Sounds - FW
		m_pHelpSound( nullptr ),
		m_pUISound( nullptr ),

		m_ulWorldID( -1 ),

		// player help messages
		m_ulNextCommAttackedMsg( 0 ),
		m_ulNextBaseAttackedMsg( 0 ),
		m_ulNextBuildAttackedMsg( 0 ),
		m_ulNextBuildMsg( BUILD_REMINDER_INTERVAL ),
		m_ulNextCommMsg( COMM_REMINDER_INTERVAL ),
		m_ulNextUnitsMsg( UNITS_REMINDER_INTERVAL ),
		m_ulNextNoBuildMsg( 0 ),
		m_bIsHelpActivte( true ),

		m_pConsoleAction( nullptr ),

		crosshair(64.0F, 64.0F, "texture/ui/field_mode/crosshair"),

		// popup screens
		m_objPlayTutorial( 512.0F, 512.0F, "Tutorial", "texture/ui/tutorial" ),
		m_objVictory( 512.0F, 512.0F, "Victory", "texture/ui/victory" ),
		m_objDefeat( 512.0F, 512.0F, "Defeat", "texture/ui/defeat" ),
		m_objCredits(512.0F, 512.0F, "Credits", "texture/ui/credits"),
		m_objControls(512.0F, 512.0F, "Controls", "texture/ui/controls")
{
	TheDisplayMgr->InstallDisplayEventHandler(&displayEventHandler);

	TheSoundMgr->RegisterSoundGroup(&effectsSoundGroup);
	TheSoundMgr->RegisterSoundGroup(&musicSoundGroup);
	TheSoundMgr->SetDefaultSoundGroup(&effectsSoundGroup);

	TheInputMgr->AddAction(&forwardAction);
	TheInputMgr->AddAction(&backwardAction);
	TheInputMgr->AddAction(&leftAction);
	TheInputMgr->AddAction(&rightAction);
	TheInputMgr->AddAction(&upAction);
	TheInputMgr->AddAction(&downAction);
	TheInputMgr->AddAction(&movementAction);
	TheInputMgr->AddAction(&horizontalAction);
	TheInputMgr->AddAction(&verticalAction);
	TheInputMgr->AddAction(&primaryFireAction);
	TheInputMgr->AddAction(&secondaryFireAction);
	TheInputMgr->AddAction(&cameraViewAction);
	TheInputMgr->AddAction(&scoreboardAction);
	TheInputMgr->AddAction(&chatAction);

	// Blood Tide Actions - CW
	TheInputMgr->AddAction(&navUpAction);
	TheInputMgr->AddAction(&navLeftAction);
	TheInputMgr->AddAction(&navDownAction);
	TheInputMgr->AddAction(&navRightAction);
	TheInputMgr->AddAction(&orderAttackToAction);
	TheInputMgr->AddAction(&orderStopAction);
	TheInputMgr->AddAction(&fieldCommanderAbility1Action);
	TheInputMgr->AddAction(&fieldCommanderAbility2Action);
	TheInputMgr->AddAction(&fieldCommanderAbility3Action);
	TheInputMgr->AddAction(&switchBTModesAction);
	TheInputMgr->AddAction(&showCursorAction);
	TheInputMgr->AddAction(&selectAllUnitsAction);
	TheInputMgr->AddAction(&sendAllUnitsToComAction);
	TheInputMgr->AddAction(&groupUnits1);
	TheInputMgr->AddAction(&groupUnits2);
	TheInputMgr->AddAction(&groupUnits3);

	prevEscapeProc = TheInputMgr->GetEscapeProc();
	prevEscapeData = TheInputMgr->GetEscapeData();
	TheInputMgr->SetEscapeProc(&EscapeProc, this);

	TheWorldMgr->SetWorldConstructor(&ConstructWorld);
	TheMessageMgr->SetPlayerConstructor(&ConstructPlayer);

	Property::InstallConstructor(&propertyConstructor);
	TheMessageMgr->SetSnapshotInterval(125);

	inputFlags = 0;
	multiplayerFlags = 0;

	TheEngine->InitVariable("looksens", "25", kVariablePermanent, &SensitivityProc, this);
	TheEngine->InitVariable("invertX", "0", kVariablePermanent, &InvertXProc, this);
	TheEngine->InitVariable("invertY", "0", kVariablePermanent, &InvertYProc, this);

	fovVariable = TheEngine->InitVariable("fov", "60", kVariablePermanent);
	fovVariable->SetTriggerProc(&FovProc, this);

	effectsVolumeVariable = TheEngine->InitVariable("volumeEffects", "100", kVariablePermanent, &EffectsVolumeProc, this);
	musicVolumeVariable = TheEngine->InitVariable("volumeMusic", "100", kVariablePermanent, &MusicVolumeProc, this);
	voiceVolumeVariable = TheEngine->InitVariable("volumeVoice", "100", kVariablePermanent, &VoiceVolumeProc, this);

	voiceReceiveVariable = TheEngine->InitVariable("voiceReceive", "0", kVariablePermanent);
	voiceReceiveVariable->SetTriggerProc(&VoiceReceiveProc, this);

	voiceSendVariable = TheEngine->InitVariable("voiceSend", "0", kVariablePermanent);
	voiceSendVariable->SetTriggerProc(&VoiceSendProc, this);

	// Grab the model with the stealth shader attached
	Model *model = Model::New( "model/units/stealth_shader" );
	if( model )
	{
		Geometry *geo = static_cast<Geometry *>( model->GetFirstSubnode() );
		if( geo && geo->GetNodeType() == kNodeGeometry )
			m_pStealthShader = geo->GetMaterialObject(0)->Clone();
	}

	delete model;
	//

	StringTableManager::New();

	UsePopupInterface::New();
	NamePopupInterface::New();
	MainWindow::Open();

	TheWorldMgr->LoadWorld( "world/menu" );

	firstLoad = TheEngine->InitVariable( "firstLoad", "0", kVariablePermanent );
	if( firstLoad && firstLoad->GetIntegerValue() == 0 ) {
		// This is the first time the game was loaded, inform the player of the tutorial.
		TheInterfaceMgr->AddElement( &m_objPlayTutorial );
		firstLoad->SetIntegerValue( 1 );
		TheEngine->AddVariable( firstLoad );
		TheEngine->WriteVariables();
	}

	// Choose name popup
	PlayerSettingsWindow::Open();
}

Game::~Game()
{
	ExitCurrentGame(true, true);
	ClearModelRegList();

	interfaceList.Purge();
	windowList.Purge();

	TheWorldMgr->SetWorldConstructor(nullptr);
	TheMessageMgr->SetPlayerConstructor(nullptr);

	TheInputMgr->SetEscapeProc(prevEscapeProc, prevEscapeData);

	if( m_pStealthShader )
		m_pStealthShader->Release();

	if( TheStringTableMgr ) delete TheStringTableMgr;
}

void Game::ClearModelRegList( void )
{
	int nLength = m_arrUnitModelReg.GetElementCount();
	for( int x = 0; x < nLength; x++ ) {
		delete m_arrUnitModelReg[x];
		m_arrUnitModelReg[x] = nullptr;
	}
}

World *Game::ConstructWorld(const char *name, void *data)
{
	return (new GameWorld(name));
}

Player *Game::ConstructPlayer(PlayerKey key, void *data)
{
	return (new GamePlayer(key));
}

Property *Game::ConstructProperty(Unpacker& data, unsigned long unpackFlags)
{
	PropertyType type = data.GetType();
	if (type - kPropertyGameMaterial < kGameMaterialCount) return (new MaterialProperty(type));

	return (nullptr);
}

void Game::HandleDisplayEvent(EventType eventType, long param, void *data)
{
	if (eventType == kEventDisplayChange)
	{
		// The display resolution has changed, so we need to reposition
		// the various interface items on the screen.

		TheUsePopupInterface->UpdateDisplayPosition();
		TheNamePopupInterface->UpdateDisplayPosition();
		if (TheDisplayInterface) TheDisplayInterface->UpdateDisplayPosition();
		if (TheBloodTideLobby) TheBloodTideLobby->UpdateDisplayPosition();
		if (TheBTHUD) TheBTHUD->UpdateDisplayPosition();
		if( TheMainWindow )
		{
			delete TheMainWindow;
			MainWindow::Open();
		}
	}
}

void Game::EscapeProc(void *data)
{
	MainWindow::Open();
}

void Game::SensitivityProc(Variable *variable, void *data)
{
	Game *game = static_cast<Game *>(data);

	long sensitivity = Min(Max(variable->GetIntegerValue(), 1), 100);
	game->lookSensitivity = sensitivity;

	unsigned long flags = game->inputFlags;
	float m = (float) sensitivity * -2.0e-4F;
	game->lookMultiplierX = (flags & kInputXInverted) ? -m : m;
	game->lookMultiplierY = (flags & kInputYInverted) ? -m : m;
}

void Game::InvertXProc(Variable *variable, void *data)
{
	Game *game = static_cast<Game *>(data);

	unsigned long flags = game->inputFlags;
	float m = Fabs(game->lookMultiplierX);

	if (variable->GetIntegerValue() != 0)
	{
		flags |= kInputXInverted;
	}
	else
	{
		flags &= ~kInputXInverted;
		m = -m;
	}

	game->inputFlags = flags;
	game->lookMultiplierX = m;
}

void Game::InvertYProc(Variable *variable, void *data)
{
	Game *game = static_cast<Game *>(data);

	unsigned long flags = game->inputFlags;
	float m = Fabs(game->lookMultiplierY);

	if (variable->GetIntegerValue() != 0)
	{
		flags |= kInputYInverted;
	}
	else
	{
		flags &= ~kInputYInverted;
		m = -m;
	}

	game->inputFlags = flags;
	game->lookMultiplierY = m;
}

void Game::FovProc(Variable *variable, void *data)
{
	GameWorld *world = static_cast<GameWorld *>(TheWorldMgr->GetWorld());
	if (world) world->SetFocalLength(TheGame->GetCameraFocalLength());
}

void Game::EffectsVolumeProc(Variable *variable, void *data)
{
	Game *game = static_cast<Game *>(data);
	game->effectsSoundGroup.SetVolume((float) variable->GetIntegerValue() * 0.01F);
}

void Game::MusicVolumeProc(Variable *variable, void *data)
{
	Game *game = static_cast<Game *>(data);
	game->musicSoundGroup.SetVolume((float) variable->GetIntegerValue() * 0.01F);
}

void Game::VoiceVolumeProc(Variable *variable, void *data)
{
	Game *game = static_cast<Game *>(data);
	game->voiceSoundGroup.SetVolume((float) variable->GetIntegerValue() * 0.01F);
}

void Game::VoiceReceiveProc(Variable *variable, void *data)
{
	// The "Receive voice chat" variable has changed. If we're in a multiplayer game,
	// then tell the server either to start or stop sending chat data.

	if (TheMessageMgr->Multiplayer())
	{
		MessageType type = (variable->GetIntegerValue() != 0) ? kMessageClientVoiceReceiveStart : kMessageClientVoiceReceiveStop;
		TheMessageMgr->SendMessage(kPlayerServer, ClientMessage(type));
	}
}

void Game::VoiceSendProc(Variable *variable, void *data)
{
	// The "Send voice chat" variable has changed. If we're in a multiplayer game,
	// then tell the Audio Capture Manager either to start or stop capturing voice.
	// (Redundant calls to these functions have no effect.)

	if (TheMessageMgr->Multiplayer())
	{
		if (variable->GetIntegerValue() != 0) TheAudioCaptureMgr->StartAudioCapture();
		else TheAudioCaptureMgr->StopAudioCapture();
	}
}

float Game::GetCameraFocalLength(void) const
{
	long angle = Min(Max(fovVariable->GetIntegerValue(), 45), 90);
	return (1.0F / Tan((float) angle * (K::radians * 0.5F)));
}

void Game::ClearInterface(void)
{
	delete TheMainWindow;
	TheInterfaceMgr->GetMenuBar()->Hide();
	TheConsoleWindow->Close();

	lookSpeedX = 0.0F;
	lookSpeedY = 0.0F;
}

WorldResult Game::StartSinglePlayerGame(const char *name)
{
	ClearInterface();
	ExitCurrentGame(false);

	WorldResult result = TheWorldMgr->LoadWorld(name);
	if (result == kWorldOkay)
	{
		TheMessageMgr->BeginSinglePlayerGame();
		//SpawnPlayer(TheMessageMgr->GetLocalPlayer());
	}

	return (result);
}



void Game::HostMultiplayerGame(const char *name, unsigned long ulWorldID,  unsigned long flags)
{
	ClearInterface();
	ExitCurrentGame(false);

	multiplayerFlags = flags;

	TheNetworkMgr->SetProtocol(kGameProtocol);
	TheNetworkMgr->SetPortNumber(kGamePort);
	TheNetworkMgr->SetBroadcastPortNumber(kGamePort);

	TheMessageMgr->BeginMultiplayerGame(true);

	GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetLocalPlayer());
	unsigned long playerFlags = (flags & kMultiplayerDedicated) ? kPlayerInactive : 0;
	if (TheEngine->GetVariable("voiceReceive")->GetIntegerValue() != 0) playerFlags |= kPlayerReceiveVoiceChat;
	player->SetPlayerFlags(playerFlags);

	if (TheEngine->GetVariable("voiceSend")->GetIntegerValue() != 0) TheAudioCaptureMgr->StartAudioCapture();

	StartBloodTideLobby(ulWorldID);

	TheGame->SetWorldID( ulWorldID );
}

void Game::JoinMultiplayerGame(const char *name, unsigned long ulWorldID, unsigned long flags)
{
	ClearInterface();

	multiplayerFlags = 0;

	GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetPlayer(kPlayerServer));
	if (flags & kMultiplayerDedicated) player->SetPlayerFlags(player->GetPlayerFlags() | kPlayerInactive);

	if (TheEngine->GetVariable("voiceSend")->GetIntegerValue() != 0) TheAudioCaptureMgr->StartAudioCapture();

	StartBloodTideLobby(ulWorldID);

	TheGame->SetWorldID( ulWorldID );
}

void Game::StartBloodTideLobby(unsigned long ulWorldID)
{
	delete TheMainWindow;
	BloodTideLobby::New(ulWorldID);
	ChatWindow::New();
	ChatInterface::New();

	ChangeKeyBindings( COMMANDER_BINDING );

	TheInputMgr->SetInputMode( kInputKeyboardActive );
	TheInterfaceMgr->SetInputManagementMode( kInputManagementCommandMode );
	TheInterfaceMgr->ShowCursor();

	TheMessageMgr->SendMessage(kPlayerServer,
			LobbyModifySlotMessage(kOperationJoin, -1, -1));

	// Disable the console action "~"
	m_pConsoleAction = TheInputMgr->FindAction(kActionConsole);
	TheInputMgr->RemoveAction( m_pConsoleAction );
}

void Game::ExitCurrentGame(bool unload, bool end)
{
	TheAudioCaptureMgr->StopAudioCapture();
	TheMessageMgr->EndGame();
	multiplayerFlags = 0;

	if (unload) TheWorldMgr->UnloadWorld();

	TheUsePopupInterface->InstantHide();
	TheNamePopupInterface->InstantHide();

	delete TheScoreboardInterface;
	delete TheDisplayInterface;

	if( TheBloodTideLobby ) delete TheBloodTideLobby;

	if( TheBTHUD ) delete TheBTHUD;
	if( TheInfoPaneMgr ) delete TheInfoPaneMgr;
	if( TheUnitIconMgr ) delete TheUnitIconMgr;
	if( TheCommanderSelecter ) delete TheCommanderSelecter;
	if( TheBuildingsMenu ) delete TheBuildingsMenu;
	if( TheLoadingTip ) delete TheLoadingTip;
	if( TheMessageInterface ) delete TheMessageInterface;
	if( TheChatInterface ) delete TheChatInterface;
	if( TheChatWindow ) delete TheChatWindow;

	if( TheScriptMgr ) delete TheScriptMgr;

	TheBTCursor->ChangeState(nullptr, Point() );
	TheInterfaceMgr->SetInputManagementMode( kInputManagementAutomatic );
	TheInputMgr->SetInputMode( kInputKeyboard );

	if( !end ) TheWorldMgr->LoadWorld( "world/menu" );

	// Re-enable the console action "~"
	if( m_pConsoleAction )
	{
		TheInputMgr->AddAction( m_pConsoleAction );
		m_pConsoleAction = nullptr;
	}
}

EngineResult Game::LoadWorld(const char *name)
{
	return (StartSinglePlayerGame(name));
}

void Game::UnloadWorld(void)
{
	ExitCurrentGame();
}

void Game::InitializeBloodTideSession()
{
	GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetLocalPlayer());
	long playerTeam = player->GetPlayerTeam();

	GameWorld *gameWorld = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	RTSCamera& commanderCamera = gameWorld->GetCommanderCamera();

	// Check to see if this world has a script associated with it.
	const char *script_file = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(TheGame->GetWorldID(), 'INFO', 'SCRP'));
	if( !Text::CompareTextCaseless(script_file, "<missing>") ) {
		StringTable *script = new StringTable( script_file );
		if( script ) {
			if( TheScriptMgr ) delete TheScriptMgr;
			ScriptManager::New( script );
		}
	}

	// Setup victory condition.
	Marker *victLoc = gameWorld->GetVictoryLocator();
	if( victLoc ) {

		BasesDestroyed_VictoryController *vc = static_cast<BasesDestroyed_VictoryController*>( victLoc->GetController() );
		if( vc ) {
			vc->Setup();
		}
	}

	// Initialize cameras
	Point3D position(0.0F,0.0F,0.0F); // default values
	Marker *marker = gameWorld->GetTeamSpawnLocator(playerTeam);

	if( marker ) position = marker->GetNodePosition();
	commanderCamera.Initialize(position);

	gameWorld->GetPlayerCamera()->SetWorld( gameWorld );

	gameWorld->SetFocalLength( 1.0F );
	//

	// Tell all of the bases to start spawning units
	if( TheMessageMgr->Server() ) {
		Array<long> *baseCache = TheGameCacheMgr->GetGameCache(kCacheBases, 1);
		if( baseCache ) {

			for( long i = 0; i < baseCache->GetElementCount(); i++ ) {

				BaseController *cont = static_cast<BaseController *>( TheWorldMgr->GetWorld()->GetController( (*baseCache)[i] ) );

				if( cont ) {
					cont->StartSpawning();
					cont->SpawnUnits();
				}
			}
		}


		baseCache = TheGameCacheMgr->GetGameCache(kCacheBases, 2);
		if( baseCache ) {

			for( long i = 0; i < baseCache->GetElementCount(); i++ ) {

				BaseController *cont = static_cast<BaseController *>( TheWorldMgr->GetWorld()->GetController( (*baseCache)[i] ) );

				if( cont ) {
					cont->StartSpawning();
					cont->SpawnUnits();
				}
			}
		}
	}
	//

	// Create the Blood Tide HUD using the images from the String table for this team
	const StringTable *worlds = TheStringTableMgr->GetStringTable( kStringTableWorlds );

	unsigned long teamID;
	if( playerTeam == 1 )
		teamID = 'TEA1';
	else
		teamID = 'TEA2';

	const char *paneChat = worlds->GetString( StringID( m_ulWorldID, 'INFO', teamID, 'CHAT') );
	const char *paneInfo = worlds->GetString( StringID( m_ulWorldID, 'INFO', teamID, 'INFO' ) );
	const char *paneMap = worlds->GetString( StringID( m_ulWorldID, 'INFO', teamID, 'MAP' ) );

	BTHUD::New( paneChat, paneInfo, paneMap );

	InfoPaneMgr::New();
	UnitIconMgr::New();
	MessageInterface::New();

	// clear chat
	if( TheChatInterface ) delete TheChatInterface;
	ChatInterface::New();

	if( TheLoadingTip ) {
		delete TheLoadingTip;
	}

	Array<long> *baseIDs = TheGameCacheMgr->GetGameCache(kCacheBases, playerTeam);
	if( baseIDs ) {
		CommanderSelecter::New(*baseIDs, 50.0F);
	}
	//

	// Start music based on player faction.
	gameWorld->StartMusic();

	// Player starts a game session as a commander
	ToCommandMode();

	// Controls-tip
	TheGame->HelpMessage( 'init' );
}

void Game::ToCommandMode()
{
	GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetLocalPlayer());
	CommanderController *controller = player->GetPlayerController();
	GameWorld *gameWorld = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	RTSCamera& commanderCamera = gameWorld->GetCommanderCamera();

	// Re-position the camera above the player's Commander if the player has one
	if( controller ) {

		Node *node = controller->GetTargetNode();

		Point3D cameraPos = node->GetSuperNode()->GetInverseWorldTransform() * node->GetNodePosition();
		cameraPos.y = cameraPos.y - commanderCamera.GetNodePosition().z * (1 / tan(Fabs(commanderCamera.GetCameraAltitude())));
		cameraPos.z = commanderCamera.GetNodePosition().z;

		commanderCamera.SetNodePosition( cameraPos );

		// turn off crosshair assistance
		controller->DisableCrosshairAssistance();
	}

	gameWorld->SetCamera( &commanderCamera );

	// Hide the crosshair
	Element *crosshair = TheGame->GetCrosshairs();

	if( crosshair )
	{
		TheInterfaceMgr->RemoveElement( crosshair );
	}
	//

	// Initialize input for Command Mode
	ChangeKeyBindings( COMMANDER_BINDING );

	TheInputMgr->SetInputMode( kInputKeyboardActive ); // mouse will be handled by the interface manager
	TheInterfaceMgr->SetInputManagementMode( kInputManagementCommandMode ); // to prevent the interface manager from hiding the cursor when windows are closed

	TheBTCursor->ChangeState(TheDefaultState, Point(0.0F,0.0F) );
	//

	Pane& infoPane = TheBTHUD->GetPaneInfo();
	infoPane.PurgePane();


	TheBTCursor->SelectGroup( kUnitFieldMode );

	if( controller == nullptr ) TheCommanderSelecter->Show();

	gameWorld->SetInCommandMode( true );
	gameWorld->SetInFieldMode( false );

	// setup next field mode reminder
	TheGame->SetNextCommRemTime( TheTimeMgr->GetAbsoluteTime() + COMM_REMINDER_INTERVAL );
}

void Game::ToFieldMode( bool instant )
{
	GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetLocalPlayer());
	if( player == nullptr )
		return;

	GameWorld *gameWorld = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( gameWorld == nullptr )
		return;

	CommanderController *controller = static_cast<CommanderController*>( player->GetPlayerController() );
	if( controller == nullptr )
		return;

	// Initialize input for Field Mode
	ChangeKeyBindings( FIELDCOMMANDER_BINDING );

	TheInterfaceMgr->HideCursor();

	TheInputMgr->SetInputMode( kInputInactive );
	TheBTCursor->ChangeState(nullptr, Point() );
	//

	// Set up the Commander Info Pane
	TheBTCursor->GroupSelectedUnits( kUnitFieldMode );
	TheBTCursor->PurgeSelectedUnits();

	CommanderInfoPane *pane = static_cast<CommanderInfoPane *>( &TheInfoPaneMgr->GetInfoPane(kInfoPaneCommander) );
	pane->SetCommander( controller );
	TheInfoPaneMgr->ShowInfoPane( kInfoPaneCommander );
	//

	gameWorld->SetInCommandMode( false );

	// Set up the Field Mode Camera
	Model *model = static_cast<Model *>( controller->GetTargetNode() );
	ModelCamera* chaseCam = gameWorld->GetPlayerCamera();

	if( model && chaseCam )
	{
		if( instant == true )
		{
			gameWorld->SetCamera( chaseCam );
			chaseCam->SetTargetModel( model );

			FinishFieldModeTransition();
		}
		else
		{
			// Setup Camera Blend
			chaseCam->SetTargetModel( model );
			chaseCam->Move(); // position the camera

			RTSCamera& cam = gameWorld->GetCommanderCamera();
			gameWorld->GetCameraBlender().Initialize( gameWorld->GetCommanderCamera(), *chaseCam, 3000.0F, Game::FinishFieldModeTransition );
		}
	}
}

void Game::FinishFieldModeTransition( void )
{
	GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetLocalPlayer());
	GameWorld *gameWorld = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );

	if( player == nullptr || gameWorld == nullptr ) {

		TheGame->ToCommandMode();
		return;
	}

	CommanderController *controller = static_cast<CommanderController*>( player->GetPlayerController() );
	if( controller == nullptr || controller->GetEntityFlags() & kEntityDead ) {

		TheGame->ToCommandMode();
		return;
	}


	TheInterfaceMgr->SetInputManagementMode(kInputManagementAutomatic);
	TheInterfaceMgr->SetInputMode();

	gameWorld->SetInFieldMode( true );

	// turn on crosshair assistance
	controller->EnableCrosshairAssistance();
}

void Game::AwardExperience( BTEntityController &objEntity )
{
	long opposingTeam = objEntity.GetTeam() == 1 ? 2 : 1;

	Array<long> *commanders = TheGameCacheMgr->GetGameCache( kCacheCommanders, opposingTeam );
	if( commanders ) {

		long size = commanders->GetElementCount();
		for( long x = 0; x < size; x++ ) {

			CommanderController *controller = static_cast<CommanderController *>( TheWorldMgr->GetWorld()->GetController( (*commanders)[x] ));

			controller->AwardExperience( objEntity );
		}
	}
}

void Game::ChangeKeyBindings(const char *strInputFile)
{

	// Store the current mode of the input manager so that it can be reverted to after
	// key bindings are changed.
	InputMode previousMode = TheInputMgr->GetInputMode();

	// Disable Input while transitioning between key bindings
	TheInputMgr->SetInputMode(kInputInactive);

	// Reset all actions for the keyboard amd mouse
	InputDevice *device = TheInputMgr->FindDevice(kInputKeyboard);

	if(device)
		device->ResetActions();

	device = TheInputMgr->FindDevice(kInputMouse);

	if(device)
		device->ResetActions();

	// Now that no actions are running, clear all input controls of their associated actions
	TheInputMgr->ClearAllControlActions();

	// Remap input controls for <inputfile> bindings
	TheEngine->ExecuteFile(strInputFile);

	// Finally, revert the input mode to its previous mode
	TheInputMgr->SetInputMode(previousMode);
}

// Check to see if a collider has collided with some other collider.
void Game::DetectEntityCollision( BTEntityController *pController, BTCollisionData& rstData, unsigned long ulCacheFlags, float fHeading, float fLookAheadDist, Array<long>& rIgnoreConts, bool bIs3D )
{
	if( !TheGameCacheMgr ) {
		return;
	}

	float fClosestSoFar = K::infinity;

	if( ulCacheFlags & kCacheUnits ) {
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheUnits, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheUnits, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheCommanders ) {
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheCommanders, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheCommanders, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheBases ) {
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheBases, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheBases, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheBuildings ) {
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheBuildings, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntityCollision( pController, rstData, fHeading, fLookAheadDist, TheGameCacheMgr->GetGameCache(kCacheBuildings, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}
}

// Check to see if a collider has collided with some other collider of a specific entity type.
void Game::DetectEntityCollision( BTEntityController *pController, BTCollisionData& rstData, float fHeading, float fLookAheadDist, Array<long> *arrColliders, float& fClosestSoFar, Array<long>& rIgnoreConts, bool bIs3D )
{
	if( !arrColliders ) {
		return;
	}

	// Sanitize heading
	float heading = fHeading;
	NormalizeAngle( heading );


	// Loop through colliders to test for collision
	int nLength = arrColliders->GetElementCount();
	for( int x = 0; x < nLength; x++ ) {

		long cIndex = (*arrColliders)[x];
		BTEntityController *cont = static_cast<BTEntityController*>( TheWorldMgr->GetWorld()->GetController(cIndex) );
		if( !cont ) {
			continue;
		}

		// Some controllers are ignored.
		if( cont == pController ) {
			continue;
		}

		int f = rIgnoreConts.FindElement( cont->GetControllerIndex() );
		if( f > -1 ) {
			continue;
		}

		if( cont->GetEntityFlags() & kEntityInvisible ) {
			continue;
		}

		if( cont->GetEntityFlags() & kEntityDead  ) {
			continue;
		}

		if( !pController->GetCollider() ) {
			continue;
		}

		BTCollisionData data;
		if( fLookAheadDist > 0.0F ) {
			pController->GetCollider()->DetectCollisionAhead( cont->GetCollider(), data, fLookAheadDist, heading, bIs3D );
		}
		else {
			pController->GetCollider()->DetectCollision( cont->GetCollider(), data, bIs3D );
		}

		if( data.m_State != kCollisionStateNone ) {
			if( data.m_Distance < fClosestSoFar ) {
				fClosestSoFar = data.m_Distance;
				rstData.m_pController = data.m_pController;
				rstData.m_State = data.m_State;
				rstData.m_Distance = data.m_Distance;
				rstData.m_v3dNormal = data.m_v3dNormal;
			}
		}
	}
}

// Checks to see if there is a unit at a given location (excluding the provided unit).
void Game::DetectEntity( Point3D& p3dPosition, BTCollisionData& rstData, unsigned long ulCacheFlags, float fSafeDist, Array<long>& rIgnoreConts, bool bIs3D )
{
	if( !TheGameCacheMgr ) {
		return;
	}

	float fClosestSoFar = K::infinity;

	if( ulCacheFlags & kCacheUnits ) {
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheUnits, 1), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheUnits, 2), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheCommanders ) {
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheCommanders, 1), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheCommanders, 2), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheBases ) {
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheBases, 1), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheBases, 2), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheBuildings ) {
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheBuildings, 1), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
		DetectEntity( p3dPosition, rstData, TheGameCacheMgr->GetGameCache(kCacheBuildings, 2), fClosestSoFar, fSafeDist, rIgnoreConts, bIs3D );
	}


}

void Game::DetectEntity( Point3D& p3dPosition, BTCollisionData& rstData, Array<long> *arrColliders, float& fClosestSoFar, float fSafeDist, Array<long>& rIgnoreConts, bool bIs3D )
{
	if( !arrColliders ) {
		return;
	}

	// Loop through colliders to test for collision
	int nLength = arrColliders->GetElementCount();
	for( int x = 0; x < nLength; x++ ) {

		long cIndex = (*arrColliders)[x];
		BTEntityController *cont = static_cast<BTEntityController*>( TheWorldMgr->GetWorld()->GetController(cIndex) );
		if( !cont ) {
			continue;
		}

		// Some controllers are ignored.
		int f = rIgnoreConts.FindElement( cont->GetControllerIndex() );
		if( f > -1 ) {
			continue;
		}

		if( cont->GetEntityFlags() & kEntityDead  ) {
			continue;
		}

		if( !cont->GetCollider() ) {
			continue;
		}

		BTCollisionData data;
		cont->GetCollider()->DetectCollision( p3dPosition, data, fSafeDist, bIs3D );
		if( data.m_State != kCollisionStateNone ) {
			if( data.m_Distance < fClosestSoFar ) {
				fClosestSoFar = data.m_Distance;
				rstData.m_pController = data.m_pController;
				rstData.m_State = data.m_State;
				rstData.m_Distance = data.m_Distance;
				rstData.m_v3dNormal = data.m_v3dNormal;
			}
		}
	}
}

void Game::DetectEntity( Ray& cameraRay, BTCollisionData& rstData, unsigned long ulCacheFlags, Array<long>& rIgnoreConts, bool bIs3D )
{
	if( !TheGameCacheMgr ) {
		return;
	}

	float fClosestSoFar = K::infinity;

	if( ulCacheFlags & kCacheUnits ) {
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheUnits, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheUnits, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheCommanders ) {
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheCommanders, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheCommanders, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheBases ) {
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheBases, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheBases, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCachePlots ) {
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCachePlots, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCachePlots, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}

	if( ulCacheFlags & kCacheBuildings ) {
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheBuildings, 1), fClosestSoFar, rIgnoreConts, bIs3D );
		DetectEntity( cameraRay, rstData, TheGameCacheMgr->GetGameCache(kCacheBuildings, 2), fClosestSoFar, rIgnoreConts, bIs3D );
	}
}

void Game::DetectEntity( Ray& cameraRay, BTCollisionData& rstData, Array<long> *arrColliders, float& fClosestSoFar, Array<long>& rIgnoreConts, bool bIs3D )
{
	if( !arrColliders ) {
		return;
	}

	// Loop through colliders to test for intersection with cameraRay
	int nLength = arrColliders->GetElementCount();
	for( int x = 0; x < nLength; x++ ) {

		long cIndex = (*arrColliders)[x];
		BTEntityController *cont = static_cast<BTEntityController*>( TheWorldMgr->GetWorld()->GetController(cIndex) );
		if( !cont ) {
			continue;
		}

		// Some controllers are ignored.
		int f = rIgnoreConts.FindElement( cont->GetControllerIndex() );
		if( f > -1 ) {
			continue;
		}

		if( cont->GetEntityFlags() & kEntityDead  ) {
			continue;
		}

		if( !cont->GetCollider() ) {
			continue;
		}

		// Use the cameraRay and entity's Z to calculate the magnitude,
		// then extend the ray out that magnitude to get an end point to detect a collision
		Point3D pos = cont->GetPosition();

		float magnitude = ( pos.z - cameraRay.origin.z ) / ( cameraRay.direction.z );
		pos = cameraRay.origin + (cameraRay.direction * magnitude);
		//

		BTCollisionData data;
		float safeDist = 1.0F;

		cont->GetCollider()->DetectCollision( pos, data, safeDist, bIs3D );
		if( data.m_State != kCollisionStateNone ) {
			if( data.m_Distance < fClosestSoFar ) {
				fClosestSoFar = data.m_Distance;
				rstData.m_pController = data.m_pController;
				rstData.m_State = data.m_State;
				rstData.m_Distance = data.m_Distance;
				rstData.m_v3dNormal = data.m_v3dNormal;
			}
		}
	}
}

void Game::DetectEntity( Point3D& p3dStart, Point3D& p3dEnd, float fRadius, BTCollisionData& rstData, Array<long> *arrColliders, float& fClosestSoFar, Array<long>& rIgnoreConts )
{
	if( !arrColliders ) {
		return;
	}

	// Loop through colliders to test for intersection with cameraRay
	int nLength = arrColliders->GetElementCount();
	for( int x = 0; x < nLength; x++ ) {

		long cIndex = (*arrColliders)[x];
		BTEntityController *cont = static_cast<BTEntityController*>( TheWorldMgr->GetWorld()->GetController(cIndex) );
		if( !cont ) {
			continue;
		}

		// Some controllers are ignored.
		int f = rIgnoreConts.FindElement( cont->GetControllerIndex() );
		if( f > -1 ) {
			continue;
		}

		if( cont->GetEntityFlags() & kEntityDead  ) {
			continue;
		}

		if( !cont->GetCollider() ) {
			continue;
		}

		BTCylinderCollider *collider = static_cast<BTCylinderCollider *>( cont->GetCollider() );
		if( collider )
		{
			Vector3D p0 = p3dStart - cont->GetPosition();
			Vector3D dp = p3dEnd - p3dStart;

			//float r = collider->GetRadius() + fRadius;

			//float a = dp * dp;
			//float b = p0 * dp;
			//float c = p0 * p0 - r * r;
			//if (b * b - a * c > 0.0F)
			//{
			//	// TODO: don't have exact position of collision
			//	float distance = Calculate2DDistance( p3dStart, collider->GetPosition() );

			//	if( distance < fClosestSoFar )
			//	{
			//		fClosestSoFar = distance;

			//		rstData.m_pController = cont;
			//		rstData.m_State = kCollisionStateCollider;
			//		rstData.m_Distance = distance;
			//	}
			//}

			Vector3D toCol = collider->GetPosition() - p3dStart;
			if( Magnitude(toCol) > Magnitude(dp) ) {
				continue;
			}

			dp.Normalize();
			dp *= Magnitude(toCol) - (collider->GetRadius()/2.0F);

			Point3D test = p3dStart + dp;
			BTCollisionData dat;
			if( collider->DetectCollision(test, dat, fRadius, true) ) {
				if( dat.m_Distance < fClosestSoFar ) {
					fClosestSoFar = dat.m_Distance;

					rstData.m_pController = cont;
					rstData.m_State = kCollisionStateCollider;
					rstData.m_Distance = dat.m_Distance;
					rstData.m_v3dNormal = (dp.Normalize() * -1.0F);
				}
			}
		}
	}
}

void Game::DetectEntities( Point3D& p3dPosition, float fRadius, Array<long>& arrData, Array<long> *arrColliders, Array<long>& rIgnoreConts, bool bIs3D )
{
	if( !arrColliders ) {
		return;
	}

	// Loop through colliders to test for collision
	int nLength = arrColliders->GetElementCount();
	for( int x = 0; x < nLength; x++ ) {

		long cIndex = (*arrColliders)[x];
		BTEntityController *cont = static_cast<BTEntityController*>( TheWorldMgr->GetWorld()->GetController(cIndex) );
		if( !cont ) {
			continue;
		}

		// Some controllers are ignored.
		int f = rIgnoreConts.FindElement( cont->GetControllerIndex() );
		if( f > -1 ) {
			continue;
		}

		if( !cont->GetCollider() ) {
			continue;
		}

		BTCollisionData data;
		cont->GetCollider()->DetectCollision( p3dPosition, data, fRadius, bIs3D );
		if( data.m_State != kCollisionStateNone ) {

			arrData.AddElement( cIndex );
		}
	}
}

// There can only be one UI sound at a time.
void Game::PlayUISound( const char *pSound, float fVol, long lLoopAmt )
{
	StopUISound();

	m_pUISound = new Sound;
	m_pUISound->SetSoundGroup( &effectsSoundGroup );
	m_pUISound->SetLoopCount( lLoopAmt );
	m_pUISound->SetSoundProperty( kSoundVolume, fVol );

	WaveStreamer *streamer = new WaveStreamer;
	streamer->AddComponent( pSound );
	m_pUISound->Stream( streamer );

	m_pUISound->Play();
}

void Game::StopUISound( void )
{
	if( m_pUISound ) {
		m_pUISound->Stop();
		m_pUISound->Release();
		m_pUISound = nullptr;
	}
}

// There can only be one Help sound at a time.
void Game::PlayHelpSound( const char *pSound, float fVol, long lLoopAmt )
{
	StopHelpSound();

	m_pHelpSound = new Sound;
	m_pHelpSound->SetSoundGroup( &effectsSoundGroup );
	m_pHelpSound->SetLoopCount( lLoopAmt );
	m_pHelpSound->SetSoundProperty( kSoundVolume, fVol );

	WaveStreamer *streamer = new WaveStreamer;
	streamer->AddComponent( pSound );
	m_pHelpSound->Stream( streamer );

	m_pHelpSound->Play();
}

void Game::StopHelpSound( void )
{
	if( m_pHelpSound ) {
		m_pHelpSound->Stop();
		m_pHelpSound->Release();
		m_pHelpSound = nullptr;
	}
}

bool Game::IsReadyForBuilding(long team)
{
	Array<long> *baseCache = TheGameCacheMgr->GetGameCache(kCacheBases, team);

	int length = baseCache->GetElementCount();

	for(int x = 0; x < length; x++)
	{
		long cIndex = (*baseCache)[x];
		BaseController* bc = static_cast<BaseController*>(TheWorldMgr->GetWorld()->GetController(cIndex));

		if(!bc){
			continue;
		}

		if(bc->IsReadyForBuilding() == false)
		{
			return false;
		}
	}

	return true;
}

/**
 * Print a help message to the message interface.
 * If there is an associated sound with the message, play it.
 * Note: the message is obtained from the string table Messages.txt
 * @author - Frank Williams (demalus@gmail.com)
 */
void Game::HelpMessage( unsigned long ulMsgID )
{
	if( !TheMessageInterface ) {
		return;
	}

	const StringTable *msg_table = TheStringTableMgr->GetStringTable( kStringTableMessages );
	if( !msg_table ) {
		return;
	}

	String<kMaxChatMessageLength> string( msg_table->GetString(StringID(ulMsgID, 'text')) );
	TheMessageInterface->AddText( string );

	const char *sound = msg_table->GetString( StringID(ulMsgID, 'asnd') );
	if( !Text::CompareTextCaseless( sound, "<missing>") ) {
		PlayHelpSound( sound, 1.0F, 1 );
	}
}

/**
 * Print a help message to the message interface.
 * If there is an associated sound with the message, play it.
 * @author - Frank Williams (demalus@gmail.com)
 */
void Game::HelpMessage( const char *msg, const char *snd )
{
	if( !TheMessageInterface ) {
		return;
	}

	TheMessageInterface->AddText( msg );

	if( !Text::CompareTextCaseless( snd, "<missing>") ) {
		PlayHelpSound( snd, 1.0F, 1 );
	}
}

// Check to see if the game is over.
void Game::CheckVictoryConditions( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	Marker *victLoc = world->GetVictoryLocator();
	if( !victLoc ) {
		return;
	}

	BasesDestroyed_VictoryController *vc = static_cast<BasesDestroyed_VictoryController*>( victLoc->GetController() );
	if( !vc ) {
		return;
	}

	long winner = vc->CheckVictoryConditions();
	if( winner > 0 ) {
		TheMessageMgr->SendMessageAll( GameOverMessage(winner) );
	}
}

MaterialObject *const *Game::GetStealthShader( void )
{
	if( m_pStealthShader == nullptr )
		return (nullptr);

	m_pStealthShader->Retain();
	return (&m_pStealthShader);
}

// Display a message that something is under attack.
void Game::UnderAttackMessage( long team, ControllerType type )
{
	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( player == nullptr ) {
		return;
	}

	if( team != player->GetPlayerTeam() ) {
		return;
	}


	unsigned long time = TheTimeMgr->GetAbsoluteTime();

	switch( type ) {
		case kControllerCommander:
			if( TheGame->IsHelpActive() && time >= TheGame->GetNextCommAttackTime() ) {
				TheGame->HelpMessage('coat');
				TheGame->SetNextCommAttackTime( time + COMMANDER_ATTACK_INTERVAL );
			}
			break;
		case kControllerBase:
			if( TheGame->IsHelpActive() && time >= TheGame->GetNextBaseAttackTime() ) {
				TheGame->HelpMessage('baat');
				TheGame->SetNextBaseAttackTime( time + BASE_ATTACK_INTERVAL );
			}
			break;
		case kControllerBuilding:
			if( TheGame->IsHelpActive() && time >= TheGame->GetNextBuildAttackTime() ) {
				TheGame->HelpMessage('buat');
				TheGame->SetNextBuildAttackTime( time + BUILDING_ATTACK_INTERVAL );
			}
			break;
		default:
			break;
	}
}

