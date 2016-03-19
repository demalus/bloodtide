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

#include "BTScript_Build.h"
#include "MGGame.h"

using namespace C4;


EventScript_Build::EventScript_Build( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID ),
	m_ulBuildingID( 0 ),
	m_lTeam( 0 )
{

}

EventScript_Build::~EventScript_Build()
{
}

// Display the optional message and store the type of base to build.
void EventScript_Build::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	m_ulBuildingID = Text::StringToType( m_pScriptTable->GetString(StringID(m_ulID, 'BGID')) );
	m_lTeam = Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, 'TEAM')) );

	// optional start message
	const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG1'));
	const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND1'));
	TheGame->HelpMessage( msg, snd );
}

// Return true if the base was built, false otherwise.
bool EventScript_Build::CheckConditions( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return false;
	}

	const Array<long> *buildings = TheGameCacheMgr->GetGameCache(kCacheBuildings, m_lTeam);
	if( !buildings ) {
		return false;
	}

	int len = buildings->GetElementCount();
	for( int x = 0; x < len; x++ ) {
		BTEntityController *pEnt = static_cast<BTEntityController*>( world->GetController(*buildings[x]) );
		if( !pEnt ) {
			continue;
		}

		if( pEnt->GetStringID() == m_ulBuildingID ) {

			// optional end message
			const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG2'));
			const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND2'));
			TheGame->HelpMessage( msg, snd );
			return true;
		}
	}

	return false;
}

