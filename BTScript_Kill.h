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


#ifndef BTScript_Kill_h
#define BTScript_Kill_h

#include "BTScript.h"
#include "C4StringTable.h"

namespace C4
{

	/**
	 * This script waits for a base to be destroyed.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_KillBase : public EventScript
	{
		private:

			// Name of the base to kill.
			const char *m_pBaseName;

			// Team that the base is on.
			long			m_lTeam;

		public:

			EventScript_KillBase( StringTable *pTable, unsigned long ulID );
			~EventScript_KillBase();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};
}


#endif
