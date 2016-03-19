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

#include "BTLobby.h"
#include "MGGame.h"
#include "BTLoadingTip.h"

// Element sizes
#define PLAYERSLOT_BKG_WIDTH 200.0F
#define PLAYERSLOT_BKG_HEIGHT 30.0F

#define OPT_ICON_WIDTH 10.0F
#define OPT_ICON_HEIGHT 10.0F
//

#define START_DELAY 13000.0F

using namespace C4;

BloodTideLobby *C4::TheBloodTideLobby = nullptr;

// Instead of sending both the state and the Player Key of a slot when sending updates to client,
// if a player is not in a slot, these Player Key variables will give the state of the slot.
const PlayerKey kSlotOpen	= -100;
const PlayerKey kSlotClosed = -99;

/* -------------- Player Slot -------------- */

PlayerSlot::PlayerSlot(TeamLobby *pTeamLobby, const char *strBkgd, long lSlotID) : Interface(kElementPlayerSlot),
							m_pTeamLobby(pTeamLobby),
							m_lSlotID(lSlotID),
							m_PlayerKey(kSlotOpen),
							m_strDefaultName("Open"),
							m_objName(m_strDefaultName),
							m_objBackground( PLAYERSLOT_BKG_WIDTH, PLAYERSLOT_BKG_HEIGHT, strBkgd )
{
	AddSubnode(&m_objBackground);

	/* ---- Text ---- */

	AutoReleaseFont font("font/Heading");

	m_objName.SetFont(font);
	m_objName.SetTextColor(K::white);
	m_objName.SetTextScale(1.2F);

	Point3D position( 10.0F, 10.0F, 0.0F );

	// center the font
	position.y = (PLAYERSLOT_BKG_HEIGHT / 2.0F) - (font->GetFontHeader()->fontHeight / 2.0F);

	m_objName.SetElementPosition(position);
	m_objName.SetElementSize(50.0F, 0.0F); // TODO: max_player_name_length?

	AddSubnode(&m_objName);
}

bool PlayerSlot::IsOpen(void) 
{ 
	return m_PlayerKey == kSlotOpen; 
}

bool PlayerSlot::IsOccupied(void)
{
	if( m_PlayerKey == kSlotOpen || m_PlayerKey == kSlotClosed )
		return false;

	return true;
}

void PlayerSlot::SetPlayer(Player &player)
{
	m_PlayerKey = player.GetPlayerKey();
	m_objName.SetText(player.GetPlayerName());
	
	GamePlayer *gamePlayer = static_cast<GamePlayer *>(&player);
	gamePlayer->SetPlayerTeam( m_pTeamLobby->GetID() );
}

void PlayerSlot::RemovePlayer(Player &player)
{
	if( m_PlayerKey == player.GetPlayerKey() ) {

		m_PlayerKey = kSlotOpen;
		m_objName.SetText(m_strDefaultName);
	}
}

bool PlayerSlot::OpenSlot(void)
{
	if( m_PlayerKey == kSlotClosed ) {

		m_PlayerKey = kSlotOpen;
		m_objName.SetText(m_strDefaultName);

		return true;
	}

	return false;
}

bool PlayerSlot::CloseSlot(void)
{
	if( m_PlayerKey != kSlotClosed ) {

		// if there is a player in the slot, he will be kicked from the server
		if( TheMessageMgr->GetPlayer(m_PlayerKey) ) {

			return KickPlayer();

		} else {

			m_PlayerKey = kSlotClosed;
			m_objName.SetText("Closed");

			return true;
		}
	}

	return false;
}

bool PlayerSlot::KickPlayer(void)
{
	Player *player = TheMessageMgr->GetPlayer(m_PlayerKey);

	// TODO: Dedicated Server
	if( player && player->GetPlayerKey() != kPlayerServer ) {

		TheMessageMgr->Disconnect(player);

		m_PlayerKey = kSlotOpen;
		m_objName.SetText(m_strDefaultName);

		return true;
	}

	return false;
}

void PlayerSlot::SetDefaultName(const char* strDefaultName)
{
	if( IsOpen() ) m_objName.SetText(strDefaultName);

	m_strDefaultName = strDefaultName;
}

bool PlayerSlot::HandleTrigger( void ) const
{
	return (true);
}

void PlayerSlot::HandleElementTrigger( Element *element )
{
	if( element && element->GetElementType() == kElementImageButton )
		TriggerElement();
}

void PlayerSlot::ReceiveLobbyUpdate(PlayerKey playerKey)
{
	if( playerKey != m_PlayerKey ) {

		m_PlayerKey = playerKey;

		switch( m_PlayerKey ) {

			case kSlotOpen:

				m_objName.SetText( m_strDefaultName );
				break;

			case kSlotClosed:

				m_objName.SetText( "Closed" );
				break;

			default:
			
				GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetPlayer(m_PlayerKey) );

				if( player ) {
		
					m_objName.SetText( player->GetPlayerName() );
					player->SetPlayerTeam( m_pTeamLobby->GetID() );
				}
		}
	}		
}

/* -------------- Options Menu -------------- */

OptionsMenu::OptionsMenu(OptionsArray &arrOptions)
{
	AutoReleaseFont font("font/Gui");

	m_fFontHeight = font->GetFontHeader()->fontHeight;
	m_fFontWidth = font->GetTextWidth("C");
	m_fFontScale = 1.5F;


	Point3D position(0.0F,0.0F,0.0F);

	int i;
	for(i=0; i < arrOptions.GetElementCount(); i++) {

		TextElement *text = new TextElement();


		text->SetTextColor(K::black);
		text->SetTextScale(m_fFontScale);
		text->SetText(arrOptions[i]);
		text->SetFont(font);

		text->SetElementSize( (m_fFontWidth * m_fFontScale * OPT_LENGTH), (m_fFontHeight * m_fFontScale) );
		text->SetElementPosition(position);


		m_arrOptions.AddElement(text);
		AddSubnode(text);

		position.y += m_fFontHeight * m_fFontScale;
	}
}

OptionsMenu::~OptionsMenu()
{

}

/* -------------- Options Icon -------------- */

OptionsIcon::OptionsIcon(PlayerSlot *objPlayerSlot) : Interface(kElementOptionsIcon),
								m_objPlayerSlot(objPlayerSlot),
								m_pOptionsMenu(nullptr),
								m_pCurrentOption(nullptr)
{
	/* ---- Background ---- */

	m_objBackground.SetElementSize(OPT_ICON_WIDTH, OPT_ICON_HEIGHT);
	m_objBackground.SetQuadColor(K::white);

	AddSubnode(&m_objBackground);

	/* ---- Options ---- */

	OptionsArray options;
	options.AddElement(String<OPT_LENGTH>("Cancel"));

	SetPlayerSlotOptions(options);
}

OptionsIcon::~OptionsIcon()
{
	//TheInterfaceMgr->RemoveElement(m_pOptionsMenu);

	if( m_pOptionsMenu ) delete m_pOptionsMenu;
}

void OptionsIcon::SetPlayerSlotOptions(OptionsArray &arrOptions)
{
	if( m_pOptionsMenu ) delete m_pOptionsMenu;

	m_pOptionsMenu = new OptionsMenu(arrOptions);
	m_pOptionsMenu->SetElementPosition( GetElementPosition() );	
}

void OptionsIcon::HandleMouseEvent(EventType EventType, PartCode code, const Point3D& p)
{
	// TODO: Dedicated Server
	// Only the host can use the options popup menu for a player slot
	if( m_pOptionsMenu == nullptr || TheMessageMgr->GetLocalPlayerKey() != kPlayerServer ) 
	{

		if( EventType == kEventRightMouseDown ) {

			m_objBackground.SetQuadColor( K::red );


		} else if( EventType == kEventRightMouseUp ) {

			m_objBackground.SetQuadColor(K::white);
		}

		return;
	}


	float fFontHeight = m_pOptionsMenu->m_fFontHeight;
	float fFontWidth = m_pOptionsMenu->m_fFontWidth;
	float fFontScale = m_pOptionsMenu->m_fFontScale;
	long lNumOptions = m_pOptionsMenu->m_arrOptions.GetElementCount();


	if( (EventType == kEventRightMouseDown) || (EventType == kEventMouseDown) ) { 	// Display the Options Popup Menu

		int width = fFontWidth * fFontScale * OPT_LENGTH;
		int height = lNumOptions * fFontHeight * fFontScale;

		m_objBackground.SetElementSize(width, height);

		// cursor will be over first option by default
		m_pCurrentOption = m_pOptionsMenu->m_arrOptions[0];
		m_pCurrentOption->SetTextColor(K::red);

		m_pOptionsMenu->SetElementPosition( GetWorldPosition() );
		TheInterfaceMgr->AddElement(m_pOptionsMenu);

	} else if( (EventType == kEventRightMouseUp) || (EventType == kEventMouseUp) || (EventType == kEventMouseMoved) ) {

		Point3D background = m_objBackground.GetElementPosition();

		// Is the cursor hovering over an option?
		if( p.x > background.x && p.x < background.x + m_objBackground.GetElementWidth() &&
			p.y > background.y && p.y < background.y + m_objBackground.GetElementHeight() )  {

			int selection = (int) ( p.y / (fFontHeight * fFontScale) );

			if( selection >= 0 && selection < lNumOptions ) { // cursor is hovering over an option

				TextElement *curOption = m_pOptionsMenu->m_arrOptions[selection];

				if( (EventType == kEventRightMouseUp) || (EventType == kEventMouseUp) ) { // execute the option

					m_pCurrentOption->SetTextColor(K::black);
					TheInterfaceMgr->RemoveElement(m_pOptionsMenu);
					m_objBackground.SetElementSize(OPT_ICON_WIDTH, OPT_ICON_HEIGHT);

					m_strSelected = m_pCurrentOption->GetText();
					TriggerElement();
				
				} else { // highlight only the current option
					
					 if( m_pCurrentOption && m_pCurrentOption != curOption ) 
						 m_pCurrentOption->SetTextColor(K::black);

					m_pCurrentOption = curOption;
					m_pCurrentOption->SetTextColor(K::red);
				}
			}

		} else { // cursor is not hovering over an option

			if( (EventType == kEventRightMouseUp) || (EventType == kEventMouseUp) ) { // hide the options popup menu

				m_objBackground.SetElementSize(OPT_ICON_WIDTH, OPT_ICON_HEIGHT);
				TheInterfaceMgr->RemoveElement(m_pOptionsMenu);
			}

			if( m_pCurrentOption ) { // stop highlighting the current option

				m_pCurrentOption->SetTextColor(K::black);
				m_pCurrentOption = nullptr;
			} 
		}
	}
}

/* -------------- Team Lobby -------------- */

TeamLobby::TeamLobby(long lLobbyID, const char *strName, const char *strBkgd, const char *strSlot, int nMaxPlayers, OptionsArray &arrOptions) :
								m_lLobbyID(lLobbyID),
								m_objName(strName),
								m_nMaxPlayers(nMaxPlayers),
								m_arrPlayerSlots(nMaxPlayers),
								m_arrOptionsIcons(nMaxPlayers)
{
	Point3D position(0.0F,0.0F,0.0F);

	AddSubnode(&m_objBackground);

	/* ---- Lobby Name ---- */

	AutoReleaseFont font("font/Gui");

	position.x = 25.0F;
	position.y += 15.0F;

	m_objName.SetFont(font);
	m_objName.SetTextScale(1.5F);
	m_objName.SetTextColor(K::white);

	m_objName.SetElementSize(50.0F, 0);
	m_objName.SetElementPosition( position );

	AddSubnode(&m_objName);

	position.y += 20.0F;


	/* ---- Player Slots and Options Icons ---- */

	PlayerSlot *playerSlot;
	OptionsIcon *optionsIcon;

	int i;
	for(i=0; i < nMaxPlayers; i++ ) {

		position.y += 10.0F;

		/* ---- Player Slot ---- */

		playerSlot = new PlayerSlot(this, strSlot, i);

		playerSlot->SetElementPosition( position );
		m_arrPlayerSlots.AddElement(playerSlot);
		AddSubnode(playerSlot);

		/* ---- Options Icon ---- */

		Point3D iconPosition;
		optionsIcon = new OptionsIcon(playerSlot);
		optionsIcon->SetPlayerSlotOptions(arrOptions);

		iconPosition.x = position.x + PLAYERSLOT_BKG_WIDTH + 10.0F;
		iconPosition.y = position.y + (PLAYERSLOT_BKG_HEIGHT / 2.0F) - (OPT_ICON_HEIGHT / 2.0F);
		iconPosition.z = 0.0F;

		optionsIcon->SetElementPosition(iconPosition);
		m_arrOptionsIcons.AddElement(optionsIcon);
		AddSubnode(optionsIcon);


		position.y += PLAYERSLOT_BKG_HEIGHT;
	}
	

	/* ---- Team Lobby Background ---- */

	float bkgdHeight;
	if( nMaxPlayers < 7 )
	{
		bkgdHeight = (10.0F + (PLAYERSLOT_BKG_HEIGHT * 1.5)) * 6.0F;
	}
	else
	{
		bkgdHeight = (10.0F + (PLAYERSLOT_BKG_HEIGHT * 1.5)) * nMaxPlayers;
	}

	m_objBackground.SetElementSize( PLAYERSLOT_BKG_WIDTH * 1.5, bkgdHeight + 20.0F );
	m_objBackground.SetTexture( strBkgd );

	SetElementSize( PLAYERSLOT_BKG_WIDTH * 1.5, position.y + 20.0F );
}

TeamLobby::~TeamLobby()
{
}

PlayerSlot *TeamLobby::GetPlayerSlotByID(long lSlotID)
{
	if( lSlotID >= 0 && lSlotID < m_arrPlayerSlots.GetElementCount() )
		return m_arrPlayerSlots[lSlotID];
	else
		return nullptr;
}

OptionsIcon *TeamLobby::GetOptionsIconByID(long lIconID)
{
	if( lIconID >= 0 && lIconID < m_arrOptionsIcons.GetElementCount() )
		return  m_arrOptionsIcons[lIconID];
	else
		return nullptr;
}

bool TeamLobby::Join(Player &player)
{
	PlayerSlot *playerSlot;

	int i;
	for(i=0; i < m_arrPlayerSlots.GetElementCount(); i++) {

		playerSlot = m_arrPlayerSlots[i];
		
		if( playerSlot->IsOpen() ) {

			playerSlot->SetPlayer(player);
			return true;
		}
	}

	return false;
}

PlayerSlot *TeamLobby::GetPlayerSlotByKey(PlayerKey key)
{
	PlayerSlot *playerSlot;

	int i;
	for(i=0; i < m_arrPlayerSlots.GetElementCount(); i++) {

		playerSlot = m_arrPlayerSlots[i];
		
		if( playerSlot->GetPlayerKey() == key )
			return playerSlot;
	}

	return nullptr;
}

bool TeamLobby::IsEmpty(void)
{
	PlayerSlot *playerSlot;

	int i;
	for(i=0; i < m_arrPlayerSlots.GetElementCount(); i++) {

		playerSlot = m_arrPlayerSlots[i];
		
		if( playerSlot->IsOccupied() == true )
			return false;
	}

	return true;
}

void TeamLobby::CloseAllOpenSlots(void)
{
	PlayerSlot *playerSlot;

	int i;
	for(i=0; i < m_arrPlayerSlots.GetElementCount(); i++) {

		playerSlot = m_arrPlayerSlots[i];
		
		if( playerSlot->IsOpen() )
			playerSlot->CloseSlot();
	}

}

void TeamLobby::ReceiveLobbyUpdate(const Array<PlayerKey> &arrPlayerKeys)
{
	if( arrPlayerKeys.GetElementCount() != m_arrPlayerSlots.GetElementCount() ) {
		
		return;
	}


	PlayerSlot *playerSlot;

	int i;
	for(i=0; i < m_arrPlayerSlots.GetElementCount(); i++) {

		playerSlot = m_arrPlayerSlots[i];
		
		playerSlot->ReceiveLobbyUpdate( arrPlayerKeys[i] );
	}
}

Array<PlayerKey> *TeamLobby::SendLobbyUpdate(void)
{
	Array<PlayerKey> *playerKeys = new Array<PlayerKey>;
	PlayerSlot *playerSlot;

	int i;
	for(i=0; i < m_arrPlayerSlots.GetElementCount(); i++) {

		playerSlot = m_arrPlayerSlots[i];
		playerKeys->AddElement( playerSlot->GetPlayerKey() );
	}

	return playerKeys;
}

/* -------------- Blood Tide Lobby -------------- */

BloodTideLobby::BloodTideLobby(unsigned long ulWorldID) : Interface(),
				Singleton<BloodTideLobby>(TheBloodTideLobby),
				m_ulWorldID(ulWorldID),
				m_StartGameButton( 75.0F, 30.0F, "texture/ui/lobby/StartButton" ),
				m_ExitGameButton( 75.0F, 30.0F, "texture/ui/lobby/ExitButton" ),
				m_bGameStarting(false),
				m_lStartDelay(0),
				m_ulStartTime(0)
{

	/* ---- Get a bunch of lobby parameters from the worlds string table ---- */

	const char *strTemporary;

	const char *strTeam1Name = "";
	const char *strTeam1Bkgd = "";
	const char *strTeam1Slot = "";
	int	nTeam1Players = 1;

	const char *strTeam2Name = "";
	const char *strTeam2Bkgd = "";
	const char *strTeam2Slot = "";
	int	nTeam2Players = 1;

	const char *strUndecidedName = "";
	const char *strUndecidedBkgd = "";
	const char *strUndecidedSlot = "";

	if( TheStringTableMgr )
	{
		const StringTable *table = TheStringTableMgr->GetStringTable( kStringTableWorlds );
		if( table )
		{
			strTeam1Name = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA1', 'NAME'));
			strTeam1Bkgd = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA1', 'LOBB'));
			strTeam1Slot = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA1', 'SLOT'));

			strTemporary = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA1', 'NUMP'));
			nTeam1Players = Text::StringToInteger(strTemporary);

			strTeam2Name = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA2', 'NAME'));
			strTeam2Bkgd = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA2', 'LOBB'));
			strTeam2Slot = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA2', 'SLOT'));

			strTemporary = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'TEA2', 'NUMP'));
			nTeam2Players = Text::StringToInteger(strTemporary);

			strUndecidedName = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'UNDE', 'NAME'));
			strUndecidedBkgd = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'UNDE', 'LOBB'));
			strUndecidedSlot = TheStringTableMgr->GetStringTable(kStringTableWorlds)->GetString(StringID(ulWorldID, 'INFO', 'UNDE', 'SLOT'));
		}
	}

	/* ---- Create the options that the Blood Tide Lobby will handle ---- */

	m_arrOptions.AddElement( "Open" );
	m_arrOptions.AddElement( "Close" );
	m_arrOptions.AddElement( "Kick" );
	m_arrOptions.AddElement( "Cancel" );

	/* ---- Team Lobbies ---- */

	m_pUndecidedLobby = new TeamLobby(0, strUndecidedName, strUndecidedBkgd, strUndecidedSlot, nTeam1Players + nTeam2Players, m_arrOptions);
	AddSubnode(m_pUndecidedLobby);

	m_pTeam1Lobby = new TeamLobby(1, strTeam1Name, strTeam1Bkgd, strTeam1Slot, nTeam1Players, m_arrOptions);
	AddSubnode(m_pTeam1Lobby);

	m_pTeam2Lobby = new TeamLobby(2, strTeam2Name, strTeam2Bkgd, strTeam2Slot, nTeam2Players, m_arrOptions);
	AddSubnode(m_pTeam2Lobby);

	/* The first slot in the lobby is for the main Commander */
	m_arrOptions.RemoveElement(0);
	m_arrOptions.RemoveElement(0);

	OptionsIcon *icon = m_pTeam1Lobby->GetOptionsIconByID(0);
	if( icon ) icon->SetPlayerSlotOptions(m_arrOptions);

	icon = m_pTeam2Lobby->GetOptionsIconByID(0);
	if( icon ) icon->SetPlayerSlotOptions(m_arrOptions);

	/* ---- Start/Exit Buttons ---- */

	// TODO: Dedicated server
	if( TheMessageMgr->GetLocalPlayerKey() != kPlayerServer ) {

		m_StartGameButton.Disable();
		m_StartGameButton.Hide();
	}

	AddSubnode(&m_StartGameButton);
	AddSubnode(&m_ExitGameButton);

	/* ---- Select Team ---- */

	m_objSelectTeam.SetText( "Select Team" );
	m_objSelectTeam.SetFont( AutoReleaseFont("font/help") );
	m_objSelectTeam.SetTextColor( K::white );
	m_objSelectTeam.SetElementSize( 30.0F, 10.0F );
	m_objSelectTeam.SetTextScale( 2.5F );

	AddSubnode( &m_objSelectTeam );


	UpdateDisplayPosition();
}

void BloodTideLobby::UpdateDisplayPosition(void)
{
	float width = TheDisplayMgr->GetDisplayWidth();
	float height = TheDisplayMgr->GetDisplayHeight();

	float half_width = width / 2.0F;

	m_objSelectTeam.SetElementPosition( Point3D( half_width - 100.0F, 15.0F, 0.0F ) ); 

	float half_undecided = m_pUndecidedLobby->GetElementWidth() / 2.0F;

	m_pUndecidedLobby->SetElementPosition( Point3D( half_width - half_undecided, 250.0F, 0.0F) );

	m_pTeam1Lobby->SetElementPosition( Point3D( half_width - m_pTeam1Lobby->GetElementWidth() - half_undecided - 15.0F, 75.0F, 0.0F) );
	m_pTeam2Lobby->SetElementPosition( Point3D( half_width + half_undecided + 15.0F, 75.0F, 0.0F) );

	m_StartGameButton.SetElementPosition( Point3D( width - 250.0F, height - 100.0F, 0.0F) );
	m_ExitGameButton.SetElementPosition( Point3D(width - 150.0F, height - 100.0F, 0.0F) );

	Invalidate();
}


BloodTideLobby::~BloodTideLobby()
{

}

void BloodTideLobby::New(unsigned long ulWorldID)
{
	if( TheBloodTideLobby == nullptr )
		TheInterfaceMgr->AddElement( new BloodTideLobby(ulWorldID) );
}

TeamLobby *BloodTideLobby::GetTeamLobbyByID(long LobbyID)
{
	if( LobbyID == kLobbyTeam1 )
		return m_pTeam1Lobby;

	if( LobbyID == kLobbyTeam2 )
		return m_pTeam2Lobby;

	if( LobbyID == kLobbyUndecided )
		return m_pUndecidedLobby;

	return nullptr;
}

PlayerSlot *BloodTideLobby::FindPlayer(Player &player)
{
	PlayerKey key = player.GetPlayerKey();
	PlayerSlot *playerSlot;

	if( (playerSlot = m_pTeam1Lobby->GetPlayerSlotByKey(key)) != nullptr )
		return playerSlot;

	if( (playerSlot = m_pTeam2Lobby->GetPlayerSlotByKey(key)) != nullptr )
		return playerSlot;

	if( (playerSlot = m_pUndecidedLobby->GetPlayerSlotByKey(key)) != nullptr )
		return playerSlot;

	return nullptr;
}

bool BloodTideLobby::PlayerConnected(Player &player)
{
	return m_pUndecidedLobby->Join(player);
}

bool BloodTideLobby::PlayerDisconnected(Player &player)
{
	PlayerSlot *playerSlot = FindPlayer(player);

	if( playerSlot ) {

		playerSlot->RemovePlayer(player);
		return true;
	}

	return false;
}

bool BloodTideLobby::RenamePlayer(Player &player)
{
	PlayerSlot *playerSlot = FindPlayer(player);

	if( playerSlot ) {

		playerSlot->SetPlayer(player);
		return true;
	}

	return false;
}

bool BloodTideLobby::MovePlayer(Player &player, long lTeamLobbyID, long lPlayerSlotID) {

	TeamLobby *lobby = GetTeamLobbyByID( lTeamLobbyID );

	if( lobby ) {

		PlayerSlot *newPlayerSlot = lobby->GetPlayerSlotByID( lPlayerSlotID );
		PlayerSlot *oldPlayerSlot;

		if( newPlayerSlot && newPlayerSlot->IsOpen() ) {

			oldPlayerSlot = FindPlayer(player);

			if( oldPlayerSlot ) {
		
				oldPlayerSlot->RemovePlayer(player);
			}

			newPlayerSlot->SetPlayer(player);

			return true;
		}
	}

	return false;
}

bool BloodTideLobby::StartGameValid(void) // TODO: Make a tip element and attach it to the start button
{
	/* TODO: To make testing easier
	/*if( m_pTeam1Lobby->GetMaxPlayers() > 0 ) {

		if( m_pTeam1Lobby->GetPlayerSlotByID(0)->IsOccupied() == false )
			return false;
	}

	if( m_pTeam2Lobby->GetMaxPlayers() > 0 ) {

		if( m_pTeam2Lobby->GetPlayerSlotByID(0)->IsOccupied() == false )
			return false;
	}*/

	if( m_pUndecidedLobby->IsEmpty() == false )
		return false;

	return true;
}

void BloodTideLobby::CloseAllOpenSlots(void)
{
	m_pTeam1Lobby->CloseAllOpenSlots();
	m_pTeam2Lobby->CloseAllOpenSlots();
	m_pUndecidedLobby->CloseAllOpenSlots();
}

void BloodTideLobby::StartGame(long lStartDelay)
{
	m_bGameStarting = true;
	m_lStartDelay = lStartDelay;

	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();
}

void BloodTideLobby::SendSnapshot(unsigned long status)
{
	// Broadcast a snapshot of the lobby
	Array<PlayerKey> *team1Keys = TheBloodTideLobby->GetTeamLobbyByID(kLobbyTeam1)->SendLobbyUpdate();
	Array<PlayerKey> *team2Keys = TheBloodTideLobby->GetTeamLobbyByID(kLobbyTeam2)->SendLobbyUpdate();
	Array<PlayerKey> *undecidedKeys = TheBloodTideLobby->GetTeamLobbyByID(kLobbyUndecided)->SendLobbyUpdate();

	TheMessageMgr->SendMessageAll( LobbyUpdateMessage( status, *team1Keys, *team2Keys, *undecidedKeys), true );
}

bool BloodTideLobby::HandleTrigger(void) const
{
	return (true);
}

void BloodTideLobby::HandleElementTrigger(Element *element)
{
	if( element == &m_ExitGameButton ) { // exit back to main game screen

		TheGame->ExitCurrentGame(true);
		MainWindow::Open();
		return;
	}

	if( m_bGameStarting == true ) 
		return;

	if( element->GetElementType() == kElementPlayerSlot ) { // send a join slot message to server

		PlayerSlot *playerSlot = static_cast<PlayerSlot*>(element);

		long lPlayerSlotID = playerSlot->GetID();
		long lTeamLobbyID = playerSlot->GetTeamLobby()->GetID();

		TheMessageMgr->SendMessage(kPlayerServer, 
			LobbyModifySlotMessage(kOperationJoin, lTeamLobbyID, lPlayerSlotID));
	}

	// TODO: Dedicated server
	if( TheMessageMgr->Server() == false )
		return;

	if( element == &m_StartGameButton ) { // start the game if valid and notify all clients

		if( StartGameValid() == true ) {

			CloseAllOpenSlots();
			SendSnapshot( kLobbyStatusLoadWorld );
		}
	}

	else if( element->GetElementType() == kElementOptionsIcon ) { // send a open, close or kick message to server

		OptionsIcon *options = static_cast<OptionsIcon*>(element);
		String<OPT_LENGTH> selected = options->GetSelected();

		long operation;

		if( selected == "Open" )
			operation = kOperationOpen;

		else if( selected == "Close" )
			operation = kOperationClose;

		else if( selected == "Kick" )
			operation = kOperationKick;

		else
			return;


		long lPlayerSlotID = options->GetPlayerSlot()->GetID();
		long lTeamLobbyID = options->GetPlayerSlot()->GetTeamLobby()->GetID();


		TheMessageMgr->SendMessage(kPlayerServer, 
				LobbyModifySlotMessage(operation, lTeamLobbyID, lPlayerSlotID));
	}
}

void BloodTideLobby::InterfaceTask(void)
{
	if( m_bGameStarting ) {

		unsigned long curTime = TheTimeMgr->GetAbsoluteTime();

		if( (curTime - m_ulStartTime) >= m_lStartDelay ) {

			m_bGameStarting = false;
			TheBloodTideLobby->Hide();

			TheGame->InitializeBloodTideSession();

		} else {

			//m_StartGameButton.SetText( String<>( m_lStartDelay - (curTime - m_ulStartTime) ) );

		}
	}
}

/* ---- Blood Tide Lobby Messages ---- */


bool LobbyModifySlotMessage::HandleMessage(Player *sender) const
{
	if( TheBloodTideLobby == nullptr ) {

		return (true);
	}

	bool lobbyModified = false; // if the lobby does not get modified, don't bother sending an update!


	if( m_lOperation == kOperationJoin ) {

		if( m_lTeamLobbyID == -1 ) {

			lobbyModified = TheBloodTideLobby->PlayerConnected(*sender);

			if( lobbyModified  == false ) {
				// Users who can't join the undecided lobby get the boot =(
				TheMessageMgr->Disconnect(sender);
			} 

		} else {

			lobbyModified  = TheBloodTideLobby->MovePlayer(*sender, m_lTeamLobbyID, m_lPlayerSlotID);
		}
	}


	// TODO: Dedicated Server
	if( sender->GetPlayerKey() == kPlayerServer ) {

		TeamLobby *teamLobby = TheBloodTideLobby->GetTeamLobbyByID( m_lTeamLobbyID );

		if( teamLobby == nullptr )
			return (true);

		PlayerSlot *playerSlot = teamLobby->GetPlayerSlotByID( m_lPlayerSlotID );

		if( playerSlot == nullptr )
			return (true);



		if( m_lOperation == kOperationOpen ) 
			lobbyModified = playerSlot->OpenSlot();

		else if( m_lOperation == kOperationClose ) 
			lobbyModified = playerSlot->CloseSlot();

		else if( m_lOperation == kOperationKick ) 
			lobbyModified = playerSlot->KickPlayer();

	}

	if( lobbyModified ) {

		TheBloodTideLobby->SendSnapshot( kLobbyStatusNothing );
	}
	

	return (true);
}


bool LobbyUpdateMessage::HandleMessage(Player *sender) const
{
	if( TheBloodTideLobby == nullptr )
		return (true);

	TeamLobby *pTeam1Lobby = TheBloodTideLobby->GetTeamLobbyByID(kLobbyTeam1);
	TeamLobby *pTeam2Lobby = TheBloodTideLobby->GetTeamLobbyByID(kLobbyTeam2);
	TeamLobby *pUndecidedLobby = TheBloodTideLobby->GetTeamLobbyByID(kLobbyUndecided);

	pTeam1Lobby->ReceiveLobbyUpdate( *m_arrTeam1Keys );
	pTeam2Lobby->ReceiveLobbyUpdate( *m_arrTeam2Keys );
	pUndecidedLobby->ReceiveLobbyUpdate( *m_arrUndecidedKeys );

	if( m_ulStatus == kLobbyStatusStart ) {
		TheBloodTideLobby->StartGame(START_DELAY);
	}
	else if( m_ulStatus == kLobbyStatusLoadWorld ) {

		const StringTable *worlds = TheStringTableMgr->GetStringTable( kStringTableWorlds );
		if( worlds ) {
			TheWorldMgr->LoadWorld( worlds->GetString(StringID(TheGame->GetWorldID())) );

			// Pass in random tips from string table.
			unsigned long teamid = 0;
			if( static_cast<GamePlayer*>(TheMessageMgr->GetLocalPlayer())->GetPlayerTeam() == 1 ) {
				teamid = 'TEA1';
			}
			else {
				teamid = 'TEA2';
			}

			const StringHeader *header = worlds->GetStringHeader( StringID(TheGame->GetWorldID(), 'INFO', teamid, 'LTIP' ) );
			if( header != nullptr ) {
				LoadingTip::New( header );
			}
		}

		TheMessageMgr->SendMessage( kPlayerServer, ClientMessage(kMessageWorldLoaded) );
	}

	return (true);
}

