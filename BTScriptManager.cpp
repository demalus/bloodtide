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

#include "BTScriptManager.h"
#include "MGGame.h"

#include "BTScript_Spawn.h"
#include "BTScript_Message.h"
#include "BTScript_Select.h"
#include "BTScript_Build.h"
#include "BTScript_Kill.h"
#include "BTScript_Activation.h"

using namespace C4;

ScriptManager *C4::TheScriptMgr = nullptr;

ScriptManager::ScriptManager( StringTable *pScript ) :
	Singleton<ScriptManager>( TheScriptMgr ),
	m_pScriptTable( pScript ),
	m_pCurString( nullptr ),
	m_pScript( nullptr ),
	m_ulNextScriptTime( 0 ),
	m_bDelay( false )
{
	
}

void ScriptManager::New( StringTable *pScript )
{
	if( TheScriptMgr == nullptr ) {
		new ScriptManager( pScript );
	}
}

ScriptManager::~ScriptManager()
{
	if( m_pScriptTable ) {
		delete m_pScriptTable;
	}

	if( m_pScript ) {
		delete m_pScript;
	}
}

// Update the script or setup a new one.
void ScriptManager::Update( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	// If there is a current script run it.
	if( m_pScript ) {

		if( m_bDelay ) {

			if( TheTimeMgr->GetAbsoluteTime() >= m_ulNextScriptTime ) {
				delete m_pScript;
				m_pScript = nullptr;
				m_bDelay = false;
			}
			return;
		}

		// If the script is now over, delete it.
		if( m_pScript->CheckConditions() ) {
			unsigned long id = m_pCurString->GetStringID();
			int delay = (int)(Text::StringToFloat( m_pScriptTable->GetString(StringID(id, 'DLAY')) ) * 1000);  // get time in seconds
			m_ulNextScriptTime = TheTimeMgr->GetAbsoluteTime() + delay;

			m_bDelay = true;
		}
	}
	else {

		// Figure out the next script in the list.
		const StringHeader *next = nullptr;
		if( m_pCurString ) {
			next = m_pCurString->GetNextString();
		}
		else {
			next = m_pScriptTable->GetRootStringHeader();
		}

		m_pCurString = next;

		// No more scripts are left.
		if( next == nullptr ) {
			if( TheGame->GetPopScreen_Victory() )
				TheInterfaceMgr->AddElement(  TheGame->GetPopScreen_Victory() );
			TheGame->ExitCurrentGame();
			return;
		}

		// Get the type of script, create it, then run it.
		unsigned long id = next->GetStringID();

		// Create the right type of script.
		if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "SPAWN_UNITS_ATTACKTO_BASE_RAND" ) ) {
			m_pScript = new EventScript_Spawn_Attackto_Base_Rand( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "MESSAGE" ) ) {
			m_pScript = new EventScript_Message( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "SELECT_UNIT" ) ) {
			m_pScript = new EventScript_SelectUnit( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "BUILD" ) ) {
			m_pScript = new EventScript_Build( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "KILL_BASE" ) ) {
			m_pScript = new EventScript_KillBase( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "SPAWN_UNITS" ) ) {
			m_pScript = new EventScript_SpawnUnits( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "ACTIVATE_BASE" ) ) {
			m_pScript = new EventScript_ActivateBase( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "DEACTIVATE_BASE" ) ) {
			m_pScript = new EventScript_DeactivateBase( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "GOTO_FIELDMODE" ) ) {
			m_pScript = new EventScript_GoTo_FieldMode( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "ACTIVATE_COMM_SPAWN" ) ) {
			m_pScript = new EventScript_ActivateCommSpawn( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "DEACTIVATE_COMM_SPAWN" ) ) {
			m_pScript = new EventScript_DeactivateCommSpawn( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "ACTIVATE_MSG_BG" ) ) {
			m_pScript = new EventScript_ActivateMsgBG( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "DEACTIVATE_MSG_BG" ) ) {
			m_pScript = new EventScript_DeactivateMsgBG( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "ACTIVATE_HELP" ) ) {
			m_pScript = new EventScript_ActivateHelp( m_pScriptTable, id );
		}
		else if( Text::CompareTextCaseless( m_pScriptTable->GetString(StringID(id, 'TYPE')), "DEACTIVATE_HELP" ) ) {
			m_pScript = new EventScript_DeactivateHelp( m_pScriptTable, id );
		}

		if( m_pScript ) {
			m_pScript->Start();
		}
	}
}

