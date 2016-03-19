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

#include "BTScript_Activation.h"
#include "MGGame.h"
#include "BTCommanderUI.h"

using namespace C4;

////// EventScript_ActivateBase

EventScript_ActivateBase::EventScript_ActivateBase( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_ActivateBase::~EventScript_ActivateBase()
{
}

// Get the specified base and then activate it.
void EventScript_ActivateBase::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	const char *pBaseName = m_pScriptTable->GetString(StringID(m_ulID, 'BSNM'));
	long lTeam = Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, 'TEAM')) );

	const Array<long> *bases = TheGameCacheMgr->GetGameCache(kCacheBases, lTeam);
	if( !bases ) {
		return;
	}

	int len = bases->GetElementCount();
	for( int x = 0; x < len; x++ ) {
		BaseController *base = static_cast<BaseController*>( world->GetController(*bases[x]) );
		if( !base ) {
			continue;
		}

		Node *node = base->GetTargetNode();
		if( !node ) {
			continue;
		}

		if( node->GetNodeName() && Text::CompareTextCaseless(node->GetNodeName(), pBaseName) ) {
			base->ActivateBase();
			return;
		}
	}
}

bool EventScript_ActivateBase::CheckConditions( void )
{
	return true;
}


////// EventScript_DeactivateBase

EventScript_DeactivateBase::EventScript_DeactivateBase( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_DeactivateBase::~EventScript_DeactivateBase()
{
}

// Get the specified base and then deactivate it.
void EventScript_DeactivateBase::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	const char *pBaseName = m_pScriptTable->GetString(StringID(m_ulID, 'BSNM'));
	long lTeam = Text::StringToInteger( m_pScriptTable->GetString(StringID(m_ulID, 'TEAM')) );

	const Array<long> *bases = TheGameCacheMgr->GetGameCache(kCacheBases, lTeam);
	if( !bases ) {
		return;
	}

	int len = bases->GetElementCount();
	for( int x = 0; x < len; x++ ) {
		BaseController *base = static_cast<BaseController*>( world->GetController(*bases[x]) );
		if( !base ) {
			continue;
		}

		Node *node = base->GetTargetNode();
		if( !node ) {
			continue;
		}

		if( node->GetNodeName() && Text::CompareTextCaseless(node->GetNodeName(), pBaseName) ) {
			base->DeactivateBase();
			return;
		}
	}
}

bool EventScript_DeactivateBase::CheckConditions( void )
{
	return true;
}


////// EventScript_ActivateCommSpawn

EventScript_ActivateCommSpawn::EventScript_ActivateCommSpawn( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_ActivateCommSpawn::~EventScript_ActivateCommSpawn()
{
}

// Activate the spawning of commanders by showing the UI for it.
void EventScript_ActivateCommSpawn::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	if( !TheCommanderSelecter ) {
		return;
	}

	TheCommanderSelecter->Show();
}

bool EventScript_ActivateCommSpawn::CheckConditions( void )
{
	return true;
}


////// EventScript_DeactivateCommSpawn

EventScript_DeactivateCommSpawn::EventScript_DeactivateCommSpawn( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_DeactivateCommSpawn::~EventScript_DeactivateCommSpawn()
{
}

// Get the specified base and then deactivate it.
void EventScript_DeactivateCommSpawn::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	if( !TheCommanderSelecter ) {
		return;
	}

	TheCommanderSelecter->Hide();
}

bool EventScript_DeactivateCommSpawn::CheckConditions( void )
{
	return true;
}


////// EventScript_ActivateMsgBG

EventScript_ActivateMsgBG::EventScript_ActivateMsgBG( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_ActivateMsgBG::~EventScript_ActivateMsgBG()
{
}

// Activate the background on the message interface.
void EventScript_ActivateMsgBG::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	if( !TheMessageInterface ) {
		return;
	}

	TheMessageInterface->EnableBG();
}

bool EventScript_ActivateMsgBG::CheckConditions( void )
{
	return true;
}


////// EventScript_DeactivateMsgBG

EventScript_DeactivateMsgBG::EventScript_DeactivateMsgBG( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_DeactivateMsgBG::~EventScript_DeactivateMsgBG()
{
}

// Turn the message interface background off.
void EventScript_DeactivateMsgBG::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	if( !TheMessageInterface ) {
		return;
	}

	TheMessageInterface->DisableBG();
}

bool EventScript_DeactivateMsgBG::CheckConditions( void )
{
	return true;
}

////// EventScript_ActivateHelp

EventScript_ActivateHelp::EventScript_ActivateHelp( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_ActivateHelp::~EventScript_ActivateHelp()
{
}

// Activate the help tips.
void EventScript_ActivateHelp::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	if( !TheMessageInterface ) {
		return;
	}

	TheGame->SetHelpActive( true );
}

bool EventScript_ActivateHelp::CheckConditions( void )
{
	return true;
}


////// EventScript_DeactivateHelp

EventScript_DeactivateHelp::EventScript_DeactivateHelp( StringTable *pTable, unsigned long ulID ) :
	EventScript( pTable, ulID )
{

}

EventScript_DeactivateHelp::~EventScript_DeactivateHelp()
{
}

// Turn the help tips off.
void EventScript_DeactivateHelp::Start( void )
{
	if( !m_pScriptTable ) {
		return;
	}

	if( !TheMessageInterface ) {
		return;
	}

	TheGame->SetHelpActive( false );
}

bool EventScript_DeactivateHelp::CheckConditions( void )
{
	return true;
}

