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


#ifndef BTScript_h
#define BTScript_h

namespace C4
{

	// Forward declarations
	class StringTable;
	class BTEntityController;

	/**
	 * This class represents a scripted event that occurs in game.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript 
	{
		protected:

			// String table containing this script.
			StringTable *m_pScriptTable;	

			// ID of this script.
			unsigned long m_ulID;

		public:

			EventScript( StringTable *pTable, unsigned long ulID ) 
			{
				m_pScriptTable = pTable;
				m_ulID = ulID;
			}

			virtual ~EventScript() {}

			// Begin script events.
			virtual void Start( void ) = 0;

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void ) = 0;
	};
}


#endif
