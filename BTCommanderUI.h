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

#ifndef BTCommanderUI_h_
#define BTCommanderUI_h_

#include "C4Interface.h"
#include "BTInterface.h"

namespace C4
{
	// forward declarations
	class StatsProtocol;

	/*
	 * Displays a circular list of commanders that the player can select from.
	 * @author Chris Williams
	 */
	class CommanderSelecter : public Interface, Singleton<CommanderSelecter>
	{
		class CSIcon : public ResizingImage
		{
			private:

				long					m_lBaseControllerID;

			public:

				CSIcon(long lBaseControllerID);

				PartCode TestPickPoint(const Point3D &p) const;
				void HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p);

				void Disable( long lBaseControllerID );
		};

		private:

			TextElement						m_objSelectCommander;
			Array<CSIcon *>					m_arrCommanderIcons;	
	
			CommanderSelecter(Array<long> &arrBaseIDs, float flRadius);

		public:

			static void New(Array<long> &arrBaseIDs, float flRadius); // IDs in the Commanders string table

			void RemoveBase( long lBaseControllerID );

			void Show( void );

			void InterfaceTask(void);
	};

	class UnitIcon : public Interface
	{
		private:

			Link<StatsProtocol>			m_pStatsController;
			long						m_lControllerIndex;

			OutlineElement				m_objOutline;
			ImageElement				m_objImage;

		public:

			UnitIcon();
			~UnitIcon();

			void SetUnit(long lControllerIndex);
			void UpdateSize(float fWidth, float fHeight);
			void Update(long lControllerIndex);

			void InterfaceTask(void);
	};

	class UnitIconMgr : public Interface, Singleton<UnitIconMgr>
	{
		private:

			long					m_lUnitIconCount;
			
			float					m_fCurrentIconWidth;
			float					m_fCurrentIconHeight;

			Interface				m_objUnitIcons;
			TextElement				m_objSelectedGroup;

			void AddUnitIcon(long lControllerIndex);

			bool ShrinkIconSize(void);
			void RefreshIcons(void);

			UnitIconMgr();

		public:

			static void New(void);

			void AddUnit(long lControllerIndex);
			void AddUnits(const Array<long> &arrControllerIndexes);

			void SetGroup( const char *group )
			{
				m_objSelectedGroup.SetText( group );
			}

			void RemoveUnit(long lControllerIndex);
			void PurgeUnits(void);

			void UpdateDisplayPosition( void );
			void InterfaceTask(void);
	};

	extern UnitIconMgr *TheUnitIconMgr;
	extern CommanderSelecter *TheCommanderSelecter;
}

#endif
