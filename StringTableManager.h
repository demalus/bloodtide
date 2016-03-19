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

#ifndef StringTableManager_h
#define StringTableManager_h

#include "C4Base.h"
#include "C4StringTable.h"

namespace C4
{
	// String table IDs.
	enum TableID
	{
		kStringTableNone = -1,
		kStringTableUnits,
		kStringTableWorlds,
		kStringTableCommanders,
		kStringTableBuildings,
		kStringTableProjectiles,
		kStringTableMusic,
		kStringTableMessages,

		kStringTableCount
	};

	/**
	* This class will hold all of the string tables in use (besides the game string table).
	* It also provides an easy way to get these string tables.
	* @author - Frank Williams (demalus@gmail.com)
	*/
	class StringTableManager : public Singleton<StringTableManager>
	{
		private:

			StringTable	m_objUnits;
			StringTable	m_objWorlds;
			StringTable	m_objCommanders;
			StringTable m_objBuildings;
			StringTable m_objProjectiles;
			StringTable m_objMusic;
			StringTable m_objMessages;

		protected:

			StringTableManager();

		public:
			
			~StringTableManager();

			// Create new instance
			static void New( void );

			/** Get's any string table held by the manager by ID. */
			const StringTable *GetStringTable( TableID nStringTableID ) const;
	};

	extern StringTableManager *TheStringTableMgr;
}

#endif