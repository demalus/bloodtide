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

#ifndef BTLobby_h_
#define BTLobby_h_

#include "MGInterface.h"

#define OPT_LENGTH 10 // maximum text length for an option in the options popup menu

/* For future reference: 

-- The Pick Box for an interface is a union of all of its sub element's pick boxes...
    So there is no need to do SetElementSize on an interface, but it must be done for every non-interface.

-- Every Element that wants to be interacted with must have a PartCode.

*/

namespace C4
{
	// ElementType - so that HandleElementTrigger() on its owning window can determine which elemented was triggered
	enum
	{
		kElementPlayerSlot		= 'pslt',
		kElementOptionsIcon		= 'popt'
	};

	enum PlayerSlotOperations
	{
		kOperationJoin			= 'join',
		kOperationOpen			= 'open',
		kOperationClose			= 'clos',
		kOperationKick			= 'kick'
	};

	enum TeamLobbies
	{
		kLobbyUndecided,
		kLobbyTeam1,
		kLobbyTeam2
	};

	enum LobbyUpdateStatus
	{
		kLobbyStatusLoadWorld	= 0,
		kLobbyStatusNothing,
		kLobbyStatusStart
	};

	typedef Array<String<OPT_LENGTH>> OptionsArray;

	// forward declerations
	class TeamLobby;


	/*
	* A Team Lobby is composed of Player Slots -- one for each player on the team.
	* A Player Slot can be in one of three states: open, closed or occupied.
	* @author Chris Williams
	*/
	class PlayerSlot : public Interface
	{
		private:

			TeamLobby					*m_pTeamLobby;

			long						m_lSlotID; // used to determine which slot a player is interacting with
			PlayerKey					m_PlayerKey;

			const char					*m_strDefaultName;

			TextElement					m_objName;
			ImageButtonElement			m_objBackground;
		
		public:

			PlayerSlot(TeamLobby *pTeamLobby, const char *strBkgd, long lSlotID);

			// Accessors
			TeamLobby *GetTeamLobby(void) { return m_pTeamLobby; }
			PlayerKey GetPlayerKey(void) { return m_PlayerKey; }
			long GetID(void) { return m_lSlotID; }

			bool IsOpen(void);
			bool IsOccupied(void);
			//

			// Operations on a Player Slot
			void SetPlayer(Player &player); 
			void RemovePlayer(Player &player); // Used to clean up a slot if the player moved or was kicked. Player parameter must match player currently in slot.

			bool OpenSlot(void); // Has no effect unless the slot is in the closed state.
			bool CloseSlot(void); // Prevents anyone from joining the slot. Closing an occupied slot will result in the occupant being kicked.
			bool KickPlayer(void); // Has no effect unless the slot is occupied.

			void SetDefaultName(const char* strDefaultName);
			//

			void ReceiveLobbyUpdate(PlayerKey playerKey);


			PartCode TestPickPoint(const Point3D &p) const { return 1; }

			bool HandleTrigger( void ) const;
			void HandleElementTrigger( Element *element );
	};

	/* 
	* An Options Popup Menu is a popup menu containing a list of all of the options that can be executed on a Player Slot. 
	* Each Team Lobby will instantiate one that all of a Team Lobby's Options Icons will share. 
	* The Options Icon that displays this will handle interaction.
	* @author Chris Williams
	*/
	class OptionsMenu : public Interface
	{
		friend class OptionsIcon;

		private:

			Array<TextElement*>			m_arrOptions;

			// Options Icons will need to query this information to define bounding boxes for each option.
			float						m_fFontHeight;
			float						m_fFontWidth;
			float						m_fFontScale;

		public:

			OptionsMenu(OptionsArray &arrOptions);
			~OptionsMenu();
	};

	/* 
	* An Options Icon is a button next to each Player Slot that will display the Options Popup Menu when pressed.
	* The Options Icon handles interactions with the menu.
	* @author Chris Williams
	*/
	class OptionsIcon : public Interface
	{
		private:

			PlayerSlot					*m_objPlayerSlot;
			OptionsMenu					*m_pOptionsMenu;

			// The Blood Tide Lobby will query for this when the Options Icon triggers itself,
			// to determine which option was selected.
			String<OPT_LENGTH>			m_strSelected; 

			// Current option that the cursor is hovering over
			TextElement					*m_pCurrentOption;
	
			QuadElement					m_objBackground;

		public:

			OptionsIcon(PlayerSlot *objPlayerSlot);
			~OptionsIcon();

			void SetPlayerSlotOptions(OptionsArray &arrOptions);

			// Accessors
			PlayerSlot *GetPlayerSlot(void) { return m_objPlayerSlot; }
			String<OPT_LENGTH> GetSelected(void) { return m_strSelected; }
			//
	

			PartCode TestPickPoint(const Point3D &p) const { return 1; }
			void HandleMouseEvent(EventType objEventType, PartCode code, const Point3D& p);
	};

	/* 
	* A Team Lobby consits of a background image, a variable number of player slots, an options popup menu and
	* an options icon next to each player slot that will display the options popup menu.
	* @author Chris Williams
	*/
	class TeamLobby : public Interface
	{
		private:

			long							m_lLobbyID;
			int								m_nMaxPlayers;

			TextElement						m_objName;

			Array<PlayerSlot*>				m_arrPlayerSlots;
			Array<OptionsIcon*>				m_arrOptionsIcons;

			ImageElement					m_objBackground;

			// used by LobbyUpdateMessage -- it only cares about the player slots
			TeamLobby();

		public:

			TeamLobby(long lLobbyID, const char *strName, const char *strBkgd, const char *strSlot, int nMaxPlayers, OptionsArray &arrOptions);
			~TeamLobby();

			// Accessors
			PlayerSlot *GetPlayerSlotByID(long lSlotID);
			PlayerSlot *GetPlayerSlotByKey(PlayerKey key);
			OptionsIcon *GetOptionsIconByID(long lIconID);

			long GetID(void) { return m_lLobbyID; }
			int GetMaxPlayers(void) { return m_nMaxPlayers; }
			//

			bool IsEmpty(void);
			void CloseAllOpenSlots(void);

			// Places the player in the first available player slot, if no player slots are available, return false
			bool Join(Player &player); 


			Array<PlayerKey> *SendLobbyUpdate(void);
			void ReceiveLobbyUpdate(const Array<PlayerKey> &arrPlayerKeys);
	};

	/*
	* The game lobby for Blood Tide. 
	* It consists of two team lobbies and an undecided lobby for players who have not chosen a side.
	* @author Chris Williams
	*/
	class BloodTideLobby : public Interface, public Singleton<BloodTideLobby>
	{
		private:

			unsigned long				m_ulWorldID; // the worlds string table defines parameters for the game lobby

			OptionsArray				m_arrOptions; // the options that this lobby can execute on a player slot

			TextElement					m_objSelectTeam;

			TeamLobby					*m_pTeam1Lobby;
			TeamLobby					*m_pTeam2Lobby;
			TeamLobby					*m_pUndecidedLobby;

			ImageButtonElement			m_StartGameButton;
			ImageButtonElement			m_ExitGameButton;

			bool						m_bGameStarting;
			unsigned long				m_lStartDelay;
			unsigned long				m_ulStartTime;

			BloodTideLobby(unsigned long ulWorldID);

			// checks all three lobbies for the player
			PlayerSlot *FindPlayer(Player &player);

		public:

			~BloodTideLobby();
			static void New(unsigned long ulWorldID);

			unsigned long GetWorldID(void) { return m_ulWorldID; }
			TeamLobby *GetTeamLobbyByID(long LobbyID);


			bool PlayerConnected(Player &player);
			bool PlayerDisconnected(Player &player);

			bool RenamePlayer(Player &player);
			bool MovePlayer(Player &player, long lTeamLobbyID, long lPlayerSlotID);

			bool StartGameValid(void);
			void CloseAllOpenSlots(void);
			void StartGame(long lStartDelay);

			// sends an update of all three lobbies
			void SendSnapshot(unsigned long status);

			bool HandleTrigger(void) const;
			void HandleElementTrigger(Element *element);

			void UpdateDisplayPosition(void);

			void InterfaceTask(void);
	};
	
	extern BloodTideLobby *TheBloodTideLobby;

}

#endif
