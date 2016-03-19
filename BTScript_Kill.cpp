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

#include "BTScript_Kill.h"
#include "MGGame.h"

using namespace C4;


EventScript_KillBase::EventScript_KillBase( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID ),
	m_pBaseName( "" ),
	m_lTeam( 0 )
{

}

EventScript_KillBase::~EventScript_KillBase()
{
}

// Display the optional message and store the name of the base to kill.
void EventScript_KillBase::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	m_pBaseName = m_pScriptTable->GetString(StringID(m_ulID, 'BSNM'));
	m_lTeam = Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, 'TEAM')) );

	// optional start message
	const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG1'));
	const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND1'));
	TheGame->HelpMessage( msg, snd );
}

// Return true if the base was killed, false otherwise.
bool EventScript_KillBase::CheckConditions( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return false;
	}

	const Array<long> *bases = TheGameCacheMgr->GetGameCache(kCacheBases, m_lTeam);
	if( !bases ) {
		return false;
	}

	int len = bases->GetElementCount();
	for( int x = 0; x < len; x++ ) {
		BTEntityController *pEnt = static_cast<BTEntityController*>( world->GetController(*bases[x]) );
		if( !pEnt ) {
			continue;
		}
		
		Node *node = pEnt->GetTargetNode();
		if( !node ) {
			continue;
		}

		if( node->GetNodeName() && Text::CompareTextCaseless(node->GetNodeName(), m_pBaseName) ) {
			return false;
		}
	}

	// optional end message
	const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG2'));
	const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND2'));
	TheGame->HelpMessage( msg, snd );

	return true;
}

