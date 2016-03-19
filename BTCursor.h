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

#ifndef BTCursor_h_
#define BTCursor_h_

#include "C4Types.h"
#include "C4Models.h"
#include "C4Collision.h"
#include "C4Math.h"
#include "C4Time.h"
#include "C4Interface.h"
#include "BTCacheManager.h"

namespace C4 {

	enum UnitGroups
	{
		kUnitGroup1 = 1,
		kUnitGroup2,
		kUnitGroup3,
		kUnitFieldMode	// whatever units were selected before entering field mode
	};

	enum UnitOrderType {

		kOrderNone,
		kOrderAttackTo,
		kOrderStop
	};


	/**
	 * Base class for mouse states.
	 * @author - Chris Williams
	 */
	class MouseState
	{
		public:

			virtual void Enter(const Point &point) {}
			virtual void Hover(const Point &point) {}
			virtual void MouseDown(const Point &point) {}
			virtual void MouseUp(const Point &point) {}
			virtual void MiddleMouseDown(const Point &point) {}
			virtual void MiddleMouseUp(const Point &point) {}
			virtual void RightMouseDown(const Point &point) {}
			virtual void RightMouseUp(const Point &point) {}
			virtual void Exit(const Point &point) {}
	};

	/**
	 * Default mouse state. User interacts with the interface and game world with a cursor.
	 * @author Chris Williams
	 */
	class DefaultState : public MouseState, public Singleton<DefaultState>
	{
		friend class BTCursor;

		private:

			// set when mouse down is called,
			// if this value changes, the state is changed to SelectBoxState
			Point				*m_pStartPosition;

			// used to determine if TRACE_TIME_DELTA has passed between ray traces
			unsigned long		m_ulLastTraceTime; 

			// only valid if MouseDown was called for this state (it is possible to click on an interface element, then mouse up on a valid space)
			bool				m_bMouseUpValid;

		public:

			DefaultState();
			~DefaultState();

			void Enter(const Point &point);
			void Hover(const Point &point);
			void MouseDown(const Point &point);
			void MouseUp(const Point &point);
			void RightMouseDown(const Point &point);
			void RightMouseUp(const Point &point);
			void Exit(const Point &point);
	};

	/**
	 * Select box state. Instead of using a mouse cursor,
	 * the user re-sizes a select box in order to select multiple units.
	 * @author Chris Williams
	 */
	class SelectBoxState : public MouseState, public Singleton<SelectBoxState>
	{
		private:

			Point				*m_pStartPosition;
			OutlineElement		*m_pSelectBox;

			void GetEntitiesInSelectBox( CacheType cache, long team, Array<long>& entities );
			void SelectEntities( void );

		public:

			SelectBoxState();
			~SelectBoxState();

			void Enter(const Point &point);
			void Hover(const Point &point);
			void MouseUp(const Point &point);
			void Exit(const Point &point);
	};

	// forward delcerations
	class PlateGeometry;
	class BTEntityController;

	/**
	 * This class provides the implementation for game-specific mouse events.
	 * This includes hovering the cursor over entities, selection of entities, etc..
	 * @author Chris Williams
	 */
	class BTCursor : public Singleton<BTCursor>
	{
		private: 

			MouseEventHandler			mouseEventHandler;

			// Cursor states
			DefaultState				m_objDefaultState;
			SelectBoxState				m_objSelectBoxState;

			MouseState					*m_pCurrentState;
			//

			Array<long>					m_arrSelectedUnits;	

			Array<long>					m_arrUnitGroup1;
			Array<long>					m_arrUnitGroup2;
			Array<long>					m_arrUnitGroup3;
			Array<long>					m_arrUnitsFieldMode;

			UnitOrderType				m_CurrentOrder;

		public:

			BTCursor();
			~BTCursor();

			static void HandleMouseEvent(EventType eventType, const Point& point, void *data);
			void BTMouseEvent(EventType eventType, const Point& point); // Handles game specific mouse events.

			UnitOrderType GetOrder(void) { return m_CurrentOrder; }
			void SetOrder(UnitOrderType Order);

			void ChangeState(MouseState *newState, const Point &point);

			const Array<long> &GetSelectedUnits(void);
			void AddSelectedUnit( BTEntityController& entity );
			void AddSelectedUnits( Array<long>& units );
			void RemoveSelectedUnit(long lControllerIndex);
			void PurgeSelectedUnits(void);

			void SelectAllUnits(void);

			void SelectGroup( UnitGroups groupNumber );
			void GroupSelectedUnits( UnitGroups groupNumber );

			void GiveOrder( Point3D& p3dWorldPosition );

			void ShowEntityInformation( long lControllerIndex );

			// Creates a ray from the current camera to the mouse cursor.
			// Modified version of http://www.terathon.com/wiki/index.php/Creating_a_Ray_for_a_Click_Location
			Ray WorldRayFromMousePosition(const Point &point);
			
			// Returns the entity (if any) that is under the mouse cursor.
			BTEntityController *GetEntityFromMousePosition(const Point &point);

			// Used when calculating which units are within the select box:
			// a ray is cast in the direction of the two corners of the select box
			// and their intersection points are used to calculate which units are within the box
			Point3D GetCameraRayBisectionWithWorld(const Point &point);

			Point3D GetCameraRayBisectionWithHeight(const Point &point, float height);

			bool CursorCollidesWithElement(void);

			void InterfaceTask(void);
	};

	extern BTCursor			*TheBTCursor;
	extern DefaultState		*TheDefaultState;
	extern SelectBoxState	*TheSelectBoxState;
}

#endif