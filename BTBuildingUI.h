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

#ifndef BTBuildingUI_h_
#define BTBuildingUI_h_

#include "C4Interface.h"


namespace C4 {

	class BuildingsMenuItem : public Interface
	{
		friend class BuildingsMenu;

		private:

			unsigned long		m_ulBuildingID;

			QuadElement			m_objBackground;
			OutlineElement		m_objOutline;
			ImageElement		m_objIcon;

			TextElement			m_objToolTip;

		public:
			
			BuildingsMenuItem(unsigned long ulBuildingID, const char *strBuildingIcon, const char *strToolTip);

			void Hilite(void);
			void Unhilite(void);
	};


	class BuildingsMenu : public Interface, Singleton<BuildingsMenu>
	{
		private:

			long							m_lPlotIndex;
			Array<BuildingsMenuItem*>		m_arrMenuItems;
			BuildingsMenuItem				*m_pCurrentOption;


			BuildingsMenu(long lPlotIndex, const Array<unsigned long> &arrBuildings, const Point &point);

		public:

			~BuildingsMenu();

			static void New(long lPlotIndex, const Array<unsigned long> &arrBuildings, const Point &point);


			void HandleMouseEvent(EventType EventType, PartCode code, const Point3D& p);
	};


	extern BuildingsMenu *TheBuildingsMenu;
}



#endif
