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


#ifndef BTScript_Select_h
#define BTScript_Select_h

#include "BTScript.h"
#include "C4StringTable.h"

namespace C4
{

	/**
	 * This script waits for a user to select a specified unit.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_SelectUnit : public EventScript
	{
		private:

			// Unit's ID in the Units string table.
			unsigned long	m_ulUnitID;

		public:

			EventScript_SelectUnit( StringTable *pTable, unsigned long ulID );
			~EventScript_SelectUnit();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script waits for a user to go into field mode.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_GoTo_FieldMode : public EventScript
	{
		private:

			// Unit's ID in the Units string table.
			unsigned long	m_ulUnitID;

		public:

			EventScript_GoTo_FieldMode( StringTable *pTable, unsigned long ulID );
			~EventScript_GoTo_FieldMode();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};
}


#endif
