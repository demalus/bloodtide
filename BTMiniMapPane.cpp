
#include "BTMiniMapPane.h"
#include "MGGame.h"
#include "BTHUD.h"
#include "BTCursor.h"
#include "BTMath.h"

using namespace C4;


MiniMapPane::MiniMapPane() : Interface()
{
	// get the map size, set the minimap size
	Marker *pMapBounds = static_cast<GameWorld*>(TheWorldMgr->GetWorld())->GetMapBounds();

	if(!pMapBounds){
		m_lGameMapWidth = 0;
		m_lGameMapHeight = 0;
	} else {
		MapBoundsController *cont = static_cast<MapBoundsController*>( pMapBounds->GetController() );
		m_lGameMapWidth = cont->GetTopRightVector().x;
		m_lGameMapHeight = cont->GetTopRightVector().y;
		float x = pMapBounds->GetNodePosition().x;
		float y = pMapBounds->GetNodePosition().y;
	}

	AddSubnode( &m_objBackground );
	AddSubnode( &m_objCameraBox );

	m_bMouseDown = false;
}

void MiniMapPane::CreateBasesArray(long team)
{
	Array<long> *baseCache = TheGameCacheMgr->GetGameCache(kCacheBases, team);
	
	for(int i = 0; i < baseCache->GetElementCount(); i++)
	{
		BaseController *cont = static_cast<BaseController *>( TheWorldMgr->GetWorld()->GetController( (*baseCache)[i] ) );

		if( cont )
		{
			// Get position relative to the mapbound marker instead of world origin
			Marker *pMapBounds = static_cast<GameWorld*>(TheWorldMgr->GetWorld())->GetMapBounds();
			Point3D p1 = cont->GetTargetNode()->GetWorldPosition();
			Point3D p2 = pMapBounds->GetController()->GetTargetNode()->GetWorldPosition();
			Point2D position = Point2D(Sqrt(Pow(p1.x - p2.x, 2)), Sqrt(Pow(p1.y - p2.y, 2)));
			
			if(team == static_cast<GamePlayer*>(TheMessageMgr->GetLocalPlayer())->GetPlayerTeam())
			{
				// own team - green
				QuadElement *q = new QuadElement(BASE_SIZE, BASE_SIZE, (K::green));

				float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
				float y = (m_objBackground.GetElementHeight() - BUILDING_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

				Point3D miniPosition(x, y, 0.0F);

				q->SetElementPosition(miniPosition);

				m_arrMyBases.AddElement(q);

				AddSubnode( q );

			} else {	
				// enemy team - red
				QuadElement *q = new QuadElement(BASE_SIZE, BASE_SIZE, (K::red));

				float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
				float y = (m_objBackground.GetElementHeight() - BUILDING_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

				Point3D miniPosition(x, y, 0.0F);

				q->SetElementPosition(miniPosition);

				m_arrEnemyBases.AddElement(q);

				AddSubnode( q );
			}
		}
	}
}

void MiniMapPane::CreateBuildingsArray(long team)
{
	Array<long> *plotCache = TheGameCacheMgr->GetGameCache(kCachePlots, team);
	
	for(int i = 0; i < plotCache->GetElementCount(); i++)
	{
		PlotController *cont = static_cast<PlotController *>( TheWorldMgr->GetWorld()->GetController( (*plotCache)[i] ) );

		if( cont )
		{
			if(cont->GetBuilding())
			{
				// Get position relative to the mapbound marker instead of world origin
				Marker *pMapBounds = static_cast<GameWorld*>(TheWorldMgr->GetWorld())->GetMapBounds();
				Point3D p1 = cont->GetTargetNode()->GetWorldPosition();
				Point3D p2 = pMapBounds->GetController()->GetTargetNode()->GetWorldPosition();
				Point2D position = Point2D(Sqrt(Pow(p1.x - p2.x, 2)), Sqrt(Pow(p1.y - p2.y, 2)));

				if(team == static_cast<GamePlayer*>(TheMessageMgr->GetLocalPlayer())->GetPlayerTeam())
				{
					// own team - green
					QuadElement *q = new QuadElement(BUILDING_SIZE, BUILDING_SIZE, (K::green));

					float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
					float y = (m_objBackground.GetElementHeight() - BUILDING_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

					Point3D miniPosition(x, y, 0.0F);

					q->SetElementPosition(miniPosition);

					m_arrMyBuildings.AddElement(q);

					AddSubnode( q );

				} else {	
					// enemy team - red
					QuadElement *q = new QuadElement(BUILDING_SIZE, BUILDING_SIZE, (K::red));

					float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
					float y = (m_objBackground.GetElementHeight() - BUILDING_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

					Point3D miniPosition(x, y, 0.0F);

					q->SetElementPosition(miniPosition);

					m_arrEnemyBuildings.AddElement(q);

					AddSubnode( q );
				}
			}
		}
	}
}

void MiniMapPane::CreateUnitsArray(long team)
{
	Array<long> *unitCache = TheGameCacheMgr->GetGameCache(kCacheUnits, team);
	
	for(int i = 0; i < unitCache->GetElementCount(); i++)
	{
		UnitController *cont = static_cast<UnitController *>( TheWorldMgr->GetWorld()->GetController( (*unitCache)[i] ) );

		if( cont )
		{
			// Get position relative to the mapbound marker instead of world origin
			Marker *pMapBounds = static_cast<GameWorld*>(TheWorldMgr->GetWorld())->GetMapBounds();
			Point3D p1 = cont->GetTargetNode()->GetWorldPosition();
			Point3D p2 = pMapBounds->GetController()->GetTargetNode()->GetWorldPosition();
			Point2D position = Point2D(Sqrt(Pow(p1.x - p2.x, 2)), Sqrt(Pow(p1.y - p2.y, 2)));

			if(team == static_cast<GamePlayer*>(TheMessageMgr->GetLocalPlayer())->GetPlayerTeam())
			{
				// own team - green
				QuadElement *q = new QuadElement(UNIT_SIZE, UNIT_SIZE, (K::green));

				float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
				float y = (m_objBackground.GetElementHeight() - UNIT_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

				Point3D miniPosition(x, y, 0.0F);

				q->SetElementPosition(miniPosition);

				m_arrMyUnits.AddElement(q);

				AddSubnode( q );

			} else {	
				// enemy team - red
				QuadElement *q = new QuadElement(UNIT_SIZE, UNIT_SIZE, (K::red));

				float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
				float y = (m_objBackground.GetElementHeight() - UNIT_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

				Point3D miniPosition(x, y, 0.0F);

				q->SetElementPosition(miniPosition);

				m_arrEnemyUnits.AddElement(q);

				AddSubnode( q );
			}
		}
	}
}

void MiniMapPane::CreateCommandersArray(long team)
{
	GamePlayer *player = static_cast<GamePlayer*>( TheMessageMgr->GetLocalPlayer() );
	if( !player ) {
		return;
	}

	Array<long> *commanderCache = TheGameCacheMgr->GetGameCache(kCacheCommanders, team);
	
	for(int i = 0; i < commanderCache->GetElementCount(); i++)
	{
		CommanderController *cont = static_cast<CommanderController *>( TheWorldMgr->GetWorld()->GetController( (*commanderCache)[i] ) );

		if( cont )
		{
			if( player->GetPlayerTeam() != cont->GetTeam() ) {
				if( cont->GetEntityFlags() & kEntityInvisible ) {
					continue;
				}
			}

			// Get position relative to the mapbound marker instead of world origin
			Marker *pMapBounds = static_cast<GameWorld*>(TheWorldMgr->GetWorld())->GetMapBounds();
			Point3D p1 = cont->GetTargetNode()->GetWorldPosition();
			Point3D p2 = pMapBounds->GetController()->GetTargetNode()->GetWorldPosition();
			Point2D position = Point2D(Sqrt(Pow(p1.x - p2.x, 2)), Sqrt(Pow(p1.y - p2.y, 2)));

			if(team == static_cast<GamePlayer*>(TheMessageMgr->GetLocalPlayer())->GetPlayerTeam())
			{
				// own team - green
				QuadElement *q = new QuadElement(COMMANDER_SIZE, COMMANDER_SIZE, (K::cyan));

				float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
				float y = (m_objBackground.GetElementHeight() - COMMANDER_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

				Point3D miniPosition(x, y, 0.0F);

				q->SetElementPosition(miniPosition);

				m_arrMyCommanders.AddElement(q);

				AddSubnode( q );

			} else {	
				// enemy team - red
				QuadElement *q = new QuadElement(COMMANDER_SIZE, COMMANDER_SIZE, (K::magenta));

				float x = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
				float y = (m_objBackground.GetElementHeight() - COMMANDER_SIZE) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());

				Point3D miniPosition(x, y, 0.0F);

				q->SetElementPosition(miniPosition);

				m_arrEnemyCommanders.AddElement(q);

				AddSubnode( q );				
			}
		}
	}

}

MiniMapPane::~MiniMapPane()
{
	Cleanup();
}

void MiniMapPane::UpdateMiniMap()
{
	Cleanup();

	RemoveSubtree();
	AddSubnode( &m_objBackground );
	
	SetMiniCam();

	// call once for each team
	for(int i = 1; i <= 2; i++)
	{
		CreateBasesArray(i);
		CreateBuildingsArray(i);
		CreateUnitsArray(i);
		CreateCommandersArray(i);
	}
}

void MiniMapPane::Cleanup( void )
{
	CleanArray(m_arrMyBases);
	CleanArray(m_arrMyBuildings);
	CleanArray(m_arrMyUnits);
	CleanArray(m_arrMyCommanders);

	CleanArray(m_arrEnemyBases);
	CleanArray(m_arrEnemyBuildings);
	CleanArray(m_arrEnemyUnits);
	CleanArray(m_arrEnemyCommanders);
}

void MiniMapPane::CleanArray(Array<QuadElement*> &arr)
{
	for(int i = 0; i < arr.GetElementCount(); i++)
	{
		delete arr[i];
	}

	arr.Purge();
}

/**
*  Creates a rectangle to draw on the minimap representing the player's camera (what is in sight)
*/ 
void MiniMapPane::SetMiniCam()
{	
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if ( world && world->GetInCommandMode() )
	{
		Point3D topleft = TheBTCursor->GetCameraRayBisectionWithHeight(Point(0.0, 0.0), 0.0F);
		Point3D topright = TheBTCursor->GetCameraRayBisectionWithHeight(Point(TheDisplayMgr->GetDisplayWidth(), 0.0F), 0.0F);
		Point3D botleft = TheBTCursor->GetCameraRayBisectionWithHeight(Point(0.0, TheDisplayMgr->GetDisplayHeight()), 0.0F);
		Point3D botright = TheBTCursor->GetCameraRayBisectionWithHeight(Point(TheDisplayMgr->GetDisplayWidth(), TheDisplayMgr->GetDisplayHeight()), 0.0F);
		
		Marker *pMapBounds = static_cast<GameWorld*>(TheWorldMgr->GetWorld())->GetMapBounds();
		Point3D p1 = TheBTCursor->GetCameraRayBisectionWithHeight(Point(botleft.x, botleft.y), 0.0F);
		Point3D p2 = pMapBounds->GetController()->GetTargetNode()->GetWorldPosition();
		Point2D position = Point2D(Sqrt(Pow(p1.x - p2.x, 2)), Sqrt(Pow(p1.y - p2.y, 2)));

		float camx = (position.x / m_lGameMapWidth) * m_objBackground.GetElementWidth();
		float camy = (m_objBackground.GetElementHeight()) - ((position.y / m_lGameMapHeight) * m_objBackground.GetElementHeight());
		Point3D miniPosition(camx, camy, 0.0F);

		m_objCameraBox.SetElementSize(Calculate2DDistance(topleft, topright), Calculate2DDistance(topleft, botleft));
		m_objCameraBox.SetElementPosition(miniPosition);
		m_objCameraBox.SetQuadColor(ColorRGBA(0.0F, 0.0F, 0.0F, 0.35F));

		AddSubnode( &m_objCameraBox );
	}
}

// called upon creation
void MiniMapPane::UpdateDisplayPosition()
{
	// set background size to the pane size, and set color
	float x = this->GetElementWidth();
	float y = this->GetElementHeight();

	m_objBackground.SetElementSize(x,y);
	// Sand color
	m_objBackground.SetQuadColor(ColorRGBA( 238.0F/255.0F, 214.0F/255.0F, 175.0F/255.0F, 1.0F ));

	SetMiniCam();
}

void MiniMapPane::InterfaceTask()
{
	UpdateMiniMap();
}

PartCode MiniMapPane::TestPickPoint(const Point3D& p) const
{
	return (kElementPartButton);
}

void MiniMapPane::HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p)
{
	GameWorld *world = static_cast<GameWorld *>( TheWorldMgr->GetWorld() );
	if( world == nullptr ) 
		return;

	float percX = p.x / GetElementWidth();
	float percY = p.y / GetElementHeight();

	// cursor has left the mini map
	if( (percX > 1.0F) || (percX < 0.0F) || (percY > 1.0F) || (percY < 0.0F) )
		return;

	Camera *camera = world->GetCamera();
	if( camera == nullptr )
		return;

	Marker *marker = world->GetMapBounds();
	if( marker == nullptr ) 
		return;

	MapBoundsController *mapBounds = static_cast<MapBoundsController *>( marker->GetController() );
	if( mapBounds == nullptr )
		return;

	if( (eventType == kEventMouseDown) || ((eventType == kEventMouseMoved) && (m_bMouseDown)) )
	{
		Point3D worldPos = marker->GetWorldPosition();

		worldPos.x += mapBounds->GetTopRightVector().x * percX;
		worldPos.y += mapBounds->GetTopRightVector().y * (1 - percY);
		worldPos.z = camera->GetNodePosition().z;

		camera->SetNodePosition( worldPos );

		m_bMouseDown = true;
	}

	else if( (eventType == kEventMouseUp) || (eventType == kEventCursorExit) )
	{
		m_bMouseDown = false;
	}

	else if( eventType == kEventRightMouseDown )
	{
		Point3D worldPos = marker->GetWorldPosition();

		worldPos.x += mapBounds->GetTopRightVector().x * percX;
		worldPos.y += mapBounds->GetTopRightVector().y * (1 - percY);
		worldPos.z = 0.0F;


		if( TheBTCursor )
		{
			TheBTCursor->GiveOrder( worldPos );
		}
	}
}

