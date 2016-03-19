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

#include "BTScript_Select.h"
#include "MGGame.h"

using namespace C4;

////// EventScript_SelectUnit

EventScript_SelectUnit::EventScript_SelectUnit( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID ),
	m_ulUnitID( 0 )
{

}

EventScript_SelectUnit::~EventScript_SelectUnit()
{
}

// Display the optional message and store the type of unit to select.
void EventScript_SelectUnit::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	m_ulUnitID = Text::StringToType( m_pScriptTable->GetString(StringID(m_ulID, 'UNID')) );

	// optional start message
	const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG1'));
	const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND1'));
	TheGame->HelpMessage( msg, snd );
}

// Return true if the unit was selected, false otherwise.
bool EventScript_SelectUnit::CheckConditions( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return false;
	}

	if( TheBTCursor ) {
		const Array<long>& units = TheBTCursor->GetSelectedUnits();
		int len = units.GetElementCount();

		for( int x = 0; x < len; x++ ) {
			BTEntityController *pEnt = static_cast<BTEntityController*>( world->GetController(units[x]) );
			if( !pEnt ) {
				continue;
			}

			if( pEnt->GetStringID() == m_ulUnitID ) {

				// optional end message
				const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG2'));
				const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND2'));
				TheGame->HelpMessage( msg, snd );
				return true;
			}
		}
	}

	return false;
}


////// EventScript_GoTo_FieldMode

EventScript_GoTo_FieldMode::EventScript_GoTo_FieldMode( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID ),
	m_ulUnitID( 0 )
{

}

EventScript_GoTo_FieldMode::~EventScript_GoTo_FieldMode()
{
}

// Display the optional message.
void EventScript_GoTo_FieldMode::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	// optional start message
	const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG1'));
	const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND1'));
	TheGame->HelpMessage( msg, snd );
}

// Return true if the unit was selected, false otherwise.
bool EventScript_GoTo_FieldMode::CheckConditions( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return false;
	}

	if( world->GetInFieldMode() ) {

		// optional end message
		const char *msg = m_pScriptTable->GetString(StringID(m_ulID, 'MSG2'));
		const char *snd = m_pScriptTable->GetString(StringID(m_ulID, 'SND2'));
		TheGame->HelpMessage( msg, snd );

		return true;
	}

	return false;
}

