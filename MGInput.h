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


#ifndef MGInput_h
#define MGInput_h


#include "C4Input.h"
#include "MGCameras.h" // - CW
#include "BTCommander.h"
#include "BTCursor.h"

#define MAX_FIELD_COMMANDER_ABILITIES 100

namespace C4
{
	// Action types
	enum
	{

		// Blood Tide action types - CW
		kActionNavUp			= 'mvup',
		kActionNavLeft			= 'mvlt',
		kActionNavDown			= 'mvdn',
		kActionNavRight			= 'mvrt',

		kActionSwitchModes		= 'swtc',

		kActionAttackToOrder	= 'orda',
		kActionStopOrder		= 'ords',

		kActionFCAbility1		= 'abi1',
		kActionFCAbility2		= 'abi2',
		kActionFCAbility3		= 'abi3',

		kActionShowCursor		= 'curs',

		kActionSelectAllUnits	= 'sall',
		kActionSendAllUnitsToCom = 'satc',

		kActionGroupUnits1		= 'grp1',
		kActionGroupUnits2		= 'grp2',
		kActionGroupUnits3		= 'grp3'
	};

	/*
	* Movement actions for the RTS camera.
	* @author Chris Williams
	*/
	class NavigationAction : public Action
	{
		private:

			NavigationFlag		m_ulNavigationType;

		public:

			NavigationAction::NavigationAction(ActionType Type, NavigationFlag ulNavigationType) : Action(Type),
													m_ulNavigationType(ulNavigationType)
			{
			}

			void Begin(void);
			void End(void);
	};

	/*
	* Issues an order to all selected units.
	* @author Chris Williams
	*/
	class UnitOrderAction : public Action
	{
		private:

			UnitOrderType			m_Order;

		public:

			UnitOrderAction(ActionType Type, UnitOrderType Order) : Action(Type), m_Order(Order) {}

			void Begin(void);
			void End(void);
	};

	class SelectAllUnitsAction : public Action
	{
		public:

			SelectAllUnitsAction();

			void Begin();
	};

	class SendAllUnitToComAction : public Action
	{
	public:

		SendAllUnitToComAction();

		void Begin();
	};

	class GroupUnitsAction : public Action
	{
		private:

			UnitGroups				m_Group;

		public:

			GroupUnitsAction(ActionType Type, UnitGroups group );

			void Begin(void);
	};


	/*
	* Triggers a Field Commander ability.
	* @author Chris Williams
	*/
	class FieldCommanderAction : public Action
	{
		private:

			long					m_lAbility; // corresponds with kMotionAbility1, 2, 3 etc. in EntityMotion in BTControllers.h

		public:

			FieldCommanderAction(ActionType Type, long lAbility) : Action(Type), m_lAbility( lAbility ) {}

			void Begin(void);
			void End(void);
	};

	/*
	* Switches from field mode to command mode.
	* @author Chris Williams
	*/
	class SwitchBTModesAction : public Action
	{
		public:

			SwitchBTModesAction() : Action(kActionSwitchModes) {};

			void Begin(void);
	};

	/*
	* Brings up the cursor when in Field Mode.
	* @author Chris Williams
	*/
	class ShowCursorAction : public Action
	{
		private:

			bool				m_bActivated;

		public:

			ShowCursorAction() : Action( kActionShowCursor ), m_bActivated( false ) {}

			void Begin(void);
			void End(void);
	};

}

#endif

