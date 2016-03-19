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

#include "BTCommanderUI.h"
#include "BTHUD.h"
#include "StatsProtocol.h"
#include "MGGame.h"

/* Constants for the Unit Icon Manager */
#define MAX_ICON_WIDTH 64
#define MAX_ICON_HEIGHT 64

#define MIN_ICON_WIDTH 32
#define MIN_ICON_HEIGHT 32

#define FC_ICON_WIDTH 128
#define FC_ICON_HEIGHT 128

#define ICON_BORDER_THICKNESS 3.0F
#define ICON_SPACING 1.0F

#define ICONS_PER_GROUP 5
#define ICON_GROUP_SPACEING 8.0F

/* ----------------------------------- */

/* Constants for the Commander Selector Interface */
#define CS_ICON_SIZE 64.0F
#define CS_HALF_SIZE 32.0F
#define CS_ICON_MAX 80.0F // how large the icon can grow to once selected
#define CS_INCR_TIME 300.0F // how long it takes the icon to grow to max size
/* ---------------------------------------------- */

using namespace C4;

UnitIconMgr *C4::TheUnitIconMgr = nullptr;
CommanderSelecter *C4::TheCommanderSelecter = nullptr;

/* -------------- Commander Selector Interface -------------- */

CommanderSelecter::CSIcon::CSIcon(long lBaseControllerID) : 
			ResizingImage( "", CS_ICON_SIZE, CS_ICON_MAX, CS_INCR_TIME ),
			m_lBaseControllerID( lBaseControllerID )
{
	World *world = TheWorldMgr->GetWorld();
	BaseController *controller = static_cast<BaseController *>( world->GetController(lBaseControllerID) );

	Type commanderType = controller->GetCommanderType();
	const StringTable *table = TheStringTableMgr->GetStringTable(kStringTableCommanders);

	const char *strIcon = table->GetString(StringID(commanderType, 'ICON'));

	m_objImage.SetTexture(strIcon);
	m_objImage.SetQuadAlpha( 0.2F );
}

PartCode CommanderSelecter::CSIcon::TestPickPoint(const Point3D &p) const
{
	return (kElementPartButton);
}

void CommanderSelecter::CSIcon::HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p)
{
	ResizingImage::HandleMouseEvent( eventType, code, p );

	if( eventType == kEventMouseUp ) {

		TheMessageMgr->SendMessage(kPlayerServer, RequestCommanderMessage(m_lBaseControllerID));
	}
	else if( eventType == kEventCursorEnter )
	{
		m_objImage.SetQuadAlpha( 1.0F );
	}
	else if( eventType == kEventCursorExit )
	{
		m_objImage.SetQuadAlpha( 0.2F );
	}
}

void CommanderSelecter::CSIcon::Disable( long lBaseControllerID )
{
	if( m_lBaseControllerID == lBaseControllerID )
	{
		m_objImage.SetQuadAlpha( 0.2F );
		Element::Disable();
	}
}

CommanderSelecter::CommanderSelecter(Array<long> &arrBaseIDs, float flRadius) : 
			Interface(),
			Singleton<CommanderSelecter>(TheCommanderSelecter)
{
	World *world = TheWorldMgr->GetWorld();
	if( !world ) return;

	// Select commander text
	m_objSelectCommander.SetText( "Select Commander" );
	m_objSelectCommander.SetFont( AutoReleaseFont("font/Gui") );
	m_objSelectCommander.SetTextColor( K::white );
	m_objSelectCommander.SetElementSize( 30.0F, 10.0F );
	m_objSelectCommander.SetTextScale( 1.2F );
	m_objSelectCommander.SetElementPosition( Point3D( -60.0F, -(flRadius * sin(K::pi_over_2) + CS_HALF_SIZE + 25.0F), 0.0F ) );

	AddSubnode( &m_objSelectCommander );

	// Cycle through all the base IDs and create a new list that only contains bases that spawn Commanders
	Array<long> validBaseIDs;

	for(long a = 0; a < arrBaseIDs.GetElementCount(); a++) {

		BaseController *controller = static_cast<BaseController *>( world->GetController( arrBaseIDs[a] ) );

		if( controller && controller->GetCommanderType() != 0 )
			validBaseIDs.AddElement( arrBaseIDs[a] );
	}
	//

	const StringTable *commanders = TheStringTableMgr->GetStringTable(kStringTableCommanders);

	long iconCount = validBaseIDs.GetElementCount();
	float angleBetween = K::two_pi / iconCount; // angle between icons in a circle
	float curAngle = K::pi_over_2;

	for(long a = 0; a < iconCount; a++) {

		CSIcon *icon = new CSIcon( validBaseIDs[a] );

		float x = (flRadius * cos(curAngle)) + CS_HALF_SIZE;
		float y = (flRadius * sin(curAngle)) + CS_HALF_SIZE;
		curAngle += angleBetween;

		icon->SetElementPosition( Point3D(-x, -y, 0.0F) );

		AddSubnode(icon);
		m_arrCommanderIcons.AddElement(icon);
	}

	Point3D position(0.0F, 0.0F, 0.0F);
	position.x = flRadius + CS_ICON_SIZE + 10.0F;
	position.y = (TheDisplayMgr->GetDisplayHeight() - PANE_CHAT_HEIGHT) - (flRadius + CS_ICON_SIZE);

	SetElementPosition(position);
}


void CommanderSelecter::New(Array<long> &arrBaseIDs, float flRadius)
{
	if( TheCommanderSelecter == nullptr )
	{
		CommanderSelecter *interface = new CommanderSelecter(arrBaseIDs, flRadius);
		TheInterfaceMgr->AddElement(interface);
	}
}

void CommanderSelecter::RemoveBase( long lBaseControllerID )
{
	long size = m_arrCommanderIcons.GetElementCount();
	for( long x = 0; x < size; x++ )
	{
		m_arrCommanderIcons[x]->Disable( lBaseControllerID );
	}
}

void CommanderSelecter::Show( void )
{
	Interface::Show();

	GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
	if( player )
	{
		CommanderController *cont = player->GetPlayerController();
		if( (cont == nullptr) || (cont->GetEntityFlags() & kEntityDead) )
		{
			m_objSelectCommander.SetText( "Select Commander" );
			m_objSelectCommander.SetTextScale( 1.2F );
		}
		else
		{
			m_objSelectCommander.SetText( "Reselect Commander" );
			m_objSelectCommander.SetTextScale( 1.0F );
		}
	}
}

void CommanderSelecter::InterfaceTask(void)
{

}

/* -------------- Unit Icon -------------- */

UnitIcon::UnitIcon() : Interface(),
				m_pStatsController(nullptr),
				m_objOutline(0.0F,0.0F,ICON_BORDER_THICKNESS),
				m_lControllerIndex(-1)
{
	/* ---- Unit Image ---- */

	AddSubnode(&m_objImage);

	/* ---- Unit Outline ---- */

	m_objOutline.SetElementPosition( Point3D(ICON_BORDER_THICKNESS,ICON_BORDER_THICKNESS,0.0F) );
	AddSubnode(&m_objOutline);
}

UnitIcon::~UnitIcon()
{

}

void UnitIcon::SetUnit(long lControllerIndex)
{
	Controller *controller = TheWorldMgr->GetWorld()->GetController(lControllerIndex);
	if( controller == nullptr )
		return;

	if( controller->GetControllerType() == kControllerUnit ) {

		UnitController *unitController = static_cast<UnitController*>(controller);
		m_pStatsController = unitController->GetStats();

		const StringTable *stringTable = TheStringTableMgr->GetStringTable( unitController->GetTableID() );
		const char *iconTexture = stringTable->GetString( StringID(unitController->GetStringID(), 'ICON') );

		m_objImage.SetTexture(iconTexture);

	} else if( controller->GetControllerType() == kControllerCommander ) {

		CommanderController *commanderController = static_cast<CommanderController*>(controller);
		m_pStatsController = commanderController->GetStats();

		const StringTable *stringTable = TheStringTableMgr->GetStringTable( commanderController->GetTableID() );
		const char *iconTexture = stringTable->GetString( StringID(commanderController->GetStringID(), 'ICON') );

		m_objImage.SetTexture(iconTexture);
	}

	m_lControllerIndex = lControllerIndex;
}

void UnitIcon::UpdateSize(float fWidth, float fHeight)
{
	m_objImage.SetElementSize(fWidth, fHeight);
	m_objOutline.SetElementSize(fWidth - 2*ICON_BORDER_THICKNESS, fHeight - 2*ICON_BORDER_THICKNESS);
}

void UnitIcon::Update(long lControllerIndex)
{
	if( m_lControllerIndex == lControllerIndex ) {

		delete this;
		return;
	}
}

void UnitIcon::InterfaceTask(void)
{
	// Unit has died
	if( m_pStatsController == nullptr )
		return;

	float maxHealth = m_pStatsController->GetMaxHealth();
	float curHealth = m_pStatsController->GetHealth();

	if( maxHealth < 0 ) maxHealth = 1;
	if( curHealth < 0 ) curHealth = 0;

	// Percentage of remaining health
	float perHealth = (curHealth / maxHealth);

	float RED = 0.0F;
	float GREEN = 0.0F;

	if( perHealth >= 0.5F ) {
	
		GREEN = 1.0F;
		RED = (1 - perHealth) * 2.0F;

	} else {

		RED = 1.0F;
		GREEN = perHealth * 2.0F;
	}

	m_objOutline.SetOutlineColor( ColorRGBA(RED, GREEN, 0.0F) );
}

/* -------------- Unit Icon Manager -------------- */

UnitIconMgr::UnitIconMgr() : Interface(),
				Singleton<UnitIconMgr>(TheUnitIconMgr),
				m_lUnitIconCount(0),
				m_fCurrentIconWidth(MAX_ICON_WIDTH),
				m_fCurrentIconHeight(MAX_ICON_HEIGHT)
{
	AddSubnode( &m_objUnitIcons );

	m_objSelectedGroup.SetFont( AutoReleaseFont("font/gui") );
	m_objSelectedGroup.SetTextColor(K::white);
	m_objSelectedGroup.SetTextScale(1.5F);
	m_objSelectedGroup.SetElementPosition( Point3D( -15.0F, -15.0F, 0.0F ) );

	AddSubnode( &m_objSelectedGroup );
}

void UnitIconMgr::UpdateDisplayPosition( void )
{
	RefreshIcons();
}

void UnitIconMgr::New(void)
{
	if( TheUnitIconMgr == nullptr )
	{
		UnitIconMgr *interface = new UnitIconMgr();
		// TheInterfaceMgr->AddElement(interface);
	}
}

bool UnitIconMgr::ShrinkIconSize(void)
{
	if( (m_fCurrentIconWidth <= MIN_ICON_WIDTH) && (m_fCurrentIconHeight <= MIN_ICON_HEIGHT) ) {

		return false;
	}

	m_fCurrentIconWidth /= 1.5F;
	m_fCurrentIconHeight /= 1.5F;

	if( m_fCurrentIconWidth < MIN_ICON_WIDTH ) m_fCurrentIconWidth = MIN_ICON_WIDTH;
	if( m_fCurrentIconHeight < MIN_ICON_HEIGHT ) m_fCurrentIconHeight = MIN_ICON_HEIGHT;

	return true;
}

void UnitIconMgr::RefreshIcons(void)
{
	Point3D position(0.0F, 0.0F, 0.0F);

	long iconCount = 0;
	UnitIcon *unitIcon = static_cast<UnitIcon*>( m_objUnitIcons.GetFirstSubnode() );

	while( unitIcon ) {

		iconCount++;

		unitIcon->SetElementPosition(position);
		unitIcon->UpdateSize(m_fCurrentIconWidth, m_fCurrentIconHeight);
		unitIcon->Show();

		/* ---- Calculate the starting position of the next icon ---- */

		position.x += m_fCurrentIconWidth;

		if( (iconCount + 5) % ICONS_PER_GROUP == 0 ) 
			position.x += ICON_GROUP_SPACEING;
		else 
			position.x += ICON_SPACING;

		/* ---- Check boundary conditions ---- */
		
		// if this row is filled, start placing icons on the next row
		if( position.x + (m_fCurrentIconWidth + ICON_SPACING) >= GetElementWidth() ) {

			position.x = 0.0F;
			position.y += m_fCurrentIconHeight;
			iconCount = 0;
		}

		// if all of the rows are completly filled and there is another icon to place, see if the icons can be shrunk
		if( unitIcon && (position.y + m_fCurrentIconHeight) >= GetElementHeight() ) {

			if( ShrinkIconSize() )
				RefreshIcons();

			break;
		}

		unitIcon = static_cast<UnitIcon*>( m_objUnitIcons.GetNextLevelNode(unitIcon) );
	}
}

void UnitIconMgr::AddUnitIcon(long lControllerIndex)
{
	Controller *controller = TheWorldMgr->GetWorld()->GetController(lControllerIndex);

	if( controller && controller->GetControllerType() == kControllerUnit ) {

		UnitIcon *icon = new UnitIcon();
		icon->SetUnit(lControllerIndex);
		icon->Hide();

		m_objUnitIcons.AddSubnode(icon);
		m_lUnitIconCount++;
	}
}

void UnitIconMgr::AddUnit(long lControllerIndex)
{
	AddUnitIcon(lControllerIndex);

	Pane& infoPane = TheBTHUD->GetPaneInfo();

	if( infoPane.ContainsElement( *this ) == false ) {

		infoPane.PurgePane();
		infoPane.InsertElement( *this );
	}

	RefreshIcons();
}

void UnitIconMgr::AddUnits(const Array<long> &arrControllerIndexes)
{
	for(int i=0; i < arrControllerIndexes.GetElementCount(); i++) {
		AddUnitIcon( arrControllerIndexes[i] );
	}

	Pane& infoPane = TheBTHUD->GetPaneInfo();

	if( infoPane.ContainsElement( *this ) == false ) {

		infoPane.PurgePane();
		infoPane.InsertElement( *this );
	}

	RefreshIcons();
}

void UnitIconMgr::RemoveUnit(long lControllerIndex)
{
	Element *element = m_objUnitIcons.GetFirstSubnode();
	while (element)
	{
		Element *next = element->Next();
		static_cast<UnitIcon *>( element )->Update(lControllerIndex);

		element = next;
	}
}
void UnitIconMgr::PurgeUnits(void)
{
	if( m_objUnitIcons.GetSubnodeCount() > 0 )
		m_objUnitIcons.PurgeSubtree();

	m_lUnitIconCount = 0;
	m_fCurrentIconWidth = MAX_ICON_WIDTH;
	m_fCurrentIconHeight = MAX_ICON_HEIGHT;
}

void UnitIconMgr::InterfaceTask(void)
{

}
