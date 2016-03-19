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


#ifndef BTScriptManager_h
#define BTScriptManager_h

#include "BTScript.h"
#include "C4StringTable.h"

namespace C4
{

	/**
	 * This class stores scripts from world files and manages
	 * the execution of said scripts.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class ScriptManager : public Singleton<ScriptManager>
	{
		private:

			StringTable *m_pScriptTable;			// String table containing the script to run.
			const StringHeader *m_pCurString;		// Currently selected string header in the string table.
			EventScript	*m_pScript;					// Current script running.

			bool m_bDelay;							// Whether or not to wait some time before moving to the next script.
			unsigned long m_ulNextScriptTime;		// Time to remove the current script and setup the next one.

		protected:

			ScriptManager( StringTable *pScript );

		public:
			
			~ScriptManager();	

			// Create new instance
			static void New( StringTable *pScript );

			// Check for conditions met, load new scripts, etc.
			void Update( void );
	};

	extern ScriptManager *TheScriptMgr;
}


#endif
