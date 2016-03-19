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


#ifndef BTScript_Message_h
#define BTScript_Message_h

#include "BTScript.h"
#include "C4StringTable.h"

namespace C4
{

	/**
	 * This script displays a message in the MessageInterface and optionally plays a sound.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_Message : public EventScript
	{
		public:

			EventScript_Message( StringTable *pTable, unsigned long ulID );
			~EventScript_Message();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};
}


#endif
