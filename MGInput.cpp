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


#include "MGMultiplayer.h"
#include "BTCommander.h"
#include "MGGame.h"
#include "BTCommanderUI.h"


using namespace C4;


void NavigationAction::Begin(void)
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );

	if( world != nullptr ) {

		RTSCamera& commanderCamera = world->GetCommanderCamera();

		commanderCamera.SetNavigationFlags( commanderCamera.GetNavigationFlags() | m_ulNavigationType );
	}
}

void NavigationAction::End(void)
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );

	if( world != nullptr ) {

		RTSCamera& commanderCamera = world->GetCommanderCamera();

		commanderCamera.SetNavigationFlags( commanderCamera.GetNavigationFlags() & ~m_ulNavigationType );
	}

}

void UnitOrderAction::Begin()
{

}

void UnitOrderAction::End()
{
	if( TheBTCursor ) {

		TheBTCursor->SetOrder(m_Order);
	}
}

SelectAllUnitsAction::SelectAllUnitsAction() : Action( kActionSelectAllUnits )
{

}

void SelectAllUnitsAction::Begin( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	if( world->GetInFieldMode() ) {
		return;
	}

	TheBTCursor->PurgeSelectedUnits();
	TheBTCursor->SelectAllUnits();

	TheUnitIconMgr->PurgeUnits();
	TheUnitIconMgr->AddUnits( TheBTCursor->GetSelectedUnits() );
}

SendAllUnitToComAction::SendAllUnitToComAction() : Action( kActionSendAllUnitsToCom )
{

}

void SendAllUnitToComAction::Begin( void )
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	if( !world->GetInFieldMode() ) {
		return;
	}

	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( !player ) {
		return;
	}


	if( !TheGameCacheMgr ) {
		return;
	}


	Array<long> *units = TheGameCacheMgr->GetGameCache( kCacheUnits, player->GetPlayerTeam() );
	if( !units ) {
		return;
	}

	if( !player->GetPlayerController() || player->GetPlayerController()->GetControllerType() != kControllerCommander ) {
		return;
	}

	long targetIndex = player->GetPlayerController()->GetControllerIndex();

	Array<UnitCommand> commands;
	int commandsSent = 0;

	long size = units->GetElementCount();
	for( long x = 0; x < size; x++ ) {

		long conti = (*units)[x];
		BTEntityController *cont = static_cast<BTEntityController *>( world->GetController( conti ) );
		if( !cont ) {
			continue;
		}

		UnitCommand command;
		command.m_lUnit = conti;
		command.m_lTarget = targetIndex;

		commands.AddElement( command );
		commandsSent++;

		if( commandsSent >= COMMANDS_PER_BATCH-1 ) {
			TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandFollow, commands) );
			commandsSent = 0;
			commands.Purge();
		}
	}

	if( commands > 0 ) {
		TheMessageMgr->SendMessage( kPlayerServer, UnitCommandMessage(kCommandFollow, commands) );
	}
}


GroupUnitsAction::GroupUnitsAction(ActionType Type, UnitGroups group ) : Action( Type ),
		m_Group( group )
{

}

void GroupUnitsAction::Begin(void)
{
	if( TheBTCursor )
	{
		if( TheInterfaceMgr->GetCommandKey() == false )
		{
			TheBTCursor->SelectGroup( m_Group );
		}
		else
		{
			TheBTCursor->GroupSelectedUnits( m_Group );
		}
	}
}


void FieldCommanderAction::Begin()
{

}

void FieldCommanderAction::End()
{
	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( player == nullptr ) return;

	CommanderController *cont = player->GetPlayerController();
	if( cont == nullptr ) return;

	TheMessageMgr->SendMessage( kPlayerServer, EntityAbilityMessage( cont->GetControllerIndex(), m_lAbility ));
}

void SwitchBTModesAction::Begin(void)
{
	// if player in command mode
	GameWorld *world = static_cast<GameWorld *>( TheWorldMgr->GetWorld() );
	if( world == nullptr ) return;

	if( world->GetCamera() == &world->GetCommanderCamera() ) { // in Command mode

		TheGame->ToFieldMode( TheInterfaceMgr->GetShiftKey() );

	} else {

		TheGame->ToCommandMode();
	}
}


void ShowCursorAction::Begin(void)
{
	if( (TheInputMgr->GetInputMode() & kInputMouseActive) == 1 ) {

		m_bActivated = true;

		TheInputMgr->SetInputMode( kInputKeyboardActive ); // mouse will be handled by the interface manager
		TheInterfaceMgr->SetInputManagementMode( kInputManagementCommandMode ); // to prevent the interface manager from hiding the cursor when windows are closed

		TheBTCursor->ChangeState(TheDefaultState, Point(0.0F,0.0F) );
	}
}

void ShowCursorAction::End(void)
{
	if( m_bActivated ) {

		TheInterfaceMgr->HideCursor();
		TheInterfaceMgr->SetInputManagementMode(kInputManagementAutomatic);
		TheInterfaceMgr->SetInputMode();

		TheBTCursor->ChangeState(nullptr, Point() );
	}

	m_bActivated = false;
}

