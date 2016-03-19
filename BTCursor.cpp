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

#include "BTCursor.h"
#include "MGGame.h"
#include "BTCommanderUI.h"
#include "BTBuildingUI.h"
#include "BTCacheManager.h"
#include "BTGoalMove.h"
#include "PointFinder.h"
#include "C4Primitives.h"

using namespace C4;

#define TRACE_TIME_DELTA 1000.0F // The delta time between ray traces for cursor hovering (in milliseconds)
#define QUERY_RADIUS 0.3F // radius of the swept sphere - passed in to World::QueryWorld()
#define MAX_RAY_LENGTH 1000.0F // max ray length of a camera ray

#define AVERAGING_OFFSET_MULT 0.1F	// When ordering many units to one location, further away units will go to a position nearby according to this offset

BTCursor *C4::TheBTCursor = nullptr;
DefaultState *C4::TheDefaultState = nullptr;
SelectBoxState *C4::TheSelectBoxState = nullptr;

/* ------------------ DefaultState ------------------- */

// last hover time is set to the delta time between ray traces,
// so that it would pass the check by default
DefaultState::DefaultState() : Singleton<DefaultState>(TheDefaultState),
			m_ulLastTraceTime(TRACE_TIME_DELTA),
			m_pStartPosition(nullptr),
			m_bMouseUpValid(false)
{
}

DefaultState::~DefaultState()
{
	if( m_pStartPosition )
		delete m_pStartPosition;

	m_pStartPosition = nullptr;
}

void DefaultState::Enter(const Point &point)
{
	TheInterfaceMgr->ShowCursor();
}

void DefaultState::Hover(const Point &point)
{
	Point currentPosition = TheInterfaceMgr->GetMousePosition();

	// has the mouse moved? if so, start drawing a select box by changeing the cursor state
	if( m_pStartPosition && *m_pStartPosition != currentPosition ) {

		TheBTCursor->ChangeState(TheSelectBoxState, *m_pStartPosition);

	} else {

		if( TheBTCursor->CursorCollidesWithElement() == true ) // TODO: Change mouse cursor back to normal..
			return;

		unsigned long currentTime = TheTimeMgr->GetAbsoluteTime();

		if( (currentTime - m_ulLastTraceTime) < TRACE_TIME_DELTA )
			return;
		else
			m_ulLastTraceTime = currentTime;


		//Model *model = TheBTCursor->GetEntityFromMousePosition( TheInterfaceMgr->GetMousePosition() );

		//if( model ) {
		//	TheChatWindow->AddText( "Hovering over target." );
		//} else {
		//	TheChatWindow->AddText( "No hover target." );
		//}
	}
}

void DefaultState::MouseDown(const Point &point)
{	
	if( TheBTCursor->CursorCollidesWithElement() == true )
		return;

	m_pStartPosition = new Point(point.x, point.y);
	m_bMouseUpValid = true;
}

void DefaultState::MouseUp(const Point &point)
{
	if( m_pStartPosition ) {

		delete m_pStartPosition;
		m_pStartPosition = nullptr;
	}

	if( m_bMouseUpValid == false ) return;
	m_bMouseUpValid = false;

	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	UnitOrderType unitOrder = TheBTCursor->GetOrder();

	if( unitOrder != kOrderNone ) {

		GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
		if( !world ) {
			return;
		}

		const Array<long> &units = TheBTCursor->GetSelectedUnits();
		long numElements = units.GetElementCount();

		if( numElements <= 0 )
			return;

		Array<UnitCommand> *commands = new Array<UnitCommand>( numElements );

		if( unitOrder == kOrderAttackTo ) {

			float avgX = 0.0F;
			float avgY = 0.0F;
			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				avgX += controller->GetPosition().x;
				avgY += controller->GetPosition().y;
			}
			avgX /= numElements;
			avgY /= numElements;
			Point3D avg( avgX, avgY, 0.0F );

			int commandsSent = 0;

			float averageHeight = 0.0F;

			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				BTCylinderCollider *col = static_cast<BTCylinderCollider*>( controller->GetCollider() );
				if( !col ) {
					continue;
				}

				averageHeight += controller->GetFloatHeight();

				GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
				if( player->GetPlayerTeam() != controller->GetTeam() ) continue;

				Vector3D offset = controller->GetPosition() - avg;
				offset *= AVERAGING_OFFSET_MULT;

				Point3D tar = TheBTCursor->GetCameraRayBisectionWithHeight(point, controller->GetFloatHeight()) + offset;
				if( !world->PointWithinMapBounds(tar) ) {
					Array<Point3D> safe_points;
					PointFinder::FindSafePoint_Spiral( tar, 1, safe_points, 1.0F, 1.0F, K::pi_over_6, 10.0F, col->GetRadius() * UNIT_SAFE_DIST_MULT, 0.0F, false );

					if( safe_points.GetElementCount() < 1 ) {
						continue;
					}

					tar = safe_points[0];
				}

				tar.z = controller->GetPosition().z;

				UnitCommand command;
				command.m_lUnit = units[i];
				command.m_p3dTarget = tar;

				commands->AddElement( command );
				commandsSent++;

				if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttackTo, *commands) );
					commandsSent = 0;
					commands->Purge();
				}
			}

			if( commands > 0 ) {
				TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttackTo, *commands) );
			}

			// setup next units reminder
			TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

			// Show visual
			Point3D matPoint = TheBTCursor->GetCameraRayBisectionWithHeight(point, (averageHeight/numElements));
			Zone *zone = world->FindZone( matPoint );
			if( zone ) {
				MaterializeSystem *system = new MaterializeSystem( ColorRGB(1.0F, 0.0F, 0.0F), 0.5F );
				system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
				zone->AddNewSubnode( system );
			}
		}

		TheBTCursor->SetOrder(kOrderNone);

		delete commands;
	}
	else {
		TheUnitIconMgr->PurgeUnits();
		TheBTCursor->PurgeSelectedUnits();
		TheInfoPaneMgr->ShowInfoPane( kInfoPaneNone );

		BTEntityController *entity = TheBTCursor->GetEntityFromMousePosition(TheInterfaceMgr->GetMousePosition());
		if( entity == nullptr ) { // User may be trying to create a select box

			return;
		}

		TheBTCursor->ShowEntityInformation( entity->GetControllerIndex() );
	}
}

void DefaultState::RightMouseDown(const Point &point)
{
	if( TheBTCursor->CursorCollidesWithElement() == true )
		return;

	m_bMouseUpValid = true;

	BTEntityController *entity = TheBTCursor->GetEntityFromMousePosition(TheInterfaceMgr->GetMousePosition());

	if( (entity != nullptr) && (entity->GetControllerType() == kControllerPlot) ) {

		PlotController *plot = static_cast<PlotController *>( entity );
		GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );

		if( plot == nullptr || player == nullptr){
			return;
		}

		if( plot->GetTeam() != player->GetPlayerTeam() )
		{
			return;
		}

		if( plot->GetBuilding() )
			return;

		BaseController *base = static_cast<BaseController*>( plot->GetBase() );
		if( !base ) {
			return;
		}

		if( base->IsActiveBase() == false ) {
			return;
		}

		if( TheGame->IsReadyForBuilding(plot->GetTeam()) == false )
		{
			// warn that only 1 building can be built at once
			unsigned long now = TheTimeMgr->GetAbsoluteTime();
			if( TheGame->IsHelpActive() && now >= TheGame->GetNextNoBuildTime() ) {
				TheGame->HelpMessage('buno');
				TheGame->SetNextNoBuildTime( now + NO_BUILD_INTERVAL );
			}
			return;
		}

		Array<Type> possible;  
		plot->GetPossibleBuildings( possible );
		BuildingsMenu::New( plot->GetControllerIndex(), possible, point );


		m_bMouseUpValid = false;
		return;
	}

	m_bMouseUpValid = true;
}

void DefaultState::RightMouseUp(const Point &point)
{
	if( m_bMouseUpValid == false ) return;
	m_bMouseUpValid = false;

	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	UnitOrderType unitOrder = TheBTCursor->GetOrder();

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	const Array<long> &units = TheBTCursor->GetSelectedUnits();
	long numElements = units.GetElementCount();

	if( numElements <= 0 )
		return;

	Array<UnitCommand> *commands = new Array<UnitCommand>( numElements );

	if( unitOrder == kOrderNone ) {

		// First check if we are right clicking on an entity, if so try to act upon it
		BTEntityController *target;
		long targetIndex;

		target = TheBTCursor->GetEntityFromMousePosition( point );
		if( target && target->GetBaseControllerType() == kControllerEntity ) {
	
			// If a plot was selected, try to get the building instead.
			if( target->GetControllerType() == kControllerPlot ) {
				PlotController *pc = static_cast<PlotController*>( target );
				if( !pc ) {
					delete commands;
					return;
				}

				if( pc->GetBuilding() == nullptr || pc->GetBuilding()->GetControllerType() != kControllerBuilding ) {
					delete commands;
					return;
				}
				
				target = pc->GetBuilding();
			}

			GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
			if( target->GetTeam() != player->GetPlayerTeam() ) {
				// Attack this entity

				targetIndex = target->GetControllerIndex();

				int commandsSent = 0;

				for( long i = 0; i < numElements; i++ ) {

					BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
					if( controller == nullptr ) continue;

					UnitCommand command;
					command.m_lUnit = units[i];
					command.m_lTarget = targetIndex;

					commands->AddElement( command );
					commandsSent++;

					if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
						TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttack, *commands) );
						commandsSent = 0;
						commands->Purge();
					}
				}

				if( commands > 0 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttack, *commands) );
				}

				// setup next units reminder
				TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

				// Show visual
				Point3D matPoint = TheBTCursor->GetCameraRayBisectionWithHeight(point, target->GetPosition().z);
				Zone *zone = world->FindZone( matPoint );
				if( zone ) {
					MaterializeSystem *system = new MaterializeSystem( ColorRGB(1.0F, 0.0F, 0.0F), 0.5F );
					system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
					zone->AddNewSubnode( system );
				}
			}
			else {
				// Follow this entity
				
				targetIndex = target->GetControllerIndex();

				int commandsSent = 0;

				for( long i = 0; i < numElements; i++ ) {

					BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
					if( controller == nullptr ) continue;

					UnitCommand command;
					command.m_lUnit = units[i];
					command.m_lTarget = targetIndex;

					commands->AddElement( command );
					commandsSent++;

					if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
						TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandFollow, *commands) );
						commandsSent = 0;
						commands->Purge();
					}
				}

				if( commands > 0 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandFollow, *commands) );
				}

				// Show visual
				Point3D matPoint = TheBTCursor->GetCameraRayBisectionWithHeight(point, target->GetPosition().z);
				Zone *zone = world->FindZone( matPoint );
				if( zone ) {
					MaterializeSystem *system = new MaterializeSystem( ColorRGB(1.0F, 1.0F, 0.0F), 0.5F );
					system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
					zone->AddNewSubnode( system );
				}
			}

		} else {

			float avgX = 0.0F;
			float avgY = 0.0F;
			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				avgX += controller->GetPosition().x;
				avgY += controller->GetPosition().y;
			}
			avgX /= numElements;
			avgY /= numElements;
			Point3D avg( avgX, avgY, 0.0F );

			int commandsSent = 0;

			float averageHeight = 0.0F;

			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				BTCylinderCollider *col = static_cast<BTCylinderCollider*>( controller->GetCollider() );
				if( !col ) {
					continue;
				}

				averageHeight += controller->GetFloatHeight();

				GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
				if( player->GetPlayerTeam() != controller->GetTeam() ) continue;

				Vector3D offset = controller->GetPosition() - avg;
				offset *= AVERAGING_OFFSET_MULT;

				Point3D tar = TheBTCursor->GetCameraRayBisectionWithHeight(point, controller->GetFloatHeight()) + offset;
				if( !world->PointWithinMapBounds(tar) ) {
					Array<Point3D> safe_points;
					PointFinder::FindSafePoint_Spiral( tar, 1, safe_points, 1.0F, 1.0F, K::pi_over_6, 10.0F, col->GetRadius() * UNIT_SAFE_DIST_MULT, 0.0F, false );

					if( safe_points.GetElementCount() < 1 ) {
						continue;
					}

					tar = safe_points[0];
				}

				tar.z = controller->GetPosition().z;

				UnitCommand command;
				command.m_lUnit = units[i];
				command.m_p3dTarget = tar;

				commands->AddElement( command );
				commandsSent++;

				if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandMove, *commands) );
					commandsSent = 0;
					commands->Purge();
				}
			}

			if( commands > 0 ) {
				TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandMove, *commands) );
			}

			// setup next units reminder
			TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

			// Show visual
			Point3D matPoint = TheBTCursor->GetCameraRayBisectionWithHeight(point, (averageHeight/numElements));
			Zone *zone = world->FindZone( matPoint );
			if( zone ) {
				MaterializeSystem *system = new MaterializeSystem( ColorRGB(0.0F, 1.0F, 0.0F), 0.5F );
				system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
				zone->AddNewSubnode( system );
			}
		}
	}
	else {
		
		if( unitOrder == kOrderAttackTo ) {

			float avgX = 0.0F;
			float avgY = 0.0F;
			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				avgX += controller->GetPosition().x;
				avgY += controller->GetPosition().y;
			}
			avgX /= numElements;
			avgY /= numElements;
			Point3D avg( avgX, avgY, 0.0F );

			int commandsSent = 0;

			float averageHeight = 0.0F;

			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				BTCylinderCollider *col = static_cast<BTCylinderCollider*>( controller->GetCollider() );
				if( !col ) {
					continue;
				}

				averageHeight += controller->GetFloatHeight();

				GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
				if( player->GetPlayerTeam() != controller->GetTeam() ) continue;

				Vector3D offset = controller->GetPosition() - avg;
				offset *= AVERAGING_OFFSET_MULT;

				Point3D tar = TheBTCursor->GetCameraRayBisectionWithHeight(point, controller->GetFloatHeight()) + offset;
				if( !world->PointWithinMapBounds(tar) ) {
					Array<Point3D> safe_points;
					PointFinder::FindSafePoint_Spiral( tar, 1, safe_points, 1.0F, 1.0F, K::pi_over_6, 10.0F, col->GetRadius() * UNIT_SAFE_DIST_MULT, 0.0F, false );

					if( safe_points.GetElementCount() < 1 ) {
						continue;
					}

					tar = safe_points[0];
				}

				tar.z = controller->GetPosition().z;

				UnitCommand command;
				command.m_lUnit = units[i];
				command.m_p3dTarget = tar;

				commands->AddElement( command );
				commandsSent++;

				if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttackTo, *commands) );
					commandsSent = 0;
					commands->Purge();
				}
			}

			if( commands > 0 ) {
				TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttackTo, *commands) );
			}

			// setup next units reminder
			TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

			// Show visual
			Point3D matPoint = TheBTCursor->GetCameraRayBisectionWithHeight(point, (averageHeight/numElements));
			Zone *zone = world->FindZone( matPoint );
			if( zone ) {
				MaterializeSystem *system = new MaterializeSystem( ColorRGB(1.0F, 0.0F, 0.0F), 0.5F );
				system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
				zone->AddNewSubnode( system );
			}
		}

		TheBTCursor->SetOrder(kOrderNone);
	}

	delete commands;
}

void DefaultState::Exit(const Point &point)
{
	if( m_pStartPosition )
		delete m_pStartPosition;

	m_pStartPosition = nullptr;
	m_bMouseUpValid = false;
}

/* ------------------ SelectBoxState ------------------- */

SelectBoxState::SelectBoxState() : Singleton<SelectBoxState>(TheSelectBoxState),
					m_pStartPosition(nullptr)
{
	m_pSelectBox = new OutlineElement(0.0F,0.0F,1.0F);

	m_pSelectBox->SetElementPosition(Point3D(0.0F, 0.0F, 0.0F));
	m_pSelectBox->SetRenderType(kRenderLineLoop);
	m_pSelectBox->SetOutlineColor(ColorRGB(0.0F,100.0F,0.0F));
	m_pSelectBox->SetOutlineThickness(0.1F);
}

SelectBoxState::~SelectBoxState()
{
	if( m_pStartPosition )
		delete m_pStartPosition;

	m_pStartPosition = nullptr;

	if( m_pSelectBox )
		delete m_pSelectBox;

	m_pSelectBox = nullptr;
}

void SelectBoxState::Enter(const Point &point)
{
	m_pStartPosition = new Point(point.x, point.y);
	
	m_pSelectBox->SetElementPosition( Point3D(point.x, point.y, 0.0F) );
	TheInterfaceMgr->AddElement(m_pSelectBox);

	TheInterfaceMgr->SetTrackElement(m_pSelectBox);
	TheInterfaceMgr->HideCursor();

	// disable the Commander's camera movement
	GameWorld *gameWorld = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );

	if( gameWorld )
		gameWorld->GetCommanderCamera().SetEnabled( false );
}

void SelectBoxState::Hover(const Point &point)
{
	if( m_pStartPosition == nullptr )
		return;

	float width = point.x - m_pStartPosition->x;
	float height = point.y - m_pStartPosition->y;

	m_pSelectBox->SetElementSize( width, height );

	m_pSelectBox->Invalidate();
}

void SelectBoxState::MouseUp(const Point &point)
{
	TheBTCursor->PurgeSelectedUnits();
	TheInfoPaneMgr->ShowInfoPane( kInfoPaneNone );

	SelectEntities();

	// enable the Commander camera's movement
	GameWorld *gameWorld = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );

	if( gameWorld )
		gameWorld->GetCommanderCamera().SetEnabled( true );


	TheBTCursor->ChangeState(TheDefaultState, point);
}

void SelectBoxState::Exit(const Point &point)
{
	TheInterfaceMgr->RemoveElement(m_pSelectBox);
	m_pSelectBox->SetElementSize(0.0F, 0.0F);

	if( m_pStartPosition ) {

		delete m_pStartPosition;
		m_pStartPosition = nullptr;
	}

	TheInterfaceMgr->SetTrackElement(nullptr);
}

void SelectBoxState::SelectEntities( void )
{
	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( player == nullptr ) return;

	long playerTeam = player->GetPlayerTeam();
	long otherTeam = playerTeam == 1 ? 2 : 1;

	Array<long> selectedEntities;

	// First check to see if we can select any units
	GetEntitiesInSelectBox( kCacheUnits, playerTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->AddSelectedUnits( selectedEntities );

		TheUnitIconMgr->PurgeUnits();
		TheUnitIconMgr->AddUnits( TheBTCursor->GetSelectedUnits() );
		return;
	}

	// No group of units to select, instead, just go through the caches and find a single entity to select
	GetEntitiesInSelectBox( kCacheCommanders, playerTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->ShowEntityInformation( selectedEntities[0] );
		return;
	}

	GetEntitiesInSelectBox( kCacheBuildings, playerTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->ShowEntityInformation( selectedEntities[0] );
		return;
	}

	GetEntitiesInSelectBox( kCacheBases, playerTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->ShowEntityInformation( selectedEntities[0] );
		return;
	}

	// Now check the other team
	GetEntitiesInSelectBox( kCacheUnits, otherTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->ShowEntityInformation( selectedEntities[0] );
		return;
	}

	GetEntitiesInSelectBox( kCacheCommanders, otherTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->ShowEntityInformation( selectedEntities[0] );
		return;
	}

	GetEntitiesInSelectBox( kCacheBuildings, otherTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->ShowEntityInformation( selectedEntities[0] );
		return;
	}

	GetEntitiesInSelectBox( kCacheBases, otherTeam, selectedEntities );
	if( selectedEntities.GetElementCount() > 0 ) 
	{
		TheBTCursor->ShowEntityInformation( selectedEntities[0] );
		return;
	}
}

void SelectBoxState::GetEntitiesInSelectBox( CacheType cache, long team, Array<long>& entities )
{
	if( TheGameCacheMgr == nullptr ) return;

	Array<long> *entityCache = TheGameCacheMgr->GetGameCache( cache, team );
	if( entityCache == nullptr ) return;

	long cacheSize = entityCache->GetElementCount();
	const Point mousePos = TheInterfaceMgr->GetMousePosition();

	for(long i=0; i < cacheSize; i++ ) {

		long controllerIndex = (*entityCache)[i];

		BTEntityController *controller = static_cast<BTEntityController *>( TheWorldMgr->GetWorld()->GetController(controllerIndex) );
		if( controller == nullptr ) continue;

		Point3D entityPos = controller->GetPosition();

		Point3D boxStart = TheBTCursor->GetCameraRayBisectionWithHeight(*m_pStartPosition, entityPos.z);
		Point3D boxEnd = TheBTCursor->GetCameraRayBisectionWithHeight(mousePos, entityPos.z);

		float top = Fmin( boxStart.y, boxEnd.y );
		float bottom = Fmax( boxStart.y, boxEnd.y );
		float right = Fmax( boxStart.x, boxEnd.x );
		float left = Fmin( boxStart.x, boxEnd.x );

		if( top <= entityPos.y && bottom >= entityPos.y && right >= entityPos.x && left <= entityPos.x ) {
			
			entities.AddElement( controllerIndex );
		}
	}
}

/* ------------------ BTCursor ------------------- */

BTCursor::BTCursor() : Singleton(TheBTCursor),
					mouseEventHandler(&HandleMouseEvent, this),
					m_CurrentOrder(kOrderNone),
					m_pCurrentState(nullptr)

{
	TheEngine->InstallMouseEventHandler(&mouseEventHandler);
}

BTCursor::~BTCursor()
{

}

void BTCursor::HandleMouseEvent(EventType eventType, const Point& point, void *data)
{
	BTCursor *cursor = static_cast<BTCursor*>(data);
	if( cursor ) cursor->BTMouseEvent(eventType, point);
}

void BTCursor::BTMouseEvent(EventType eventType, const Point& point)
{
	if( m_pCurrentState == nullptr )
		return;

	if( eventType == kEventMouseDown ) 
		m_pCurrentState->MouseDown(point);

	else if( eventType == kEventMouseUp ) 
		m_pCurrentState->MouseUp(point);

	else if( eventType == kEventMiddleMouseDown )
		m_pCurrentState->MiddleMouseDown(point);

	else if( eventType == kEventMiddleMouseUp )
		m_pCurrentState->MiddleMouseUp(point);

	else if( eventType == kEventRightMouseDown ) 
		m_pCurrentState->RightMouseDown(point);

	else if( eventType == kEventRightMouseUp ) 
		m_pCurrentState->RightMouseUp(point);
}

void BTCursor::ChangeState(MouseState *newState, const Point &point)
{
	Point pointCopy = point;

	if( m_pCurrentState )
		m_pCurrentState->Exit(point);

		m_pCurrentState = newState;

	if( newState ) 
		m_pCurrentState->Enter(pointCopy);
}

void BTCursor::AddSelectedUnit( BTEntityController& entity )
{
	m_arrSelectedUnits.AddElement( entity.GetControllerIndex() );

	entity.SetEntityFlags( entity.GetEntityFlags() | kEntitySelected );
}

void BTCursor::AddSelectedUnits( Array<long>& units )
{
	World *world = TheWorldMgr->GetWorld();
	if( world )
	{
		long size = units.GetElementCount();
		for( long x = 0; x < size; x++ )
		{
			BTEntityController *cont = static_cast<BTEntityController *>( world->GetController( units[x] ) );
			if( cont && cont->GetBaseControllerType() == kControllerEntity )
			{
				m_arrSelectedUnits.AddElement( units[x] );

				cont->SetEntityFlags( cont->GetEntityFlags() | kEntitySelected );
			}
		}
	}
}

void BTCursor::RemoveSelectedUnit(long lControllerIndex)
{
	long index = m_arrSelectedUnits.FindElement( lControllerIndex );

	if( index != -1 ) 
		m_arrSelectedUnits.RemoveElement( index );

	World *world = TheWorldMgr->GetWorld();
	if( world )
	{
		BTEntityController *cont = static_cast<BTEntityController *>( world->GetController( lControllerIndex ) );
		if( cont )
			cont->SetSelected( false );
	}
}

const Array<long> &BTCursor::GetSelectedUnits(void)
{
	return m_arrSelectedUnits;
}

void BTCursor::PurgeSelectedUnits(void)
{
	long size = m_arrSelectedUnits.GetElementCount();
	for( long x = 0; x < size; x++ ) {

		World *world = TheWorldMgr->GetWorld();
		if( world )
		{
			BTEntityController *cont = static_cast<BTEntityController *>( world->GetController( m_arrSelectedUnits[x] ) );
			if( cont )
				cont->SetSelected( false );
		}
	}

	m_arrSelectedUnits.Purge();

	TheUnitIconMgr->SetGroup( "" );
}

void BTCursor::SelectAllUnits(void)
{
	World *world = TheWorldMgr->GetWorld();
	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( player && world )
	{
		if( TheGameCacheMgr )
		{
			Array<long> *units = TheGameCacheMgr->GetGameCache( kCacheUnits, player->GetPlayerTeam() );
			if( units )
			{
				long size = units->GetElementCount();
				for( long x = 0; x < size; x++ ) {

					BTEntityController *cont = static_cast<BTEntityController *>( world->GetController( (*units)[x] ) );
					if( cont ) {

						m_arrSelectedUnits.AddElement( cont->GetControllerIndex() );
						cont->SetSelected( true );
					}
				}
			}
		}
	}
}

void BTCursor::SelectGroup( UnitGroups groupNumber )
{
	World *world = TheWorldMgr->GetWorld();
	if( world == nullptr ) return;

	Array<long> *group;

	if( groupNumber == kUnitGroup1 )
		group = &m_arrUnitGroup1;
	else if( groupNumber == kUnitGroup2 )
		group = &m_arrUnitGroup2;
	else if( groupNumber == kUnitGroup3 )
		group = &m_arrUnitGroup3;
	else
		group = &m_arrUnitsFieldMode;

	PurgeSelectedUnits();


	long size = group->GetElementCount();
	for( long x = 0; x < size; x++ )
	{
		BTEntityController *cont = static_cast<BTEntityController *>( world->GetController( (*group)[x] ) );
		if( cont && cont->GetBaseControllerType() == kControllerEntity )
		{
			cont->SetSelected( true );
			m_arrSelectedUnits.AddElement( (*group)[x] );
		}
		else
		{
			group->RemoveElement( x );
		}
	}

	TheUnitIconMgr->PurgeUnits();
	TheUnitIconMgr->AddUnits( m_arrSelectedUnits );

	if( groupNumber != kUnitFieldMode ) {

		TheUnitIconMgr->SetGroup( String<>((long) groupNumber) );
	}
}

void BTCursor::GroupSelectedUnits( UnitGroups groupNumber )
{
	Array<long> *group;

	if( groupNumber == kUnitGroup1 )
		group = &m_arrUnitGroup1;
	else if( groupNumber == kUnitGroup2 )
		group = &m_arrUnitGroup2;
	else if( groupNumber == kUnitGroup3 )
		group = &m_arrUnitGroup3;
	else
		group = &m_arrUnitsFieldMode;

	group->Purge();

	long size = m_arrSelectedUnits.GetElementCount();
	for( long x = 0; x < size; x++ )
	{
		group->AddElement( m_arrSelectedUnits[x] );
	}

	if( groupNumber != kUnitFieldMode ) {

		TheUnitIconMgr->SetGroup( String<>((long) groupNumber) );
	}
}

void BTCursor::SetOrder(UnitOrderType Order) 
{ 
	if( Order == kOrderStop ) {

		long numElements = m_arrSelectedUnits.GetElementCount();
		if( numElements <= 0 ) 
			return;

		Array<UnitCommand> *commands = new Array<UnitCommand>( numElements );

		int commandsSent = 0;

		for( long i = 0; i < numElements; i++ ) {

			BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(m_arrSelectedUnits[i]) );
			if( controller == nullptr ) continue;

			UnitCommand command;
			command.m_lUnit = m_arrSelectedUnits[i];
			command.m_lTarget = m_arrSelectedUnits[i];

			commands->AddElement( command );
			commandsSent++;

			if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
				TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandStop, *commands) );
				commandsSent = 0;
				commands->Purge();
			}
		}

		if( commands > 0 ) {
			TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandStop, *commands) );
		}

		// setup next units reminder
		TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

		m_CurrentOrder = kOrderNone;
		return;
	}

	m_CurrentOrder = Order; 
}

void BTCursor::GiveOrder( Point3D& p3dWorldPosition )
{
	TheGame->PlayUISound( "sound/ui", 1.0F, 1 );

	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	UnitOrderType unitOrder = TheBTCursor->GetOrder();

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	const Array<long> &units = TheBTCursor->GetSelectedUnits();
	long numElements = units.GetElementCount();

	if( numElements <= 0 )
		return;

	Array<UnitCommand> *commands = new Array<UnitCommand>( numElements );

	if( unitOrder == kOrderNone ) {

		// First check if we are right clicking on an entity, if so try to attack it
		BTCollisionData data;
		Array<long> ignoreConts;
		BTEntityController *target;
		long targetIndex;

		TheGame->DetectEntity( p3dWorldPosition, data, kCacheAll, 0.1F, ignoreConts, false );
		target = data.m_pController;

		if( target && target->GetBaseControllerType() == kControllerEntity ) {

			// If a plot was selected, try to get the building instead.
			if( target->GetControllerType() == kControllerPlot ) {
				PlotController *pc = static_cast<PlotController*>( target );
				if( !pc ) {
					delete commands;
					return;
				}

				if( pc->GetBuilding() == nullptr || pc->GetBuilding()->GetControllerType() != kControllerBuilding ) {
					delete commands;
					return;
				}

				target = pc->GetBuilding();
			}

			BTEntityController *cont = static_cast<BTEntityController *>( target );
			GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
			if( cont->GetTeam() != player->GetPlayerTeam() ) {

				targetIndex = target->GetControllerIndex();

				int commandsSent = 0;

				for( long i = 0; i < numElements; i++ ) {

					BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
					if( controller == nullptr ) continue;

					UnitCommand command;
					command.m_lUnit = units[i];
					command.m_lTarget = targetIndex;

					commands->AddElement( command );
					commandsSent++;

					if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
						TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttack, *commands) );
						commandsSent = 0;
						commands->Purge();
					}
				}

				if( commands > 0 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttack, *commands) );
				}

				// setup next units reminder
				TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

				// Show visual
				Point3D matPoint = p3dWorldPosition;
				matPoint.z = target->GetPosition().z;
				Zone *zone = world->FindZone( matPoint );
				if( zone ) {
					MaterializeSystem *system = new MaterializeSystem( ColorRGB(1.0F, 0.0F, 0.0F), 0.5F );
					system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
					zone->AddNewSubnode( system );
				}
			}

		} else {

			float avgX = 0.0F;
			float avgY = 0.0F;
			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				avgX += controller->GetPosition().x;
				avgY += controller->GetPosition().y;
			}
			avgX /= numElements;
			avgY /= numElements;
			Point3D avg( avgX, avgY, 0.0F );

			int commandsSent = 0;

			float averageHeight = 0.0F;

			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				BTCylinderCollider *col = static_cast<BTCylinderCollider*>( controller->GetCollider() );
				if( !col ) {
					continue;
				}

				averageHeight += controller->GetFloatHeight();

				GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
				if( player->GetPlayerTeam() != controller->GetTeam() ) continue;

				Vector3D offset = controller->GetPosition() - avg;
				offset *= AVERAGING_OFFSET_MULT;

				Point3D tar = Point3D(p3dWorldPosition.x, p3dWorldPosition.y, controller->GetFloatHeight()) + offset;
				if( !world->PointWithinMapBounds(tar) ) {
					Array<Point3D> safe_points;
					PointFinder::FindSafePoint_Spiral( tar, 1, safe_points, 1.0F, 1.0F, K::pi_over_6, 10.0F, col->GetRadius() * UNIT_SAFE_DIST_MULT, 0.0F, false );

					if( safe_points.GetElementCount() < 1 ) {
						continue;
					}

					tar = safe_points[0];
				}

				tar.z = controller->GetPosition().z;

				UnitCommand command;
				command.m_lUnit = units[i];
				command.m_p3dTarget = tar;

				commands->AddElement( command );
				commandsSent++;

				if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandMove, *commands) );
					commandsSent = 0;
					commands->Purge();
				}
			}

			if( commands > 0 ) {
				TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandMove, *commands) );
			}

			// setup next units reminder
			TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

			// Show visual
			Point3D matPoint = p3dWorldPosition;
			matPoint.z = (averageHeight / numElements);
			Zone *zone = world->FindZone( matPoint );
			if( zone ) {
				MaterializeSystem *system = new MaterializeSystem( ColorRGB(0.0F, 1.0F, 0.0F), 0.5F );
				system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
				zone->AddNewSubnode( system );
			}
		}
	}
	else {

		if( unitOrder == kOrderAttackTo ) {

			float avgX = 0.0F;
			float avgY = 0.0F;
			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				avgX += controller->GetPosition().x;
				avgY += controller->GetPosition().y;
			}
			avgX /= numElements;
			avgY /= numElements;
			Point3D avg( avgX, avgY, 0.0F );

			int commandsSent = 0;

			float averageHeight = 0.0F;

			for( long i = 0; i < numElements; i++ ) {

				BTCharacterController *controller = static_cast<BTCharacterController *>( TheWorldMgr->GetWorld()->GetController(units[i]) );
				if( !controller ) {
					continue;
				}

				BTCylinderCollider *col = static_cast<BTCylinderCollider*>( controller->GetCollider() );
				if( !col ) {
					continue;
				}

				averageHeight += controller->GetFloatHeight();

				GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
				if( player->GetPlayerTeam() != controller->GetTeam() ) continue;

				Vector3D offset = controller->GetPosition() - avg;
				offset *= AVERAGING_OFFSET_MULT;

				Point3D tar = Point3D(p3dWorldPosition.x, p3dWorldPosition.y, controller->GetFloatHeight()) + offset;
				if( !world->PointWithinMapBounds(tar) ) {
					Array<Point3D> safe_points;
					PointFinder::FindSafePoint_Spiral( tar, 1, safe_points, 1.0F, 1.0F, K::pi_over_6, 10.0F, col->GetRadius() * UNIT_SAFE_DIST_MULT, 0.0F, false );

					if( safe_points.GetElementCount() < 1 ) {
						continue;
					}

					tar = safe_points[0];
				}

				tar.z = controller->GetPosition().z;

				UnitCommand command;
				command.m_lUnit = units[i];
				command.m_p3dTarget = tar;

				commands->AddElement( command );
				commandsSent++;

				if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
					TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttackTo, *commands) );
					commandsSent = 0;
					commands->Purge();
				}
			}

			if( commands > 0 ) {
				TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandAttackTo, *commands) );
			}

			// setup next units reminder
			TheGame->SetNextUnitsRemTime( TheTimeMgr->GetAbsoluteTime() + UNITS_REMINDER_INTERVAL );

			// Show visual
			Point3D matPoint = p3dWorldPosition;
			matPoint.z = (averageHeight / numElements);
			Zone *zone = world->FindZone( matPoint );
			if( zone ) {
				MaterializeSystem *system = new MaterializeSystem( ColorRGB(1.0F, 0.0F, 0.0F), 0.5F );
				system->SetNodePosition( Point3D(matPoint.x, matPoint.y, matPoint.z + 1.0F) );
				zone->AddNewSubnode( system );
			}
		}

		TheBTCursor->SetOrder(kOrderNone);
	}

	delete commands;
}

void BTCursor::ShowEntityInformation( long lControllerIndex )
{
	World *world = TheWorldMgr->GetWorld();
	if( world == nullptr ) return;

	BTEntityController *entity = static_cast<BTEntityController *>( world->GetController( lControllerIndex ) );
	if( (entity == nullptr) || (entity->GetBaseControllerType() != kControllerEntity) ) return;

	if( entity->GetControllerType() == kControllerUnit ) {

		UnitController *unit = static_cast<UnitController *>( entity );
		TheBTCursor->AddSelectedUnit( *entity );

		// Update UI
		UnitInfoPane *pane = static_cast<UnitInfoPane *>( &TheInfoPaneMgr->GetInfoPane( kInfoPaneUnit ) );
		pane->SetUnit( unit );
		TheInfoPaneMgr->ShowInfoPane( kInfoPaneUnit );
	}

	else if( entity->GetControllerType() == kControllerCommander ) {

		CommanderController *commander = static_cast<CommanderController *>( entity );

		// Update UI
		CommanderInfoPane *pane = static_cast<CommanderInfoPane *>( &TheInfoPaneMgr->GetInfoPane( kInfoPaneCommander ) );
		pane->SetCommander( commander );

		TheInfoPaneMgr->ShowInfoPane( kInfoPaneCommander );
	}

	else if( entity->GetControllerType() == kControllerBase ) {

		BaseController *base = static_cast<BaseController *>( entity );

		// Update UI
		BaseInfoPane *pane = static_cast<BaseInfoPane *>( &TheInfoPaneMgr->GetInfoPane( kInfoPaneBase ) );
		pane->SetBase( base );
		TheInfoPaneMgr->ShowInfoPane( kInfoPaneBase );
	}

	else if( (entity->GetControllerType() == kControllerBuilding) || (entity->GetControllerType() == kControllerPlot) ) {

		// if clicked on a plot and there is a building on the plot, just click on the building
		if( entity->GetControllerType() == kControllerPlot )
		{
			PlotController *plot = static_cast<PlotController *>( entity );
			entity = plot->GetBuilding();

			if( (entity == nullptr) || (entity->GetControllerType() != kControllerBuilding) ) {

				entity = nullptr;
			}
		}

		if( entity )
		{
			BuildingController *building = static_cast<BuildingController *>( entity );

			// Update UI
			BuildingInfoPane *pane = static_cast<BuildingInfoPane *>( &TheInfoPaneMgr->GetInfoPane( kInfoPaneBuilding ) );
			pane->SetBuilding( building );
			TheInfoPaneMgr->ShowInfoPane( kInfoPaneBuilding );
		}
	}
}

Ray BTCursor::WorldRayFromMousePosition(const Point &point)
{
	FrustumCamera *currentCamera = TheWorldMgr->GetWorld()->GetCamera();

	Ray cameraRay;

	float x; // cursor x-position as a percentange of display width [0..1]
	float y; // cursor y-position as a percentange of display height [0..1]

	x = point.x / (float)TheDisplayMgr->GetDisplayWidth();
	y = point.y / (float)TheDisplayMgr->GetDisplayHeight();

	// x=0 y=0 is the top-left of the screen
	// x=1 y=1 is the bottom-right of the screen
 	currentCamera->CastRay(x,y,&cameraRay);

	// the origin of the ray is relative to the camera, change it to the camera's origin
	cameraRay.origin = currentCamera->GetNodePosition();

	// direction is also relative to the camera
	cameraRay.direction = currentCamera->GetNodeTransform() * cameraRay.direction;
	cameraRay.direction = cameraRay.direction.Normalize();

	// fill in the rest of the ray structure
	cameraRay.radius = QUERY_RADIUS;
	cameraRay.tmin = 0.0F;
	cameraRay.tmax = MAX_RAY_LENGTH;

	return cameraRay;
}

BTEntityController *BTCursor::GetEntityFromMousePosition(const Point &point)
{
	Model *returnModel = nullptr;
	GameWorld *gameWorld;
	FrustumCamera *camera;

	if( (gameWorld = static_cast<GameWorld *>(TheWorldMgr->GetWorld())) == nullptr )
		return nullptr;

	if( (camera = gameWorld->GetCamera()) == nullptr )
		return nullptr;

	Ray cameraRay = WorldRayFromMousePosition(point);

	BTCollisionData data;
	Array<long> ignoreConts;

	TheGame->DetectEntity( cameraRay, data, kCacheAll, ignoreConts );

	return data.m_pController;
}

Point3D BTCursor::GetCameraRayBisectionWithWorld(const Point &point)
{
	GameWorld *gameWorld = static_cast<GameWorld *>(TheWorldMgr->GetWorld());

	Ray cameraRay = WorldRayFromMousePosition(point);

	Point3D pStart = cameraRay.origin + (cameraRay.direction * cameraRay.tmin);
	Point3D pEnd = cameraRay.origin + (cameraRay.direction * cameraRay.tmax);

	CollisionData collisionData;

	bool collision = gameWorld->DetectCollision(pStart, pEnd, 0.0, 0, &collisionData);
	
	if( collision ) {

		pEnd = collisionData.position;
	}

	return pEnd;
}

Point3D BTCursor::GetCameraRayBisectionWithHeight(const Point &point, float height)
{
	GameWorld *gameWorld = static_cast<GameWorld *>(TheWorldMgr->GetWorld());

	Ray cameraRay = WorldRayFromMousePosition(point);

	float magnitude = ( height - cameraRay.origin.z ) / ( cameraRay.direction.z );
	Point3D pEnd = cameraRay.origin + (cameraRay.direction * magnitude);
	
	return pEnd;
}

bool BTCursor::CursorCollidesWithElement(void)
{
	const Point point = TheInterfaceMgr->GetMousePosition();
	Point3D point3D(point.x, point.y, 0.0F);

	if( TheInterfaceMgr->TestCursorCollision(TheInterfaceMgr->GetRootElement(), point3D, 0) == true )
		return true;

	if( TheInterfaceMgr->TestCursorCollision(TheInterfaceMgr->GetWindowRoot(), point3D, 0) == true )
		return true;

	return false;
}

void BTCursor::InterfaceTask(void)
{
	if( m_pCurrentState )
		m_pCurrentState->Hover(TheInterfaceMgr->GetMousePosition());
}
