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

#include "BTBuildingUI.h"
#include "StringTableManager.h"
#include "MGMultiplayer.h"

#define BM_ITEM_WIDTH 300
#define BM_ITEM_HEIGHT 80

#define BM_ITEM_OUTLINE 1.0F // outline thickness

using namespace C4;

BuildingsMenu *C4::TheBuildingsMenu = nullptr;


/* -------------- Building menu's item -------------- */

BuildingsMenuItem::BuildingsMenuItem(unsigned long ulBuildingID, const char *strBuildingIcon, const char *strToolTip) :
					m_objOutline(0.0F,0.0F,BM_ITEM_OUTLINE),
					m_ulBuildingID( ulBuildingID )
{
	/* ---- Building Icon ---- */

	m_objIcon.SetTexture(strBuildingIcon);
	m_objIcon.SetElementSize(64,64);
	m_objIcon.SetElementPosition( Point3D(BM_ITEM_OUTLINE, BM_ITEM_OUTLINE, 0.0F) );

	AddSubnode(&m_objIcon);

	/* ---- Background ---- */

	m_objBackground.SetQuadColor( ColorRGBA(0.0F, 0.0F, 0.0F, 0.375F) );
	m_objBackground.SetElementSize( BM_ITEM_WIDTH, BM_ITEM_HEIGHT );

	AddSubnode(&m_objBackground);

	/* ---- Outline ---- */

	m_objOutline.SetOutlineColor(K::black);
	m_objOutline.SetElementPosition( Point3D(BM_ITEM_OUTLINE, BM_ITEM_OUTLINE, 0.0F) );
	m_objOutline.SetElementSize(BM_ITEM_WIDTH - 2*BM_ITEM_OUTLINE, BM_ITEM_HEIGHT - 2*BM_ITEM_OUTLINE);

	AddSubnode(&m_objOutline);

	/* ---- Tool Tip ---- */

	m_objToolTip.SetFont( AutoReleaseFont("font/Gui") );
	m_objToolTip.SetText( strToolTip );
	m_objToolTip.SetTextColor( K::white );
	m_objToolTip.SetTextScale( 1.2F );
	m_objToolTip.SetTextFlags( kTextWrapped );

	Point3D pos( m_objIcon.GetElementPosition() );
	pos.x += m_objIcon.GetElementWidth() + 3.0F;
	pos.y = m_objOutline.GetElementPosition().y + 3.0F;

	m_objToolTip.SetElementSize( m_objOutline.GetElementWidth() - pos.x, m_objOutline.GetElementHeight() - 3.0F );
	m_objToolTip.SetElementPosition( pos );

	AddSubnode( &m_objToolTip );
}

void BuildingsMenuItem::Hilite(void)
{
	m_objOutline.SetOutlineColor(K::yellow);
}

void BuildingsMenuItem::Unhilite(void)
{
	m_objOutline.SetOutlineColor(K::black);
}

/* -------------- Buildings Menu -------------- */

BuildingsMenu::BuildingsMenu(long lPlotIndex, const Array<unsigned long> &arrBuildings, const Point &point) : Interface(),
						Singleton<BuildingsMenu>(TheBuildingsMenu),
						m_lPlotIndex( lPlotIndex ),
						m_pCurrentOption(nullptr)
{
	long numBuildings = arrBuildings.GetElementCount();

	const char *strBuildingIcon;
	const char *strToolTip;
	BuildingsMenuItem *menuItem;

	Point3D position(0.0F,0.0F,0.0F);

	for(long i=0; i < numBuildings; i++ ) {

		strBuildingIcon = TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(arrBuildings[i], 'ICON'));
		if( Text::CompareText( strBuildingIcon, "<missing>" ) ) continue;

		strToolTip = TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(arrBuildings[i], 'TTIP'));

		menuItem = new BuildingsMenuItem( arrBuildings[i], strBuildingIcon, strToolTip );

		menuItem->SetElementSize(BM_ITEM_WIDTH, BM_ITEM_HEIGHT);
		menuItem->SetElementPosition(position);

		position.y += BM_ITEM_HEIGHT;

		AddSubnode(menuItem);
		m_arrMenuItems.AddElement( menuItem );
	}

	SetElementSize(BM_ITEM_WIDTH, BM_ITEM_HEIGHT * m_arrMenuItems.GetElementCount());

	// Position the menu so that it fits entirely on the screen
	Point3D menuPosition(point.x, point.y, 0.0F);

	if( point.x + GetElementWidth() > TheDisplayMgr->GetDisplayWidth() )	
		menuPosition.x -= GetElementWidth();

	if( point.y + GetElementHeight() > TheDisplayMgr->GetDisplayHeight() )	
		menuPosition.y -= GetElementHeight();

	SetElementPosition( menuPosition );
}

BuildingsMenu::~BuildingsMenu()
{

}

void BuildingsMenu::New(long lPlotIndex, const Array<unsigned long> &arrBuildings, const Point &point)
{
	if( TheBuildingsMenu == nullptr ) {
		Interface *interface = new BuildingsMenu(lPlotIndex, arrBuildings, point);
		TheInterfaceMgr->AddElement(interface);
		TheInterfaceMgr->SetTrackElement(interface);
	}
}

void BuildingsMenu::HandleMouseEvent(EventType EventType, PartCode code, const Point3D& p)
{
	long lNumOptions = m_arrMenuItems.GetElementCount();

	if( EventType == kEventRightMouseUp || EventType == kEventMouseMoved ) {

		Point3D background = Point3D(0.0F,0.0F,0.0F);

		// Is the cursor hovering over an option?
		if( p.x > background.x && p.x < background.x + GetElementWidth() &&
			p.y > background.y && p.y < background.y + GetElementHeight() )  {

			int selection = (int) ( p.y / BM_ITEM_HEIGHT );

			if( selection >= 0 && selection < lNumOptions ) { // cursor is hovering over an option

				BuildingsMenuItem *curOption = m_arrMenuItems[selection];

				if( EventType == kEventRightMouseUp) { // execute the option

					//m_iSelected = selection;
					//TriggerElement();
					// TODO: hacky

					TheMessageMgr->SendMessage( kPlayerServer, RequestBuildingMessage( m_lPlotIndex, curOption->m_ulBuildingID ));

					delete this;
					return;
				
				} else { // highlight only the current option
					
					 if( m_pCurrentOption && m_pCurrentOption != curOption ) 
						 m_pCurrentOption->Unhilite();

					m_pCurrentOption = curOption;
					m_pCurrentOption->Hilite();
				}
			}

		} else { // cursor is not hovering over an option

			if( EventType == kEventRightMouseUp ) { // hide the options popup menu

				delete this;
				return;
			}

			if( m_pCurrentOption ) { // stop highlighting the current option

				m_pCurrentOption->Unhilite();
				m_pCurrentOption = nullptr;
			} 
		}
	}

}


