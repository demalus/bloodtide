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


#ifndef BTScript_Build_h
#define BTScript_Build_h

#include "BTScript.h"
#include "C4StringTable.h"

namespace C4
{

	/**
	 * This script waits for a user to build a specified building.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_Build : public EventScript
	{
		private:

			// Building's ID in the Buildings string table.
			unsigned long	m_ulBuildingID;

			// Team that the building is on.
			long			m_lTeam;

		public:

			EventScript_Build( StringTable *pTable, unsigned long ulID );
			~EventScript_Build();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};
}


#endif
