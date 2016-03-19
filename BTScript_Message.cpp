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

#include "BTScript_Message.h"
#include "MGGame.h"

using namespace C4;


EventScript_Message::EventScript_Message( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_Message::~EventScript_Message()
{
}

// Spawn the specified amount of units and attack a random base the player owns.
void EventScript_Message::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'HMSG'));
	const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'HSND'));
	TheGame->HelpMessage( msg, snd );
}

// If all spawn units are dead, this script is over.
// Return true if this script is done, false otherwise.
bool EventScript_Message::CheckConditions( void )
{
	return true;
}


