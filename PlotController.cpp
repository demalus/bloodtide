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

PlotController::PlotController() : 
	BTEntityController( kControllerPlot ),
	m_BuildingType( nullptr )

{
	SetBaseControllerType( kControllerEntity );

	for( int i = 0; i < NUM_BUILDINGS; i++ ) {
		m_arrPossibleBuildings[i] = 0;
	}
}

PlotController::~PlotController()
{
	if( TheMessageMgr->Server() )
	{
		if( GetBuilding() )
		{
			TheMessageMgr->SendMessageAll( ControllerMessage( kEntityMessageDeath, GetBuilding()->GetControllerIndex() ) );
		}
	}

	if( TheGameCacheMgr	== nullptr )
		return;

	Array<long> *plotCache = TheGameCacheMgr->GetGameCache(kCachePlots, m_lTeam);

	if( plotCache && GetControllerIndex() >= 0 ) {
		long ix = plotCache->FindElement( GetControllerIndex() );
		if( ix >= 0 ) plotCache->RemoveElement( ix );
	}
}

// Needed as part of replicate to work in the world editor.
PlotController::PlotController( const PlotController& plotController ) : 
	BTEntityController( kControllerPlot )
{
	m_BuildingType = plotController.GetBuildingType();
	
	for( int i = 0; i < NUM_BUILDINGS; i++ ) {
		m_arrPossibleBuildings[i] = plotController.m_arrPossibleBuildings[i];
	}

	SetBaseControllerType( kControllerEntity );
}

// Needed to work in world editor.
Controller *PlotController::Replicate(void) const
{
	return (new PlotController(*this));
}

// This controller can only be placed on a marker node.
bool PlotController::ValidNode( const Node *node )
{
	if (node->GetNodeType() == kNodeMarker )
		return (true);

	return (false);
}

// This is used for packing all of a controller's information 
// into a single stream of bits (to be sent over a network or whatever).
void PlotController::Pack(Packer& data, unsigned long packFlags) const
{
	// Pack up the base-controller data.

	BTEntityController::Pack(data, packFlags);

	data << m_BuildingType;
	
	data << m_arrPossibleBuildings[0];
	data << m_arrPossibleBuildings[1];
	data << m_arrPossibleBuildings[2];
	data << m_arrPossibleBuildings[3];
}

// When received, this will take a bunch of bits and construct a controller out of them.
// This works by unpacking all of the chunks. A chunk isn't necessary, but it makes 
// things a bit cleaner. They are basically a way to arbitrary group portions of
// the data.
void PlotController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	BTEntityController::Unpack(data, unpackFlags);
	
	data >> m_BuildingType;

	data >> m_arrPossibleBuildings[0];
	data >> m_arrPossibleBuildings[1];
	data >> m_arrPossibleBuildings[2];
	data >> m_arrPossibleBuildings[3];
}

// The number of settings this controller will display in the world editor.
long PlotController::GetSettingCount(void) const
{
	return (5);
}

// Get the value of the setting (from what was entered in the world editor).
Setting *PlotController::GetSetting(long index) const
{
	const StringTable *table = TheGame->GetStringTable();

	if (index == 0)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerPlot, 'DFLT'));
		return (new TextSetting('DFLT', Text::TypeToString(m_BuildingType), title, 4, NULL));
	}
	
	if (index == 1)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerPlot, 'BLD1'));
		return (new TextSetting('BLD1', Text::TypeToString(m_arrPossibleBuildings[0]), title, 4, NULL));
	}
	
	if (index == 2)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerPlot, 'BLD2'));
		return (new TextSetting('BLD2', Text::TypeToString(m_arrPossibleBuildings[1]), title, 4, NULL));
	}
	
	if (index == 3)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerPlot, 'BLD3'));
		return (new TextSetting('BLD3', Text::TypeToString(m_arrPossibleBuildings[2]), title, 4, NULL));
	}
	
	if (index == 4)
	{
		const char *title = table->GetString(StringID('CTRL', kControllerPlot, 'BLD4'));
		return (new TextSetting('BLD4', Text::TypeToString(m_arrPossibleBuildings[3]), title, 4, NULL));
	}

	return (nullptr);
}

// World editor uses this to go from text field in editor to actually stored by this controller.
void PlotController::SetSetting(const Setting *setting)
{
	Type identifier = setting->GetSettingIdentifier();

	if( identifier == 'DFLT' ) 
	{
		m_BuildingType = Text::StringToType((static_cast<const TextSetting *>(setting))->GetText());
	}

	if( identifier == 'BLD1' ) 
	{
		m_arrPossibleBuildings[0] = Text::StringToType((static_cast<const TextSetting *>(setting))->GetText());
	}

	if( identifier == 'BLD2' ) 
	{
		m_arrPossibleBuildings[1] = Text::StringToType((static_cast<const TextSetting *>(setting))->GetText());
	}

	if( identifier == 'BLD3' ) 
	{
		m_arrPossibleBuildings[2] = Text::StringToType((static_cast<const TextSetting *>(setting))->GetText());
	}

	if( identifier == 'BLD4' ) 
	{
		m_arrPossibleBuildings[3] = Text::StringToType((static_cast<const TextSetting *>(setting))->GetText());
	}
}


void PlotController::Preprocess()
{
	BTEntityController::Preprocess();

	// Running in game, not the world editor
	if(!GetTargetNode()->GetManipulator())
	{
			if(GetTargetNode()->GetConnectedNode('BASE'))
			{
				SetBase(static_cast<BaseController*>(GetTargetNode()->GetConnectedNode('BASE')->GetController()));
				m_lTeam = m_myBase->GetTeam();
			}
	}

	SetCollider( new BTCylinderCollider( GetTargetNode()->GetNodePosition(), this, 1.5F, 0.5F ) );
	
	// Spawn the default building
	if(m_BuildingType != NULL)
	{
		//SpawnDefaultBuilding( m_BuildingType);
	}

}

void PlotController::Move()
{
	BTEntityController::Move();

	if(!m_myBase){
		//stuff
	}
}

void PlotController::SpawnBuilding(Type buildingType)
{
	// If this is not the server
	if(!TheMessageMgr->Server())
	{
		return;
	}

	BaseController *base = static_cast<BaseController*>( m_myBase.GetTarget() );
	if( !base ) {
		return;
	}

	Point3D pos = GetTargetNode()->GetWorldPosition();
	long newIndex = TheWorldMgr->GetWorld()->NewControllerIndex();
	long plotIndex = GetControllerIndex();

	//if the plot is empty
	if( !m_myBuilding && base->IsActiveBase() )
	{
		m_BuildingType = buildingType;
		long time = Text::StringToInteger(TheStringTableMgr->GetStringTable(kStringTableBuildings)->GetString(StringID(m_BuildingType, 'TIME')));
		TheMessageMgr->SendMessageAll(CreateBuildingMessage(kModelMessageBuilding, newIndex, buildingType, pos, 0.0F, m_lTeam, plotIndex, time));
	}

}

//void PlotController::SpawnDefaultBuilding(Type buildingType)
//{
//	// If this is not the server
//	if(!TheMessageMgr->Server())
//	{
//		return;
//	}
//
//	Point3D pos = GetTargetNode()->GetWorldPosition();
//	long newIndex = TheWorldMgr->GetWorld()->NewControllerIndex();
//	long plotIndex = GetControllerIndex();
//
//	TheMessageMgr->SendMessageAll(CreateBuildingMessage(kModelMessageBuilding, newIndex, buildingType, pos, 0.0F, m_lTeam, plotIndex));
//	m_BuildingType = buildingType;
//
//	// hide the plot
//	SetPerspectiveExclusionMask(kPerspectiveDirect);
//
//	// apply stat effect
//}


/* Destroys the building, making room for a new building */
void PlotController::DespawnBuilding()
{
	m_BuildingType = NULL;
	SetPerspectiveExclusionMask(0);
}

/* ---- Request Building Message ---- */

bool RequestBuildingMessage::HandleMessage(Player *sender) const
{
	Controller *controller = TheWorldMgr->GetWorld()->GetController( m_lPlotControllerID );
	if( controller && controller->GetControllerType() == kControllerPlot ) {

		PlotController *plot = static_cast<PlotController *>( controller );
		
		plot->SpawnBuilding( m_lBuildingID );
	}

	return (true);
}