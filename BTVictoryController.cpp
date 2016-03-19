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

#include "BTVictoryController.h"
#include "MGGame.h"

using namespace C4;


//  ------------- Bases Destroyed ---------------

BasesDestroyed_VictoryController::BasesDestroyed_VictoryController() : 
	Controller( kControllerVictory ),
	m_bCheckTeam1( false ),
	m_bCheckTeam2( false )
{
}

BasesDestroyed_VictoryController::~BasesDestroyed_VictoryController()
{
}

// Needed as part of replicate to work in the world editor.
BasesDestroyed_VictoryController::BasesDestroyed_VictoryController(const BasesDestroyed_VictoryController& pController) : 
	Controller( kControllerVictory )
{
	m_bCheckTeam1 = pController.m_bCheckTeam1;
	m_bCheckTeam2 = pController.m_bCheckTeam2;
}

// Needed to work in world editor.
Controller *BasesDestroyed_VictoryController::Replicate(void) const
{
	return (new BasesDestroyed_VictoryController(*this));
}

// This controller can only be placed on a marker node.
bool BasesDestroyed_VictoryController::ValidNode( const Node *node )
{
	return ( node->GetNodeType() == kNodeMarker );
}

// This is used for packing all of a controller's information 
// into a format to be saved in a world file..
void BasesDestroyed_VictoryController::Pack(Packer& data, unsigned long packFlags) const
{
	// Pack up the base-controller data.
	Controller::Pack(data, packFlags);
}

// When received, this will take a bunch of bits and construct a controller out of them.
void BasesDestroyed_VictoryController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	Controller::Unpack(data, unpackFlags);
}

// The number of settings this controller will display in the world editor.
long BasesDestroyed_VictoryController::GetSettingCount(void) const
{
	return (0);
}

// Get the value of the setting (from what was entered in the world editor).
Setting *BasesDestroyed_VictoryController::GetSetting(long index) const
{
	return (nullptr);
}

// World editor uses this to go from text field in editor to actually stored by this controller.
void BasesDestroyed_VictoryController::SetSetting(const Setting *setting)
{
}

// Setup this victory controller based on world state.
void BasesDestroyed_VictoryController::Setup( void ) 
{
	Array<long> *bc1 = TheGameCacheMgr->GetGameCache( kCacheBases, 1 );
	if( bc1 ) {
		if( bc1->GetElementCount() > 0 ) {
			m_bCheckTeam1 = true;
		}
	}

	Array<long> *bc2 = TheGameCacheMgr->GetGameCache( kCacheBases, 2 );
	if( bc2 ) {
		if( bc2->GetElementCount() > 0 ) {
			m_bCheckTeam2 = true;
		}
	}
}

// If all of a team's bases are destroyed, that team loses and the other team wins.
// Return team that won or 0 if none.
long BasesDestroyed_VictoryController::CheckVictoryConditions()
{
	if( !TheGameCacheMgr ) {
		return 0;
	}

	if( m_bCheckTeam1 ) {
		Array<long> *bc1 = TheGameCacheMgr->GetGameCache( kCacheBases, 1 );
		if( bc1 ) {
			if( bc1->GetElementCount() == 0 ) {
				return 2;
			}
		}
	}

	if( m_bCheckTeam2 ) {
		Array<long> *bc2 = TheGameCacheMgr->GetGameCache( kCacheBases, 2 );
		if( bc2 ) {
			if( bc2->GetElementCount() == 0 ) {
				return 1;
			}
		}
	}

	return 0;
}