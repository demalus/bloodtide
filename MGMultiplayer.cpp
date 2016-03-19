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


#include "MGMultiplayer.h"
#include "BTCommander.h"
#include "MGGame.h"
#include "BTCursor.h"
#include "BTLobby.h"
#include "StatsProtocol.h"
#include "BTCommanderUI.h"
#include "BTInfoPane.h"
#include "BTUnit.h"
#include "BTEffects.h"
#include "C4ExtrasBase.h"
#include "C4FlashController.h"

using namespace C4;

void Game::HandleConnectionEvent(ConnectionEvent event, const NetworkAddress& address, const void *param)
{
	//TODO: Need to open MainWindow if connections fail... - CW
	switch (event)
	{
		case kConnectionQueryReceived:
		{
			World *world = TheWorldMgr->GetWorld();
			if (world)
			{
				const char *gameName = TheEngine->GetVariable("gameName")->GetValue();
				const char *worldName = world->GetWorldName();

				ServerInfoMessage message(TheMessageMgr->GetPlayerCount(), TheMessageMgr->GetMaxPlayerCount(), gameName, worldName);
				TheMessageMgr->SendConnectionlessMessage(address, message);
			}

			break;
		}

		case kConnectionAttemptFailed:

			// The server rejected our connection.

			if (TheConnectWindow)
			{
				unsigned long id = 'TIME';
				unsigned long reason = *static_cast<const unsigned long *>(param);
				if (reason == kNetworkFailWrongProtocol) id = 'PROT';
				else if (reason == kNetworkFailNotServer) id = 'NSRV';
				else if (reason == kNetworkFailServerFull) id = 'FULL';

				TheConnectWindow->SetFailMessage(stringTable.GetString(StringID('MULT', 'FAIL', id)));
			}

			break;

		case kConnectionServerAccepted:

			// The server accepted our connection.

			if (TheConnectWindow) TheConnectWindow->SetAcceptMessage(stringTable.GetString(StringID('MULT', 'ACPT')));

			break;

		case kConnectionServerClosed:

			// The server was shut down.

			ExitCurrentGame();
			MainWindow::Open();
			break;

		case kConnectionServerTimedOut:

			// The server has stopped responding.

			ExitCurrentGame();
			MainWindow::Open();
			break;
	}

	Application::HandleConnectionEvent(event, address, param);
}

void Game::HandlePlayerEvent(PlayerEvent event, Player *player, const void *param)
{
	switch (event)
	{
		case kPlayerConnected:
		{
			if (TheMessageMgr->Server())
			{
				GamePlayer *p = static_cast<GamePlayer *>(TheMessageMgr->GetFirstPlayer());
				while (p)
				{
					if ((p != player) && (p->GetPlayerFlags() & kPlayerReceiveVoiceChat)) new Channel(player, p);
					p = p->Next();
				}
			}

			break;
		}

		case kPlayerDisconnected:
		{
			Controller *controller = static_cast<GamePlayer *>(player)->GetPlayerController();
			if (controller) delete controller->GetTargetNode();

			if( TheNetworkMgr->GetLocalAddress() == player->GetPlayerAddress() ) {

				TheGame->ExitCurrentGame(true);
				MainWindow::Open();

			// Remove this player from the lobby, if it still exists - CW
			} else if( TheBloodTideLobby ) {

				TheBloodTideLobby->PlayerDisconnected(*player);
			}

			break;
		}

		case kPlayerTimedOut:
		{
			Controller *controller = static_cast<GamePlayer *>(player)->GetPlayerController();
			if (controller) delete controller->GetTargetNode();

			// Remove this player from the lobby, if it still exists - CW
			if( TheBloodTideLobby ) {

				TheBloodTideLobby->PlayerDisconnected(*player);
			}

			break;
		}

		case kPlayerInitialized:
		{
			// A new player joining the game has been initialized. For each player already
			// in the game, send a message containing the existing player's style to the new player.

			const GamePlayer *gamePlayer = static_cast<GamePlayer *>(TheMessageMgr->GetFirstPlayer());
			do
			{
				gamePlayer = static_cast<GamePlayer *>(gamePlayer->Next());
			} while (gamePlayer);

			// Now tell the new player what world is being played.

			World *world = TheWorldMgr->GetWorld();
			if( world ) {

				if( TheBloodTideLobby )
					player->SendMessage(GameInfoMessage(multiplayerFlags, world->GetWorldName(), TheBloodTideLobby->GetWorldID()));
			}
			break;
		}

		case kPlayerChatReceived:
		{
			if( TheChatInterface ) {
				String<kMaxChatMessageLength + kMaxPlayerNameLength + 32> string(player->GetPlayerName());
				string += TheGame->GetStringTable()->GetString(StringID('MULT', 'CHAT'));
				string += static_cast<const char *>(param);

				TheChatInterface->AddText(string);
			}

			break;
		}

		case kPlayerRenamed:
		{
			// Update the player's name if the Blood Tide lobby, if it still exists - CW
			if( TheBloodTideLobby ) {

				TheBloodTideLobby->RenamePlayer(*player);
			}

			break;
		}
	}

	Application::HandlePlayerEvent(event, player, param);
}

void Game::ReceiveMessage(Player *sender, const NetworkAddress& address, const Message *message)
{
	if (message->GetMessageType() == kMessageServerInfo)
	{
		if (TheJoinGameWindow) TheJoinGameWindow->ReceiveServerInfo(address, static_cast<const ServerInfoMessage *>(message));
	}
}

void Game::InitializeModel(const CreateModelMessage *message, GameWorld *world, Model *model, Controller *controller)
{
	controller->SetControllerIndex(message->GetControllerIndex());
	model->SetController(controller);

	const Point3D& position = message->GetInitialPosition();
	Zone *zone = world->FindZone(position);

	Point3D zonePosition = zone->GetInverseWorldTransform() * position;
	model->SetNodePosition(zonePosition);
	zone->AddNewSubnode(model);
	model->Update();

	if (TheMessageMgr->Synchronized()) controller->EnterWorld(zone, zonePosition);
}

void Game::InitializeEffect(const CreateEffectMessage& message, Effect& effect, Controller& effectController, BTEntityController& target)
{
	effectController.SetControllerIndex(message.GetControllerIndex());
	effect.SetController(&effectController);

	const Point3D& position = target.GetPosition();
	Zone *zone = target.GetTargetNode()->GetOwningZone();

	Point3D zonePosition = zone->GetInverseWorldTransform() * position;

	effect.SetNodePosition(zonePosition);
	zone->AddNewSubnode(&effect);
	effect.Update();

	if (TheMessageMgr->Synchronized()) effectController.EnterWorld(zone, zonePosition);
}

void Game::CreateModel(const CreateModelMessage *message)
{
	GameWorld *world = static_cast<GameWorld *>(TheWorldMgr->GetWorld());
	if (world)
	{
		MessageType type = message->GetModelMessageType();
		switch (type)
		{
			case kModelMessageCommander:
			{
				const CreateCommanderMessage *m = static_cast<const CreateCommanderMessage *>(message);

				GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetPlayer(m->GetPlayerKey()) );

				unsigned long ulEntityID = m->GetEntityID();
				long startKillCount = m->GetStartKillCount();

				// Get stats from string table
				float mhp = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'STAT', 'MHP' )));
				float hp = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'STAT', 'HP' )));
				float str = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'STAT', 'STR' )));
				float armr = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'STAT', 'ARMR' )));
				float mspd = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'STAT', 'MSPD' )));

				StatsProtocol *pStats = new StatsProtocol( mhp, hp, str, armr, mspd );

				// Apply upgrades from base - increase by percentage
				if( m->GetBaseIndex() >= 0 && pStats ) {
					BaseController *bcont = static_cast<BaseController*>( world->GetController(m->GetBaseIndex()) );
					if( bcont ) {
						pStats->SetEffectOnMaxHealth( mhp * (bcont->GetUpgrades().GetBaseMaxHealth() * 0.01F));
						pStats->SetEffectOnMovementSpeed( mspd * (bcont->GetUpgrades().GetBaseMovementSpeed() * 0.01F));
						pStats->SetEffectOnStrength( str * (bcont->GetUpgrades().GetBaseStrength() * 0.01F));
						pStats->SetEffectOnArmor( armr * (bcont->GetUpgrades().GetBaseArmor() * 0.01F));
					}
				}

				// Get collision/model information
				float radius = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'MDL', 'RADI' )));
				float height = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'MDL', 'HGHT' )));
				float float_height = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'MDL', 'FLTZ' )));

				float mdl_ori = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableCommanders)->GetString(StringID(ulEntityID, 'MDL', 'ORI' )));

				long team = 0;
				if( player ) {
					team = player->GetPlayerTeam();
				}

				CommanderController *controller = new CommanderController( m->GetInitialPosition(), ulEntityID, kStringTableCommanders, pStats, radius, height, float_height, team, mdl_ori, startKillCount );
				pStats->SetOwner( controller );

				float azm = m->GetInitialAzimuth();
				controller->SetPrimaryAzimuth(azm);
				controller->SetLookAzimuth(azm);
				controller->SetLookAltitude(m->GetInitialAltitude());

				controller->SetCharFlags(m->GetCharacterFlags());
				controller->SetMovementFlags(m->GetMovementFlags());

				LazyModelRegistration(ulEntityID, kStringTableCommanders);
				Model *model = Model::Get(ulEntityID);
				if( model->GetController() )
				{
					delete model->GetController();
					model->SetController(nullptr);
				}

				// We don't want to save the player's character, so make it non-persistent.

				model->SetNodeFlags(kNodeNonpersistent);

				if (player) player->SetPlayerController(controller);

				InitializeModel(message, world, model, controller);

				if ((player) && (player == TheMessageMgr->GetLocalPlayer()))
				{
					//world->SetLocalPlayerVisibility();
					//world->SetCameraTargetModel(model);

					// We don't want the local player to be able to see himself for the name popup.

					//controller->SetCollisionExclusionMask(kColliderSightPath);
					// TheCommanderUI->UpdateCommanderIcon( controller->GetControllerIndex() );

					//CommanderInfoPane *pane = static_cast<CommanderInfoPane *>( &TheInfoPaneMgr->GetInfoPane(kInfoPaneCommander) );
					//pane->SetCommander( controller );

					TheCommanderSelecter->Hide();

					delete TheScoreboardInterface;
					//DisplayInterface::Open();

					if( team == player->GetPlayerTeam() ) {
						TheGame->HelpMessage( 'cosp' );
					}
				}

				Array<long> *commanderCache = TheGameCacheMgr->GetGameCache(kCacheCommanders, team);
				if( commanderCache ) commanderCache->AddElement(controller->GetControllerIndex());

				break;
			}

			case kModelMessageUnit:
			{
				const CreateUnitMessage *m = static_cast<const CreateUnitMessage *>(message);

				unsigned long ulEntityID = m->GetEntityID();

				// Get stats from string table
				float mhp = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'STAT', 'MHP' )));
				float hp = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'STAT', 'HP' )));
				float str = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'STAT', 'STR' )));
				float armr = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'STAT', 'ARMR' )));
				float mspd = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'STAT', 'MSPD' )));
				float arng = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'STAT', 'ARNG' )));

				StatsProtocol *pStats = new StatsProtocol( mhp, hp, str, armr, mspd, arng );

				// Apply upgrades from base - increase by percentage
				if( m->GetBaseIndex() >= 0 && pStats ) {
					BaseController *bcont = static_cast<BaseController*>( world->GetController(m->GetBaseIndex()) );
					if( bcont ) {
						pStats->SetEffectOnMaxHealth( mhp * (bcont->GetUpgrades().GetBaseMaxHealth() * 0.01F));
						pStats->SetEffectOnMovementSpeed( mspd * (bcont->GetUpgrades().GetBaseMovementSpeed() * 0.01F));
						pStats->SetEffectOnStrength( str * (bcont->GetUpgrades().GetBaseStrength() * 0.01F));
						pStats->SetEffectOnArmor( armr * (bcont->GetUpgrades().GetBaseArmor() * 0.01F));
					}
				}

				// Get collision/model information
				float radius = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'MDL', 'RADI' )));
				float height = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'MDL', 'HGHT' )));
				float turn_radius = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'MDL', 'TRAD' )));
				float speed = pStats->GetMovementSpeed();
				float float_height = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'MDL', 'FLTZ' )));

				float mdl_ori = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableUnits)->GetString(StringID(ulEntityID, 'MDL', 'ORI' )));

				long team = m->GetTeam();

				UnitController *controller = new UnitController( m->GetInitialPosition(), ulEntityID, kStringTableUnits, pStats, radius, height, float_height, team, mdl_ori );
				pStats->SetOwner( controller );

				float azm = m->GetInitialAzimuth();
				float alt = m->GetInitialAltitude();
				controller->SetAzimuth(azm);
				//controller->SetUnitAltitude(alt);

				controller->SetCharFlags(m->GetCharacterFlags());
				//controller->SetMovementFlags(m->GetMovementFlags());

				controller->GetSteering()->SetMaxTurnRadius(turn_radius);
				controller->GetSteering()->SetSpeed(speed);

				LazyModelRegistration(ulEntityID, kStringTableUnits);
				Model *model = Model::Get(ulEntityID);
				if( model->GetController() )
				{
					delete model->GetController();
					model->SetController(nullptr);
				}

				// We don't want to save the unit, so make it non-persistent.
				model->SetNodeFlags(kNodeNonpersistent);

				InitializeModel(message, world, model, controller);

				Array<long> *unitCache = TheGameCacheMgr->GetGameCache(kCacheUnits, team);
				if( unitCache ) unitCache->AddElement(controller->GetControllerIndex());

				break;
			}

			case kModelMessageBuilding:
			{
				const CreateBuildingMessage *m = static_cast<const CreateBuildingMessage *>(message);

				unsigned long ulEntityID = m->GetEntityID();

				StatsProtocol *pStats = new StatsProtocol(
						Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(ulEntityID, 'STAT', 'MHP' ))),
						Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(ulEntityID, 'STAT', 'HP' ))),
						0.0F, 0.0F, 0.0F,
						Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(ulEntityID, 'STAT', 'ARNG' ))));

				float radius = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(ulEntityID, 'MDL', 'RADI' )));
				float height = Text::StringToFloat(TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(ulEntityID, 'MDL', 'HGHT' )));

				long team = m->GetTeam();
				long time = m->GetTime();
				long plotindex = m->GetPlot();

				// needs to be buildingcontroller, see BTUnit/BTCharacter
				BuildingController *bcontroller = new BuildingController( m->GetInitialPosition(), ulEntityID, pStats, radius, height, team, plotindex, time );
				pStats->SetOwner( bcontroller );

				float azm = m->GetInitialAzimuth();
				bcontroller->SetAzimuth( azm );

				LazyModelRegistration(ulEntityID, kStringTableBuildings);
				Model *model = Model::Get(ulEntityID);
				if( model->GetController() )
				{
					delete model->GetController();
					model->SetController(nullptr);
				}

				// We don't want to save the unit, so make it non-persistent.
				model->SetNodeFlags(kNodeNonpersistent);

				// get the plot, tell controller this is on top of it
				PlotController *plot = static_cast<PlotController*> (TheWorldMgr->GetWorld()->GetController(m->GetPlot()));
				plot->SetBuilding(bcontroller);
				BaseController *base = static_cast<BaseController*> (plot->GetBase());

				// The base isn't ready to build any other buildings until the building itself tells the base it is done constructing
				base->SetReadyForBuilding(false);

				InitializeModel(message, world, model, bcontroller);

				// Add to the cache
				Array<long> *buildingCache = TheGameCacheMgr->GetGameCache(kCacheBuildings, team);
				if( buildingCache ) buildingCache->AddElement( bcontroller->GetControllerIndex() );

				// setup next building reminder
				GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
				if( player && player->GetPlayerTeam() == team ) {
					TheGame->SetNextBuildRemTime( TheTimeMgr->GetAbsoluteTime() + time + BUILD_REMINDER_INTERVAL );
				}

				break;
			}

			case kModelMessageProjectile:
			{
				const CreateProjectileMessage *m = static_cast<const CreateProjectileMessage *>(message);
				const StringTable *table = TheStringTableMgr->GetStringTable(kStringTableProjectiles);

				float radius = Text::StringToFloat( table->GetString( StringID(m->m_ulEntityID, 'MDL', 'RADI')) );
				float height = Text::StringToFloat( table->GetString( StringID(m->m_ulEntityID, 'MDL', 'HGHT')) );
				float ori = Text::StringToFloat( table->GetString( StringID(m->m_ulEntityID, 'MDL', 'ORI')) );
				float speed = Text::StringToFloat( table->GetString( StringID(m->m_ulEntityID, 'SPEE')) );
				float dist = Text::StringToFloat( table->GetString( StringID(m->m_ulEntityID, 'RNGE')) );
				const char *hitSound = table->GetString( StringID(m->m_ulEntityID, 'SOND') );

				const char *soundOnlyOnImpact = table->GetString( StringID(m->m_ulEntityID, 'SOIM') );
				bool bSoundOnlyOnImpact = true;
				if( soundOnlyOnImpact && Text::CompareTextCaseless(soundOnlyOnImpact, "false") ) {
					bSoundOnlyOnImpact = false;
				}

				Vector3D velocity( 1.0F, 0.0F, 0.0F );
				velocity.RotateAboutZ( m->m_fAzimuth );

				BTProjectileController *controller = new BTProjectileController( kControllerBTProjectile, radius, height, 0, m->GetInitialPosition(), m->m_fAzimuth, speed, dist, m->m_lTeam, m->m_fTargetAlt, ori, bSoundOnlyOnImpact, hitSound );

				LazyModelRegistration( m->m_ulEntityID, kStringTableProjectiles );
				Model *model = Model::Get( m->m_ulEntityID );
				if( model->GetController() )
				{
					delete model->GetController();
					model->SetController(nullptr);
				}

				model->SetNodeFlags( kNodeNonpersistent );
				InitializeModel( message, world, model, controller );

				model->SetNodeMatrix3D( model->GetSuperNode()->GetInverseWorldTransform() * Matrix3D().SetRotationAboutY( m->m_fTargetAlt ) * Matrix3D().SetRotationAboutZ(m->m_fAzimuth + (ori * K::pi)) );
				model->Invalidate();

				controller->Wake();

				break;
			}
		}
	}
}

void Game::CreateEffect(const CreateEffectMessage *message)
{
	GameWorld *world = static_cast<GameWorld *>(TheWorldMgr->GetWorld());
	if (world)
	{
		EffectMessageType type = message->GetEffectMessageType();
		switch (type)
		{
			case kEffectMessageBeam:
			{
				const CreateBeamEffectMessage *m = static_cast<const CreateBeamEffectMessage *>(message);

				// Oriente the beam effect
				float azimuth =  m->m_fAzimuth;
				float altitude = m->m_fAltitude;

				altitude *= -1.0F;

				float cp = Cos(altitude);
				float sp = Sin(altitude);

				float ct = Cos(azimuth);
				float st = Sin(azimuth);

				Vector3D view(ct * cp, st * cp, sp);
				Vector3D right(st, -ct, 0.0F);
				Vector3D down = view % right;

				Transform4D transform( right, down, view, m->m_p3dSourcePosition );

				ColorRGBA color;
				if( m->m_cColor == 0 )
					color = K::yellow;
				else
					color = K::blue;

				// Create the beam effect and place it into the world
				BeamEffect *beamEffect = new BeamEffect( 0.4F, m->m_fDistance, color );
				beamEffect->SetNodeTransform( transform );

				BeamEffectController *beamController = new BeamEffectController( 1000 );
				beamEffect->SetController( beamController );

				Zone *zone = world->FindZone(m->m_p3dSourcePosition);
				Point3D zonePosition = zone->GetInverseWorldTransform() * m->m_p3dSourcePosition;
				beamEffect->SetNodePosition(zonePosition);
				zone->AddNewSubnode(beamEffect);

				break;
			}

			case kEffectMessageBolt:
			{
				const CreateBoltEffectMessage *m = static_cast<const CreateBoltEffectMessage *>(message);

				World *world = TheWorldMgr->GetWorld();
				if( world == nullptr )
					break;

				BTEntityController *target = static_cast<BTEntityController *>( world->GetController( m->m_lTargetController ) );
				if( (target== nullptr) || (target->GetBaseControllerType() != kControllerEntity) )
					break;

				QuantumBolt *boltEffect = new QuantumBolt( m->m_p3dSourcePosition, target->GetPosition(), Vector3D(0.0F, 0.0F, 0.0F), 2000 );

				Zone *zone = world->FindZone(m->m_p3dSourcePosition);
				Point3D zonePosition = zone->GetInverseWorldTransform() * m->m_p3dSourcePosition;
				boltEffect->SetNodePosition(zonePosition);
				zone->AddNewSubnode(boltEffect);

				break;
			}

			case kEffectMessagePositional:
			{
				const CreatePositionalEffectMessage *m = static_cast<const CreatePositionalEffectMessage *>(message);

				Zone *zone = world->FindZone(m->m_p3dSourcePosition);
				Point3D zonePosition = zone->GetInverseWorldTransform() * m->GetSourcePosition();

				switch( m->GetPositionalType() )
				{
					case kPositionalEffectMessageShockwave:
					{
						ShockwaveEffect *shockwave = new ShockwaveEffect("effects/Shock", 100.0F, 4.0F, 0.0003125F);
						GrenadeExplosion *explosion = new GrenadeExplosion();

						shockwave->SetNodePosition(zonePosition);
						zone->AddNewSubnode(shockwave);

						explosion->SetNodePosition(zonePosition);
						zone->AddNewSubnode(explosion);

						break;
					}

					case kPositionalEffectMessageDepthCharge:
					{
						ShockwaveEffect *shockwave = new ShockwaveEffect("effects/Shock", 100.0F, 4.0F, 0.0003125F);
						ChargeExplosion *explosion = new ChargeExplosion( zonePosition, Vector3D(0.0F, 0.0F, 1.0F) );

						shockwave->SetNodePosition(zonePosition);
						zone->AddNewSubnode(shockwave);

						explosion->SetNodePosition(zonePosition);
						zone->AddNewSubnode(explosion);

						break;
					}

					case kPositionalEffectMessageSmoke:
					{
						BlackCatExplosion *system = new BlackCatExplosion( ColorRGBA(0.0F, 0.0F, 0.0F, 1.0F) );

						system->SetNodePosition(zonePosition);
						zone->AddNewSubnode(system);

						break;
					}
				}

				break;
			}
			case kEffectMessageTargetted:
			{
				const CreateTargettedEffectMessage *m = static_cast<const CreateTargettedEffectMessage *>(message);

				World *world = TheWorldMgr->GetWorld();
				if( world == nullptr )
					break;

				BTEntityController *target = static_cast<BTEntityController *>( world->GetController( m->m_lTargetController ) );
				if( (target== nullptr) || (target->GetBaseControllerType() != kControllerEntity) )
					break;

				switch( m->GetTargettedType() )
				{
					case kTargettedEffectMessageFlash:
					{
						break;
					}

					case kTargettedEffectMessageHeal:
					{
						MaterializeSystem *system = new MaterializeSystem( ColorRGBA(1.0F, 0.0F, 0.0F, 0.0F), 0.25F);
						system->SetTexture( "effects/heal" );

						AttachToEntityController *attach = new AttachToEntityController( target );
						InitializeEffect(*message, *system, *attach, *target);

						break;
					}

					case kTargettedEffectMessageSparks:
					{
						SparksSystem *system = new SparksSystem( 64, 0.25F );

						AttachToEntityController *attach = new AttachToEntityController( target );
						InitializeEffect(*message, *system, *attach, *target);

						break;
					}

					case kTargettedEffectMessageStun:
					{
						SpiralHelix *system = new SpiralHelix( ColorRGBA(1.0F, 1.0F, 0.0F, 1.0F), 0.2F, 0.001F);
						AttachToEntityController *attach = new AttachToEntityController( target );

						InitializeEffect(*message, *system, *attach, *target);

						break;
					}

					case kTargettedEffectMessageBladeCounter:
					{
						LazyModelRegistration( 'BCTR', kStringTableProjectiles );

						long numBlades = 3;
						float ringRadius = 1.0F;

						Node *ring = new Node();

						float angleBetween = K::two_pi / numBlades; // angle between icons in a circle
						float curAngle = K::pi_over_2;

						for(long a = 0; a < numBlades; a++) {

							Model *model = new Model('BCTR');

							float x = (ringRadius * cos(curAngle));
							float y = (ringRadius* sin(curAngle));
							curAngle += angleBetween;

							model->SetNodePosition( Point3D(-x, -y, 0.0F) );

							ring->AddSubnode( model );
						}

						// Now initialize the blade counter effect controller and place everything in the world
						BladeCounterEffectController *controller = new BladeCounterEffectController( target );

						controller->SetControllerIndex(message->GetControllerIndex());
						ring->SetController(controller);

						const Point3D& position = target->GetPosition();
						Zone *zone = world->FindZone(position);

						Point3D zonePosition = zone->GetInverseWorldTransform() * position;
						ring->SetNodePosition(zonePosition);
						zone->AddNewSubnode(ring);
						ring->Update();

						controller->Wake();

						if (TheMessageMgr->Synchronized()) controller->EnterWorld(zone, zonePosition);

						break;
					}
					case kTargettedEffectMessageTurtleBarrier:
					{
						const StringTable *table = TheStringTableMgr->GetStringTable(kStringTableProjectiles);
						if( table )
						{
							LazyModelRegistration( 'BARR', kStringTableProjectiles );

							Model *model = new Model('BARR');
							AttachToEntityController *controller = new AttachToEntityController( target, true );

							Point3D offset;
							offset.x = -Text::StringToFloat( table->GetString( StringID('BARR', 'OFFS', 'X')) );
							offset.y = -Text::StringToFloat( table->GetString( StringID('BARR', 'OFFS', 'Y')) );
							offset.z = -Text::StringToFloat( table->GetString( StringID('BARR', 'OFFS', 'Z')) );

							//controller->SetOffset( offset );

							controller->SetControllerIndex(message->GetControllerIndex());
							model->SetController(controller);

							const Point3D& position = target->GetPosition();
							Zone *zone = world->FindZone(position);

							Point3D zonePosition = zone->GetInverseWorldTransform() * position;
							model->SetNodePosition(zonePosition);
							zone->AddNewSubnode(model);
							model->Update();

							controller->Wake();

							if (TheMessageMgr->Synchronized()) controller->EnterWorld(zone, zonePosition);

						}

						break;
					}
					case kTargettedEffectMessagePoison:
					{
						BlackCatExplosion *system = new BlackCatExplosion( ColorRGBA(0.0F, 0.3F, 0.0F, 1.0F) );

						AttachToEntityController *attach = new AttachToEntityController( target );
						InitializeEffect(*message, *system, *attach, *target);

						break;
					}
					case kTargettedEffectMessageVanish:
					{
						BlackCatExplosion *system = new BlackCatExplosion( ColorRGBA(0.0F, 0.0F, 0.0F, 1.0F) );

						AttachToEntityController *attach = new AttachToEntityController( target );
						InitializeEffect(*message, *system, *attach, *target);

						break;
					}
				}
			}
		}
	}
}

void Game::LazyModelRegistration(ModelType type, TableID nStringTableID)
{
	int nLength = m_arrUnitModelReg.GetElementCount();
	for( int x = 0; x < nLength; x++ ) {
		ModelRegistration *mr = m_arrUnitModelReg[x];
		if( mr && type == mr->GetModelType() ) {
			return;
		}
	}

	// Registration was not found, so add it.
	if( TheStringTableMgr && TheStringTableMgr->GetStringTable(nStringTableID) ) {
		const char *rsrcName = TheStringTableMgr->GetStringTable(nStringTableID)->GetString(StringID(type, 'MDL'));
		unsigned long flags = kModelPrivate;
		m_arrUnitModelReg.AddElement( new ModelRegistration(type, nullptr, rsrcName, flags) );
	}
}

//void Game::SpawnPlayer(Player *player)
//{
//	GameWorld *world = static_cast<GameWorld *>(TheWorldMgr->GetWorld());
//	if (world)
//	{
//		long count = world->GetSpawnLocatorCount();
//		if (count != 0)
//		{
//			const Marker *marker = world->GetSpawnLocator(Math::Random(count));
//
//			const Vector3D direction = marker->GetWorldTransform()[0];
//			float azimuth = Atan(direction.y, direction.x);
//
//			long index = TheWorldMgr->GetWorld()->NewControllerIndex();
//			TheMessageMgr->SendMessageAll(CreateCommanderMessage(index, 'SPID', marker->GetWorldPosition(), azimuth, 0.0F, K::z_unit, K::zero_3D, K::zero_3D, kCharacterFloat, 0, player->GetPlayerKey()));
//		}
//	}
//}

void Game::ProcessGeometryProperties(const Geometry *geometry, const Point3D& position, const Vector3D& impulse)
{
	const Property *property = geometry->GetFirstProperty();
	while (property)
	{
		//switch (property->GetPropertyType())
		//{
		//}

		property = property->Next();
	}
}

void Game::RefreshScoreboard(const RefreshScoreboardMessage *message)
{
	long count = message->GetPlayerCount();
	for (natural a = 0; a < count; a++)
	{
		const RefreshScoreboardMessage::PlayerData *data = message->GetPlayerData(a);

		GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetPlayer(data->playerKey));
		if (player)
		{
			player->playerPing = data->playerPing;
		}
	}

	if (TheScoreboardInterface) TheScoreboardInterface->Refresh();
}

void Game::ApplicationTask(void)
{
	// Update scripts
	if( TheMessageMgr->Multiplayer() ) {
		TheGame->CheckVictoryConditions();

		if( TheMessageMgr->Server() ) {
			if( TheScriptMgr ) {
				TheScriptMgr->Update();
			}
		}
	}

	// Scoreboard updates
	if (TheMessageMgr->Server())
	{
		long dt = TheTimeMgr->GetSystemDeltaTime();

		GamePlayer *player = static_cast<GamePlayer *>(TheMessageMgr->GetFirstPlayer());
		while (player)
		{
			unsigned long flags = player->GetPlayerFlags();

			long time = MaxZero(player->GetScoreUpdateTime() - dt);
			if (time == 0)
			{
				time = kScoreUpdateInterval;

				player->SetPlayerFlags(flags);
			}

			player->SetScoreUpdateTime(time);

			if (flags & kPlayerScoreboardOpen)
			{
				time = player->GetScoreboardTime() - dt;
				if (time <= 0)
				{
					time = kScoreboardRefreshInterval;

					const GamePlayer *p = static_cast<GamePlayer *>(TheMessageMgr->GetFirstPlayer());
					long playerCount = TheMessageMgr->GetPlayerCount();
					while (playerCount > 0)
					{
						long count = Min(playerCount, kMaxScoreboardRefreshCount);
						RefreshScoreboardMessage message(count);

						for (natural a = 0; a < count; a++)
						{
							RefreshScoreboardMessage::PlayerData *data = message.GetPlayerData(a);

							data->playerKey = p->GetPlayerKey();
							data->playerPing = p->GetNetworkPing();

							p = p->Next();
						}

						player->SendMessage(message);

						playerCount -= count;
					}
				}

				player->SetScoreboardTime(time);
			}

			player = player->Next();
		}

	}

	// UI updates
	TheBTCursor->InterfaceTask(); // TODO: Dedicated - CW

	// Check to see if the local player's commander respawn time has elapsed
	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( player && player->GetDeathTime() != 0 )
	{
		if( (TheTimeMgr->GetAbsoluteTime() - player->GetDeathTime()) >= COMMANDER_RESPAWN_TIME )
		{
			player->SetDeathTime( 0 );

			if( TheCommanderSelecter )
				TheCommanderSelecter->Show();
		}
	}

	// Help message updates
	unsigned long time = TheTimeMgr->GetAbsoluteTime();

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );

	if( TheGame->IsHelpActive() && time >= TheGame->GetNextCommRemTime() && world && world->GetInCommandMode() ) {
		TheGame->HelpMessage('crem');
		TheGame->SetNextCommRemTime( time + COMM_REMINDER_INTERVAL );
	}

	if( TheGame->IsHelpActive() && time >= TheGame->GetNextUnitsRemTime() ) {
		TheGame->HelpMessage('urem');
		TheGame->SetNextUnitsRemTime( time + UNITS_REMINDER_INTERVAL );
	}

	// if the number of buildings is >= to the number of plots, don't remind the player to build
	bool canBuild = true;
	if( player ) {
		Array<long> *plotCache = TheGameCacheMgr->GetGameCache(kCachePlots, player->GetPlayerTeam());
		Array<long> *buildCache = TheGameCacheMgr->GetGameCache(kCacheBuildings, player->GetPlayerTeam());
		if( plotCache != nullptr && buildCache != nullptr ) {
			if( buildCache->GetElementCount() >= plotCache->GetElementCount() ) {
				canBuild = false;
			}
		}
	}

	if( TheGame->IsHelpActive() && time >= TheGame->GetNextBuildRemTime() && canBuild == true ) {
		TheGame->HelpMessage('brem');
		TheGame->SetNextBuildRemTime( time + BUILD_REMINDER_INTERVAL );
	}

	// if we are looking at an entity, hi-lite that entity and display their health
}

void Game::InterfaceRenderTask(void)
{

}

/* ---- Lobby Modify Slot Message ---- */

LobbyModifySlotMessage::LobbyModifySlotMessage() : Message(kMessageLobbyModifySlot)
{

}

LobbyModifySlotMessage::LobbyModifySlotMessage(long lOperation, long lTeamLobbyID, long lPlayerSlotID) :
						Message(kMessageLobbyModifySlot),
						m_lOperation(lOperation),
						m_lTeamLobbyID(lTeamLobbyID),
						m_lPlayerSlotID(lPlayerSlotID)
{
}

void LobbyModifySlotMessage::Compress(Compressor& data) const
{
	data << m_lOperation;
	data << m_lTeamLobbyID;
	data << m_lPlayerSlotID;
}

bool LobbyModifySlotMessage::Decompress(Decompressor& data)
{
	data >> m_lOperation;
	data >> m_lTeamLobbyID;
	data >> m_lPlayerSlotID;

	return (true);
}

/* ---- Lobby Update Message ---- */

LobbyUpdateMessage::LobbyUpdateMessage() :
	Message(kMessageLobbyUpdate),
		m_ulStatus( 0 ),
		m_arrTeam1Keys( new Array<PlayerKey> ),
		m_arrTeam2Keys( new Array<PlayerKey> ),
		m_arrUndecidedKeys( new Array<PlayerKey> )
{

}

LobbyUpdateMessage::LobbyUpdateMessage(unsigned long ulStatus,
									   Array<PlayerKey> &arrTeam1Keys,
									   Array<PlayerKey> &arrTeam2Keys,
									   Array<PlayerKey> &arrUndecidedKeys) :
	Message(kMessageLobbyUpdate),
	m_ulStatus( ulStatus ),
	m_arrTeam1Keys( &arrTeam1Keys ),
	m_arrTeam2Keys( &arrTeam2Keys ),
	m_arrUndecidedKeys( &arrUndecidedKeys )
{

}

LobbyUpdateMessage::~LobbyUpdateMessage()
{
	delete m_arrTeam1Keys;
	delete m_arrTeam2Keys;
	delete m_arrUndecidedKeys;
}

void LobbyUpdateMessage::Compress(Compressor& data) const
{
	long numElements;
	long i;

	data << m_ulStatus;

	numElements = m_arrTeam1Keys->GetElementCount();
	data << numElements;

	for(i=0; i < numElements; i++) {

		data << (*m_arrTeam1Keys)[i];
	}


	numElements = m_arrTeam2Keys->GetElementCount();
	data << numElements;

	for(i=0; i < numElements; i++) {

		data << (*m_arrTeam2Keys)[i];
	}


	numElements = m_arrUndecidedKeys->GetElementCount();
	data << numElements;

	for(i=0; i < numElements; i++) {

		data << (*m_arrUndecidedKeys)[i];
	}
}

bool LobbyUpdateMessage::Decompress(Decompressor& data)
{
	long numElements;
	PlayerKey playerKey;
	long i;

	data >> m_ulStatus;
	data >> numElements;

	for(i=0; i < numElements; i++) {

		data >> playerKey;
		m_arrTeam1Keys->AddElement(playerKey);
	}


	data >> numElements;

	for(i=0; i < numElements; i++) {

		data >> playerKey;
		m_arrTeam2Keys->AddElement(playerKey);
	}


	data >> numElements;

	for(i=0; i < numElements; i++) {

		data >> playerKey;
		m_arrUndecidedKeys->AddElement(playerKey);
	}

	return (true);
}

/* ---- Request Commander Message ---- */

RequestCommanderMessage::RequestCommanderMessage() : Message(kMessageRequestCommander)
{

}

RequestCommanderMessage::RequestCommanderMessage(long lBaseControllerID) : Message(kMessageRequestCommander),
		m_lBaseControllerID(lBaseControllerID)
{

}

void RequestCommanderMessage::Compress(Compressor& data) const
{
	data << m_lBaseControllerID;
}

bool RequestCommanderMessage::Decompress(Decompressor& data)
{
	data >> m_lBaseControllerID;

	return (true);
}

/* ---- Game Over Message ---- */

GameOverMessage::GameOverMessage() :
	Message( kMessageGameOver ),
	m_lWinner( 0 )
{

}

GameOverMessage::GameOverMessage( long lWinner ) :
	Message( kMessageGameOver ),
	m_lWinner( lWinner )
{

}

void GameOverMessage::Compress(Compressor& data) const
{
	data << m_lWinner;
}

bool GameOverMessage::Decompress(Decompressor& data)
{
	data >> m_lWinner;

	return (true);
}

// Display victory or defeat screen.
bool GameOverMessage::HandleMessage(Player *sender) const
{
	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( player ) {
		if( player->GetPlayerTeam() == m_lWinner ) {
			if( TheGame->GetPopScreen_Victory() )
				TheInterfaceMgr->AddElement(  TheGame->GetPopScreen_Victory() );
		}
		else {
			if(  TheGame->GetPopScreen_Defeat() )
				TheInterfaceMgr->AddElement(  TheGame->GetPopScreen_Defeat() );
		}
	}

	TheGame->ExitCurrentGame();

	return (true);
}

/* ---- World Sound Message ---- */

WorldSoundMessage::WorldSoundMessage() :
	Message( kMessageWorldSound ),
	m_fRange( 0.0F ),
	m_strSound( nullptr )
{

}

WorldSoundMessage::WorldSoundMessage( const char *pSound, Point3D& p3dPosition, float fRange ) :
	Message( kMessageWorldSound ),
	m_fRange( fRange ),
	m_strSound( pSound ),
	m_p3dPosition( p3dPosition )
{

}

void WorldSoundMessage::Compress(Compressor& data) const
{
	data << m_fRange;

	data << m_p3dPosition.x;
	data << m_p3dPosition.y;
	data << m_p3dPosition.z;

	data << m_strSound;
}

bool WorldSoundMessage::Decompress(Decompressor& data)
{
	data >> m_fRange;

	data >> m_p3dPosition.x;
	data >> m_p3dPosition.y;
	data >> m_p3dPosition.z;

	data >> m_strSound;

	return (true);
}

// Display victory or defeat screen.
bool WorldSoundMessage::HandleMessage(Player *sender) const
{
	// Play sound
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if(! world ) {
		return true;
	}

	world->CreateWorldSound( m_strSound, world->GetRootNode(), m_p3dPosition, m_fRange );

	return (true);
}

/* ---- Request Building Message ---- */

RequestBuildingMessage::RequestBuildingMessage() : Message(kMessageRequestBuilding)
{

}


RequestBuildingMessage::RequestBuildingMessage(long lPlotControllerID, long lBuildingID) : Message(kMessageRequestBuilding),
		m_lPlotControllerID( lPlotControllerID ),
		m_lBuildingID( lBuildingID )
{

}

void RequestBuildingMessage::Compress(Compressor& data) const
{
	data << m_lPlotControllerID;
	data << m_lBuildingID;
}

bool RequestBuildingMessage::Decompress(Decompressor& data)
{
	data >> m_lPlotControllerID;
	data >> m_lBuildingID;

	return (true);
}

/* ---- Create Projectile Message ---- */

CreateProjectileMessage::CreateProjectileMessage() : CreateModelMessage(kModelMessageProjectile)
{

}

CreateProjectileMessage::CreateProjectileMessage(long lIndex, unsigned long ulEntityID, long lTeam, const Point3D &position, float fAzimuth, float fTargetAlt) :
		CreateModelMessage( kModelMessageProjectile, lIndex, position ),
		m_lTeam( lTeam ),
		m_ulEntityID( ulEntityID ),
		m_fAzimuth( fAzimuth ),
		m_fTargetAlt( fTargetAlt )
{

}

CreateProjectileMessage::~CreateProjectileMessage()
{

}

void CreateProjectileMessage::Compress(Compressor& data) const
{
	CreateModelMessage::Compress(data);

	data << m_lTeam;
	data << m_ulEntityID;
	data << m_fAzimuth;
	data << m_fTargetAlt;
}

bool CreateProjectileMessage::Decompress(Decompressor& data)
{
	CreateModelMessage::Decompress(data);

	data >> m_lTeam;
	data >> m_ulEntityID;
	data >> m_fAzimuth;
	data >> m_fTargetAlt;

	return (true);
}

/* ---- BTEntity Update Position Message ---- */

EntityUpdatePositionMessage::EntityUpdatePositionMessage(long controllerIndex) :
	ControllerMessage(BTEntityController::kEntityMessageUpdatePosition, controllerIndex)
{
}

EntityUpdatePositionMessage::EntityUpdatePositionMessage(long controllerIndex, const Point3D& position) :
	ControllerMessage(BTEntityController::kEntityMessageUpdatePosition, controllerIndex),
	updatePosition( position )
{
	SetMessageFlags(kMessageUnreliable);
}

EntityUpdatePositionMessage::~EntityUpdatePositionMessage()
{
}

void EntityUpdatePositionMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress(data);

	data << updatePosition.x;
	data << updatePosition.y;
	data << updatePosition.z;
}

bool EntityUpdatePositionMessage::Decompress(Decompressor& data)
{
	if (ControllerMessage::Decompress(data))
	{
		data >> updatePosition.x;
		data >> updatePosition.y;
		data >> updatePosition.z;

		return (true);
	}

	return (false);
}

/* ---- BTEntity Update Azimuth Message ---- */

EntityUpdateAzimuthMessage::EntityUpdateAzimuthMessage(long controllerIndex) : ControllerMessage(BTEntityController::kEntityMessageUpdateAzimuth, controllerIndex)
{
}

EntityUpdateAzimuthMessage::EntityUpdateAzimuthMessage(long controllerIndex, float& fAzimuth) :
	ControllerMessage(BTEntityController::kEntityMessageUpdateAzimuth, controllerIndex),
	m_fUpdateAzimuth( fAzimuth )
{
	SetMessageFlags(kMessageUnreliable);
}

EntityUpdateAzimuthMessage::~EntityUpdateAzimuthMessage()
{
}

void EntityUpdateAzimuthMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress(data);

	data << m_fUpdateAzimuth;
}

bool EntityUpdateAzimuthMessage::Decompress(Decompressor& data)
{
	if (ControllerMessage::Decompress(data))
	{
		data >> m_fUpdateAzimuth;

		return (true);
	}

	return (false);
}

/* ---- Entity Ability Message ---- */

EntityAbilityMessage::EntityAbilityMessage(long controllerIndex) :
		ControllerMessage( BTEntityController::kEntityMessageAbility, controllerIndex ),
		m_lAbility( 0 )
{
}

EntityAbilityMessage::EntityAbilityMessage(long controllerIndex, long lAbility) :
		ControllerMessage( BTEntityController::kEntityMessageAbility, controllerIndex ),
		m_lAbility( lAbility )
{

}

EntityAbilityMessage::~EntityAbilityMessage()
{
}

void EntityAbilityMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress(data);

	data << m_lAbility;
}

bool EntityAbilityMessage::Decompress(Decompressor& data)
{
	if (ControllerMessage::Decompress(data))
	{
		data >> m_lAbility;

		return (true);
	}

	return (false);
}

/* -------------- Play Animation Message -------------- */

EntityPlayAnimationMessage::EntityPlayAnimationMessage( long lControllerIndex ) : ControllerMessage( BTEntityController::kEntityMessagePlayAnimation, lControllerIndex)
{

}

EntityPlayAnimationMessage::EntityPlayAnimationMessage( long lControllerIndex, long lEntityMotion ) : ControllerMessage( BTEntityController::kEntityMessagePlayAnimation, lControllerIndex),
		m_lEntityMotion( lEntityMotion )
{

}

void EntityPlayAnimationMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress( data );

	data << m_lEntityMotion;
}

bool EntityPlayAnimationMessage::Decompress(Decompressor& data)
{
	if( ControllerMessage::Decompress( data ) )
	{
		data >> m_lEntityMotion;

		return (true);
	}

	return (false);
}

/* -------------- Entity Update Stat Message -------------- */

EntityUpdateStatsMessage::EntityUpdateStatsMessage( long lControllerIndex ) :
		ControllerMessage( BTEntityController::kEntityMessageUpdateStat, lControllerIndex )
{

}

EntityUpdateStatsMessage::EntityUpdateStatsMessage( long lControllerIndex, unsigned long ulDirtyStats, Array<float>& arrStats ) :
		ControllerMessage( BTEntityController::kEntityMessageUpdateStat, lControllerIndex ),
		m_ulDirtyStats( ulDirtyStats ),
		m_arrStats( arrStats.GetElementCount() )
{
	long size = arrStats.GetElementCount();

	for( long x = 0; x < size; x++ )
		m_arrStats.AddElement( arrStats[x] );
}

void EntityUpdateStatsMessage::Compress(Compressor& data) const
{
	ControllerMessage::Compress( data );

	data << m_ulDirtyStats;

	long size = m_arrStats.GetElementCount();
	for( long x = 0; x < size; x++ )
	{
		data << m_arrStats[x];
	}
}

bool EntityUpdateStatsMessage::Decompress(Decompressor& data)
{
	if( ControllerMessage::Decompress( data ) )
	{
		data >> m_ulDirtyStats;

		float val;
		long numBits = sizeof(long) * 8;

		for( long x = 0; x < numBits; x++ )
		{
			if( m_ulDirtyStats & 1 << x )
			{
				data >> val;
				m_arrStats.AddElement( val );
			}
		}

		return (true);
	}

	return (false);
}

/* -------------- Create Effect Message -------------- */

CreateEffectMessage::CreateEffectMessage() : Message( kMessageCreateEffect ),
		m_lControllerIndex( -1 )
{

}

CreateEffectMessage::CreateEffectMessage( EffectMessageType type, long lControllerIndex ) : Message( kMessageCreateEffect ),
		m_effectMessageType( type ),
		m_lControllerIndex( lControllerIndex )
{
}

CreateEffectMessage::~CreateEffectMessage()
{
}

void CreateEffectMessage::Compress(Compressor& data) const
{
	data << (long) m_effectMessageType;
	data << m_lControllerIndex;
}

bool CreateEffectMessage::Decompress(Decompressor& data)
{
	data >> m_lControllerIndex;

	return (true);
}

bool CreateEffectMessage::HandleMessage(Player *sender) const
{
	TheGame->CreateEffect(this);
	return (true);
}

CreateEffectMessage *CreateEffectMessage::ConstructMessage(EffectMessageType type)
{
	switch (type)
	{
		case kEffectMessageBeam:

			return (new CreateBeamEffectMessage(type));

		case kEffectMessageBolt:

			return (new CreateBoltEffectMessage(type));

		case kEffectMessageTargetted:

			return (new CreateTargettedEffectMessage(type));

		case kEffectMessagePositional:

			return (new CreatePositionalEffectMessage(type));
	}

	return (nullptr);
}

/* ---- Create Beam Effect Message ---- */

CreateBeamEffectMessage::CreateBeamEffectMessage( EffectMessageType type ) : CreateEffectMessage( type ),
		m_p3dSourcePosition( 0.0F, 0.0F, 0.0F ),
		m_fAltitude( 0.0F ),
		m_fAzimuth( 0.0F ),
		m_fDistance( 0.0F )
{

}

CreateBeamEffectMessage::CreateBeamEffectMessage(Point3D& p3dSourcePosition, float fAltitude, float fAzimuth, float fDistance, char color) : CreateEffectMessage(kEffectMessageBeam),
		m_p3dSourcePosition( p3dSourcePosition ),
		m_fAltitude( fAltitude ),
		m_fAzimuth( fAzimuth ),
		m_fDistance( fDistance ),
		m_cColor( color )
{

}

CreateBeamEffectMessage::~CreateBeamEffectMessage()
{

}

void CreateBeamEffectMessage::Compress(Compressor& data) const
{
	CreateEffectMessage::Compress(data);

	data << m_p3dSourcePosition.x;
	data << m_p3dSourcePosition.y;
	data << m_p3dSourcePosition.z;

	data << m_fAltitude;
	data << m_fAzimuth;
	data << m_fDistance;

	data << m_cColor;
}

bool CreateBeamEffectMessage::Decompress(Decompressor& data)
{
	if( CreateEffectMessage::Decompress(data) )
	{
		data >> m_p3dSourcePosition.x;
		data >> m_p3dSourcePosition.y;
		data >> m_p3dSourcePosition.z;

		data >> m_fAltitude;
		data >> m_fAzimuth;
		data >> m_fDistance;

		data >> m_cColor;

		return (true);
	}

	return (false);
}

/* ---- Create Bolt Effect Message ---- */

CreateBoltEffectMessage::CreateBoltEffectMessage( EffectMessageType type ) : CreateEffectMessage( type )
{

}

CreateBoltEffectMessage::CreateBoltEffectMessage( Point3D& p3dSourcePosition, long lTargetController ) : CreateEffectMessage( kEffectMessageBolt ),
		m_p3dSourcePosition( p3dSourcePosition ),
		m_lTargetController( lTargetController )
{

}

CreateBoltEffectMessage::~CreateBoltEffectMessage()
{

}

void CreateBoltEffectMessage::Compress(Compressor& data) const
{
	CreateEffectMessage::Compress(data);

	data << m_p3dSourcePosition.x;
	data << m_p3dSourcePosition.y;
	data << m_p3dSourcePosition.z;

	data << m_lTargetController;
}

bool CreateBoltEffectMessage::Decompress(Decompressor& data)
{
	if( CreateEffectMessage::Decompress(data) )
	{
		data >> m_p3dSourcePosition.x;
		data >> m_p3dSourcePosition.y;
		data >> m_p3dSourcePosition.z;

		data >> m_lTargetController;

		return (true);
	}

	return (false);
}

/* ---- Create Positional Effect Message ---- */

CreatePositionalEffectMessage::CreatePositionalEffectMessage( EffectMessageType type ) : CreateEffectMessage( type ),
		m_p3dSourcePosition( 0.0F, 0.0F, 0.0F )

{

}

CreatePositionalEffectMessage::CreatePositionalEffectMessage( PositionalEffects type, long lControllerIndex, Point3D& p3dSourcePosition ) : CreateEffectMessage( kEffectMessagePositional, lControllerIndex),
		m_effectPositionalType( type ),
		m_p3dSourcePosition( p3dSourcePosition )
{

}

CreatePositionalEffectMessage::~CreatePositionalEffectMessage()
{

}

void CreatePositionalEffectMessage::Compress(Compressor& data) const
{
	CreateEffectMessage::Compress(data);

	data << (long) m_effectPositionalType;

	data << m_p3dSourcePosition.x;
	data << m_p3dSourcePosition.y;
	data << m_p3dSourcePosition.z;
}

bool CreatePositionalEffectMessage::Decompress(Decompressor& data)
{
	if( CreateEffectMessage::Decompress(data) )
	{
		long val;
		data >> val;
		m_effectPositionalType = (PositionalEffects) val;

		data >> m_p3dSourcePosition.x;
		data >> m_p3dSourcePosition.y;
		data >> m_p3dSourcePosition.z;

		return (true);
	}

	return (false);
}

/* ---- Create Targgeted Effect Message ---- */

CreateTargettedEffectMessage::CreateTargettedEffectMessage( EffectMessageType type ) : CreateEffectMessage( type )
{

}

CreateTargettedEffectMessage::CreateTargettedEffectMessage( TargettedEffects type, long lControllerIndex, long lTargetController ) : CreateEffectMessage( kEffectMessageTargetted, lControllerIndex ),
		m_effectTargettedType( type ),
		m_lTargetController( lTargetController )
{

}

CreateTargettedEffectMessage::~CreateTargettedEffectMessage()
{

};

void CreateTargettedEffectMessage::Compress(Compressor& data) const
{
	CreateEffectMessage::Compress(data);

	data << (long) m_effectTargettedType;
	data << m_lTargetController;
}

bool CreateTargettedEffectMessage::Decompress(Decompressor& data)
{
	if( CreateEffectMessage::Decompress(data) )
	{
		long val;
		data >> val;
		m_effectTargettedType = (TargettedEffects) val;

		data >> m_lTargetController;
		return (true);
	}

	return (false);
}
