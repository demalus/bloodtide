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


#ifndef BTScript_Spawn_h
#define BTScript_Spawn_h

#include "BTScript.h"
#include "C4Array.h"
#include "BTControllers.h"

namespace C4
{

	/**
	 * This script spawns units and makes them attack one of the player's bases (at random).
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_Spawn_Attackto_Base_Rand : public EventScript
	{
		private:

			// List of spawned attackers.
			Array<Link<BTEntityController>*> m_arrAttackers;

		public:

			EventScript_Spawn_Attackto_Base_Rand( StringTable *pTable, unsigned long ulID );
			~EventScript_Spawn_Attackto_Base_Rand();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script spawns units at a location.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_SpawnUnits : public EventScript
	{
		public:

			EventScript_SpawnUnits( StringTable *pTable, unsigned long ulID );
			~EventScript_SpawnUnits();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};
}


#endif
