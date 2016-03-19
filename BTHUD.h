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

#ifndef BTHUD_h_
#define BTHUD_h_

#define PANE_HEIGHT 180.0F
#define PANE_CHAT_HEIGHT 180.0F

// dimensions of the backgrounds for each of the three panes
// the panes themselves must be offset by the PANE_DIVISOR constant so that they do not overlap with the backgrounds
#define PANE_INFO_HEIGHT 180.0F
#define PANE_INFO_DELTA 0.1796875F // the info pane is 46 pixels below the other two panes (using a height of 256), the delta value is 46/256
#define PANE_MMAP_HEIGHT 180.0F

#define PANE_DIVISOR 22.0F
#define PANE_DIVISOR_2 44.0F

#define PANE_CHAT_X(displayWidth) ( 0.0F )
#define PANE_CHAT_WIDTH(displayWidth) ( displayWidth * 0.25F )

#define PANE_INFO_X(displayWidth) ( PANE_CHAT_WIDTH(displayWidth) )
#define PANE_INFO_WIDTH(displayWidth) ( displayWidth * 0.5F )

#define PANE_MMAP_X(displayWidth) ( PANE_INFO_X(displayWidth) + PANE_INFO_WIDTH(displayWidth) )
#define PANE_MMAP_WIDTH(displayWidth) ( displayWidth * 0.25F )

#include "C4Interface.h"
#include "BTMiniMapPane.h"

namespace C4
{
	class Pane : public Interface
	{
		public:

			Pane();

			~Pane();

			void InsertElement( Interface& element ); // inserts an interface into the pane and updates its height, width and position to fit into the pane,
													  // will call HandleDisplayUpdate() on the interface

			bool ContainsElement( Interface& element ); // returns true if the given interface is on the pane

			void PurgePane( void ); // removes any elements that were inserted into the pane (does not destroy them)

			void UpdateDisplayPosition( void );
	};

	/*
	 * The Blood Tide HUD features three panes along the bottom of the screen: the chat window pane, 
	 * the general information pane (that will display information depending on which entities are selected)
	 * and a mini map pane.
	 * @author Chris Williams
	 */
	class BTHUD : public Interface, public Singleton<BTHUD>
	{
		private:

			Pane					m_objPaneChat;
			Pane					m_objPaneInfo;
			Pane					m_objPaneMMap;

			MiniMapPane				*m_pMiniMap;

			ImageElement			m_objPaneChatBackground;
			ImageElement			m_objPaneInfoBackground;
			ImageElement			m_objPaneMMapBackground;

			BTHUD( const char *paneChat, const char *paneInfo, const char *paneMap );

		public:

			~BTHUD();

			void UpdateDisplayPosition( void ); // will call this on every pane

			static void New( const char *paneChat, const char *paneInfo, const char *paneMap );

			Pane& GetPaneChat()
			{
				return (m_objPaneChat);
			}

			Pane& GetPaneInfo()
			{
				return (m_objPaneInfo);
			}

			Pane& GetPaneMap()
			{
				return (m_objPaneMMap);
			}
	};

	extern BTHUD *TheBTHUD;
}

#endif
