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


#include "StringTableManager.h"

using namespace C4;

StringTableManager *C4::TheStringTableMgr = nullptr;

StringTableManager::StringTableManager() :
	Singleton<StringTableManager>(TheStringTableMgr),
	m_objUnits("string_table/Units"),
	m_objWorlds("string_table/Worlds"),
	m_objCommanders("string_table/Commanders"),
	m_objBuildings("string_table/Buildings"),
	m_objProjectiles("string_table/Projectiles"),
	m_objMusic("string_table/Music"),
	m_objMessages("string_table/Messages")
{
}

StringTableManager::~StringTableManager()
{
}

void StringTableManager::New( void )
{
	if( TheStringTableMgr == nullptr ) {
		new StringTableManager();
	}
}

const StringTable *StringTableManager::GetStringTable( TableID nStringTableID ) const
{
	switch( nStringTableID ) {

		case kStringTableUnits:
			return (&m_objUnits);

		case kStringTableWorlds:
			return (&m_objWorlds);

		case kStringTableCommanders:
			return (&m_objCommanders);

		case kStringTableBuildings:
			return (&m_objBuildings);

		case kStringTableProjectiles:
			return (&m_objProjectiles);

		case kStringTableMusic:
			return (&m_objMusic);

		case kStringTableMessages:
			return (&m_objMessages);

		default:
			return nullptr;
	}
}
