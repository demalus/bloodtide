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

#ifndef BTInfoPane_h_
#define BTInfoPane_h_

#include "BTInterface.h"
#include "StringTableManager.h"

namespace C4
{
	enum InfoPaneType
	{
		kInfoPaneNone,
		kInfoPaneUnit,
		kInfoPaneCommander,
		kInfoPaneBase,
		kInfoPaneBuilding
	};


	//forward declerations
	class CommanderController;
	class UnitController;
	class BaseController;
	class BuildingController;
	class Ability;

	class AbilityBar : public Interface
	{
		private:

			class Icon : public ResizingImage
			{
				class Quad : public QuadElement
				{
					public:

						bool TestCursorCollision(const Point3D& p) const { return (false); }
				};

				private:

					Ability					*m_pAbility;
					long					m_lAbilityNumber; // whatever number the entity has mapped the given ability to

					Quad					m_objTimer; // for cooldowns and graying out locked abilities

					bool					m_bUnlocked;
					bool					m_bClickable; // if false, clicking on this ability will have no effect
	
				public:

					Icon( float fDefaultSize, float fMaxSize );
					~Icon();

					void SetAbility( Ability *pAbility, TableID stringTableID, Type ulEntityID, Type ulAbilityID, long lAbilityNumber, bool bClickable );
					void Unlock( bool bUnlock );

					PartCode TestPickPoint(const Point3D &p) const;
					void HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p);

					void InterfaceTask( void );
			};

			Array<Icon *>		m_arrAbilityIcons;

		public:

			AbilityBar( long numAbilities, float fDefaultSize, float fMaxSize );
			~AbilityBar();

			/*
			 * stringTableID must be either kStringTableUnits or kStringTableCommanders
			 * ulEntityID is the Commander or Unit ID within the string table (e.g. 'SCUB')
			 * ulAbilityID is the ability within that Commander or Unit (e.g. 'abi1')
			 * lAbilityNumber is the number that the entity has mapped the given ability to
			 * if bClickable is false, then clicking on the ability will have no effect (for non-commander panes that want to use the ability bar)
			 */
			void SetAbility( long lSlotNumber, Ability *pAbility, TableID stringTableID, Type ulEntityID, Type ulAbilityID, long lAbilityNumber, bool bClickable );

			void UnlockAbility( long lSlotNumber, bool bUnlock );
	};

	class RankBar : public Interface
	{
		private:

			class Icon : public Interface
			{
				private:

					ResizingImage			m_objDefaultImage;
					ResizingImage			m_objUnlockedImage;

					bool					m_bUnlocked;

					// for effects when unlocking the rank
					bool					m_bUnlocking;
					unsigned long			m_ulStartTime;

				public:

					Icon( const char *strDefaultImage, const char *strUnlockedImage, float fSize );

					void Unlock( bool bUnlock );

					void InterfaceTask( void );
			};

			Array<Icon *>				m_arrRankIcons;

			long						m_lCommanderKills;		

		public:

			RankBar( long numRanks );
			~RankBar();

			void AddToKillCount( long lDeltaKills );
			void UnlockRank( long lSlotNumber, bool bUnlock );
	};

	class StatsBox : public Interface
	{
		private:
		
			long						m_lControllerIndex;
			Link<StatsProtocol>			m_pStatsController;
			
			TextElement					m_textStrength;
			TextElement					m_textArmor;
			TextElement					m_textMoveSpeed;

			void FormatStat(char *strIdentifier, float fBaseStat, float fEffectOnStat, String<> &strText);
			
		public:

			StatsBox();

			void SetCurrentUnit(long lControllerIndex);
			long GetCurrentUnit(void) { return m_lControllerIndex; }

			void InterfaceTask(void);
	};

	class InfoPane : public Interface
	{
		private:

			long		m_lControllerIndex; // the controller index of the entity that is being displayed in this info pane

		public:
		
			InfoPane();
			~InfoPane();

			void SetEntity( long lControllerIndex );
			void HideIfSelected( long lControllerIndex );
	};

	class CommanderInfoPane : public InfoPane
	{
		private:

			RankBar						*m_pRankBar;

			ImageElement				m_objCommanderIcon;

			StatsBox					m_objStats;

			HealthBar					*m_pHealthBar;

			AbilityBar					*m_pAbilityBar;

		public:

			CommanderInfoPane( long numAbilities, long numRanks );
			~CommanderInfoPane();

			RankBar *GetRankBar( void )
			{
				return (m_pRankBar);
			}

			AbilityBar *GetAbilityBar( void )
			{
				return (m_pAbilityBar);
			}

			HealthBar *GetHealthBar( void )
			{
				return (m_pHealthBar);
			}

			// passing in a nullptr for commander will reset the Commander Pane back to defaults
			void SetCommander(CommanderController *commander );

			void UpdateDisplayPosition( void );
	};

	class UnitInfoPane : public InfoPane
	{
		private:

			ImageElement				m_objUnitIcon;

			StatsBox					m_objStats;

			HealthBar					*m_pHealthBar;

			AbilityBar					*m_pAbilityBar;

		public:

			UnitInfoPane( long numAbilities );
			~UnitInfoPane();


			AbilityBar *GetAbilityBar( void )
			{
				return (m_pAbilityBar);
			}

			HealthBar *GetHealthBar( void )
			{
				return (m_pHealthBar);
			}

			// passing in a nullptr for unit will reset the Unit Pane back to defaults
			void SetUnit( UnitController *unit );

			void UpdateDisplayPosition( void );
	};

	class BaseInfoPane : public InfoPane
	{
		private:

			ImageElement				m_objBaseIcon;
			TextElement					m_objBaseName;

			HealthBar					*m_pHealthBar;

		public:

			BaseInfoPane();
			~BaseInfoPane();

			// passing in a nullptr for unit will reset the Base Pane back to defaults
			void SetBase( BaseController *base );

			void UpdateDisplayPosition( void );
	};

	class BuildingInfoPane : public InfoPane
	{
		private:

			ImageElement				m_objBuildingIcon;
			TextElement					m_objBuildingName;

			HealthBar					*m_pHealthBar;

		public:

			BuildingInfoPane();
			~BuildingInfoPane();

			// passing in a nullptr for unit will reset the Building Pane back to defaults
			void SetBuilding( BuildingController *building );

			void UpdateDisplayPosition( void );
	};

	class InfoPaneMgr : public Singleton<InfoPaneMgr>
	{
		private:

			InfoPaneType						m_typeCurrentPane;

			UnitInfoPane						*m_pUnitInfoPane;
			CommanderInfoPane					*m_pCommanderInfoPane;
			BaseInfoPane						*m_pBaseInfoPane;
			BuildingInfoPane					*m_pBuildingInfoPane;

			InfoPaneMgr( void );

		public:

			~InfoPaneMgr( void );

			static void New( void );

			InfoPane& GetInfoPane( InfoPaneType type );
			void ShowInfoPane( InfoPaneType type );

			InfoPane& GetCurrentPane( void );
			InfoPaneType CurrentPane( void );
	};

	extern InfoPaneMgr *TheInfoPaneMgr;
}

#endif