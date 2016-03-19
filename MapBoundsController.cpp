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

#include "MGGame.h"

using namespace C4;

MapBoundsController::MapBoundsController() : 
	Controller( kControllerMapBounds ),
	m_topRight( 0, 0 )
{
}

MapBoundsController::~MapBoundsController()
{
}

// Needed as part of replicate to work in the world editor.
MapBoundsController::MapBoundsController(const MapBoundsController& pController) : 
	Controller( kControllerMapBounds )
{
	m_topRight = pController.m_topRight;
}

// Needed to work in world editor.
Controller *MapBoundsController::Replicate(void) const
{
	return (new MapBoundsController(*this));
}

// This controller can only be placed on a marker node.
bool MapBoundsController::ValidNode( const Node *node )
{
	return ( node->GetNodeType() == kNodeMarker );
}

// This is used for packing all of a controller's information 
// into a format to be saved in a world file..
void MapBoundsController::Pack(Packer& data, unsigned long packFlags) const
{
	// Pack up the base-controller data.
	Controller::Pack(data, packFlags);

	// Add each primitive component.
	data << m_topRight.x;
	data << m_topRight.y;
}

// When received, this will take a bunch of bits and construct a controller out of them.
void MapBoundsController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	Controller::Unpack(data, unpackFlags);

	data >> m_topRight.x;
	data >> m_topRight.y;
	
}

// The number of settings this controller will display in the world editor.
long MapBoundsController::GetSettingCount(void) const
{
	return (2);
}

// Get the value of the setting (from what was entered in the world editor).
Setting *MapBoundsController::GetSetting(long index) const
{
	const StringTable *table = TheGame->GetStringTable();

	if (index == 0)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerMapBounds, 'X'));
		return (new FloatSetting('X', m_topRight.x, title, 0.0F, 500.0F, 1.0F));
	}

	if (index == 1)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerMapBounds, 'Y'));
		return (new FloatSetting('Y', m_topRight.y, title, 0.0F, 500.0F, 1.0F));
	}

	return (nullptr);
}

// World editor uses this to go from text field in editor to actually stored by this controller.
void MapBoundsController::SetSetting(const Setting *setting)
{
	Type identifier = setting->GetSettingIdentifier();

	if( identifier == 'X' ) 
	{
		m_topRight.x = static_cast<const FloatSetting *>(setting)->GetFloatValue();
	}
	else if( identifier == 'Y' )
	{
		m_topRight.y = static_cast<const FloatSetting *>(setting)->GetFloatValue();
	}
}