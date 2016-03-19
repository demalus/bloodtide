	
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

#ifndef BTMiniMapPane_h_
#define BTMiniMapPane_h_

#include "C4Interface.h"

namespace C4
{

#define UNIT_SIZE		4.0F
#define BASE_SIZE		10.0F
#define COMMANDER_SIZE	8.0F
#define BUILDING_SIZE	6.0F

class MiniMapPane : public Interface
{
	private:
		// The map background texture
		QuadElement				m_objBackground;

		// The box representing what the player sees
		QuadElement				m_objCameraBox;

		// Stores information about the size of the map
		long					m_lGameMapWidth;
		long					m_lGameMapHeight;

		// Arrays store information about a type of entity to draw
		Array<QuadElement*>		m_arrMyBases;
		Array<QuadElement*>		m_arrEnemyBases;

		Array<QuadElement*>		m_arrMyBuildings;
		Array<QuadElement*>		m_arrEnemyBuildings;

		Array<QuadElement*>		m_arrMyUnits;
		Array<QuadElement*>		m_arrEnemyUnits;

		Array<QuadElement*>		m_arrMyCommanders;
		Array<QuadElement*>		m_arrEnemyCommanders;

		// Minimap cursor state
		bool					m_bMouseDown;

		void CreateBasesArray( long team );
		void CreateBuildingsArray( long team );
		void CreateUnitsArray( long team );
		void CreateCommandersArray( long team );

		void CleanArray (Array<QuadElement*> &arr);
		void Cleanup( void );

	public:

		MiniMapPane();
		~MiniMapPane();

		void UpdateDisplayPosition( void );

		// Redraw the minimap with accurate information
		void UpdateMiniMap( void );

		void InterfaceTask( void );

		void SetMiniCam( void );

		PartCode TestPickPoint(const Point3D& p) const;
		void HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p);
};

}
#endif