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

#include "BTHUD.h"
#include "MGGame.h"
#include "MGInterface.h"
#include "BTMiniMapPane.h"

using namespace C4;

BTHUD *C4::TheBTHUD	= nullptr;

/* -------------- Blood Tide HUD Pane -------------- */

Pane::Pane()
{
}

Pane::~Pane()
{
	PurgePane();
}

void Pane::InsertElement( Interface& element )
{
	AddSubnode( &element );

	element.SetElementPosition( Point3D( 0.0F, 0.0F, 0.0F ) );//GetElementPosition() );
	element.SetElementSize( GetElementWidth(), GetElementHeight() );

	element.UpdateDisplayPosition();
	element.Show();

	Invalidate();
}

bool Pane::ContainsElement( Interface& element )
{
	return Descendant( &element );
}

void Pane::PurgePane( void )
{
	Element *subnode = GetFirstSubnode();
	while( subnode ) {

		subnode->Detach();

		subnode = GetFirstSubnode();
	}
}

void Pane::UpdateDisplayPosition( void )
{
	Element *subnode = GetFirstSubnode();
	while( subnode ) {

		if( subnode->GetElementType() == kElementInterface ) {

			static_cast<Interface *>( subnode )->UpdateDisplayPosition();
		}

		subnode = subnode->Next();
	}

	Invalidate();
}

/* -------------- Blood Tide HUD -------------- */

BTHUD::BTHUD( const char *paneChat, const char *paneInfo, const char *paneMap ) : 
	Singleton<BTHUD>(TheBTHUD),
	m_pMiniMap( nullptr )
{
	UpdateDisplayPosition();

	m_objPaneChatBackground.SetTexture( paneChat );
	m_objPaneInfoBackground.SetTexture( paneInfo );
	m_objPaneMMapBackground.SetTexture( paneMap );

	AddSubnode( &m_objPaneChatBackground );
	AddSubnode( &m_objPaneInfoBackground );
	AddSubnode( &m_objPaneMMapBackground );

	AddSubnode( &m_objPaneChat );
	AddSubnode( &m_objPaneInfo );
	AddSubnode( &m_objPaneMMap );

	if( TheChatWindow ) {

		// temporary
		float height = TheDisplayMgr->GetDisplayHeight();

		Point3D newPos;

		newPos.y = height - TheChatWindow->GetElementHeight();

		TheChatWindow->SetElementPosition( newPos );
	}

	// Add minimap if there are map boundaries.
	Marker *pMapBounds = static_cast<GameWorld*>(TheWorldMgr->GetWorld())->GetMapBounds();
	if( pMapBounds != nullptr ) {
		if( m_pMiniMap ) {
			delete m_pMiniMap;
		}
		m_pMiniMap = new MiniMapPane();
		m_objPaneMMap.InsertElement( *m_pMiniMap );
	}

}

void BTHUD::UpdateDisplayPosition(void)
{
	float width = TheDisplayMgr->GetDisplayWidth();
	float height = TheDisplayMgr->GetDisplayHeight();

	m_objPaneChatBackground.SetElementPosition( Point3D( PANE_CHAT_X(width), height - PANE_CHAT_HEIGHT, 0.0F ) );
	m_objPaneInfoBackground.SetElementPosition( Point3D( PANE_INFO_X(width), height - PANE_INFO_HEIGHT, 0.0F ) );
	m_objPaneMMapBackground.SetElementPosition( Point3D( PANE_MMAP_X(width), height - PANE_MMAP_HEIGHT, 0.0F ) );

	m_objPaneChatBackground.SetElementSize( PANE_CHAT_WIDTH(width), PANE_CHAT_HEIGHT );
	m_objPaneInfoBackground.SetElementSize( PANE_INFO_WIDTH(width), PANE_INFO_HEIGHT );
	m_objPaneMMapBackground.SetElementSize( PANE_MMAP_WIDTH(width), PANE_MMAP_HEIGHT );

	m_objPaneChat.SetElementPosition( Point3D( PANE_CHAT_X(width) + PANE_DIVISOR, height - PANE_CHAT_HEIGHT + PANE_DIVISOR, 0.0F ) );
	m_objPaneInfo.SetElementPosition( Point3D( PANE_INFO_X(width) + PANE_DIVISOR, height - PANE_INFO_HEIGHT + PANE_DIVISOR + (PANE_INFO_HEIGHT * PANE_INFO_DELTA), 0.0F ) );
	m_objPaneMMap.SetElementPosition( Point3D( PANE_MMAP_X(width) + PANE_DIVISOR, height - PANE_INFO_HEIGHT + PANE_DIVISOR, 0.0F ) );

	m_objPaneChat.SetElementSize( PANE_CHAT_WIDTH(width) - PANE_DIVISOR_2, PANE_CHAT_HEIGHT - PANE_DIVISOR_2 );
	m_objPaneInfo.SetElementSize( PANE_INFO_WIDTH(width) - PANE_DIVISOR_2, PANE_INFO_HEIGHT - PANE_DIVISOR_2 - (PANE_INFO_HEIGHT * PANE_INFO_DELTA));
	m_objPaneMMap.SetElementSize( PANE_MMAP_WIDTH(width) - PANE_DIVISOR_2, PANE_MMAP_HEIGHT - PANE_DIVISOR_2 );

	m_objPaneChatBackground.Invalidate();
	m_objPaneInfoBackground.Invalidate();
	m_objPaneMMapBackground.Invalidate();

	m_objPaneChat.UpdateDisplayPosition();
	m_objPaneInfo.UpdateDisplayPosition();
	m_objPaneMMap.UpdateDisplayPosition();
}

BTHUD::~BTHUD()
{
	if( m_pMiniMap ) {
		delete m_pMiniMap;
		m_pMiniMap = nullptr;
	}
}

void BTHUD::New( const char *paneChat, const char *paneInfo, const char *paneMap )
{
	if( TheBTHUD == nullptr )
	{
		BTHUD *interface = new BTHUD( paneChat, paneInfo, paneMap );
		TheInterfaceMgr->AddElement( interface );
	}
}
