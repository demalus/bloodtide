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


#ifndef MGMultiplayer_h
#define MGMultiplayer_h

#include "C4Messages.h"
#include "C4Resources.h"


namespace C4
{
	class Model;
	class CommanderController;

	/*
	* The Lobby Modify Slot Message can represent a join, open, close or kick command on a player slot.
	* @author Chris Williams
	*/
	class LobbyModifySlotMessage : public Message
	{
		friend class BloodTideLobby;
		friend class Game;

		private:

			 // see BTLobby::PlayerSlotOperations
			long						m_lOperation;

			// an ID of -1 is interpreted as a wildcard  -- this is used when the player first enters the lobby
			long						m_lTeamLobbyID;

			long						m_lPlayerSlotID;

			LobbyModifySlotMessage();

		public:

			LobbyModifySlotMessage(long lOperation, long lTeamLobbyID, long lPlayerSlotID);
			~LobbyModifySlotMessage() {};

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);

			bool HandleMessage(Player *sender) const;
	};

	/*
	* The Lobby Update Message is a snapshot of the entire lobby.
	* It contains the player keys for all slots of the three lobbies.
	* Lobbies that are closed or open use a special player key defined in BTLobby.cpp
	* Also a start game flag is piggybacked along with the lobby update.
	* @author Chris Williams
	*/
	class LobbyUpdateMessage : public Message
	{
		friend class BloodTideLobby;
		friend class Game;

		private:

			unsigned long					m_ulStatus;

			int								m_nTeam1Slots;
			Array<PlayerKey>				*m_arrTeam1Keys;

			int								m_nTeam2Slots;
			Array<PlayerKey>				*m_arrTeam2Keys;

			int								m_nUndecidedSlots;
			Array<PlayerKey>				*m_arrUndecidedKeys;

			LobbyUpdateMessage();

		public:

			LobbyUpdateMessage(unsigned long ulStatus,
								Array<PlayerKey> &arrTeam1Keys,
								Array<PlayerKey> &arrTeam2Keys,
								Array<PlayerKey> &arrUndecidedKeys );

			~LobbyUpdateMessage();

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);

			bool HandleMessage(Player *sender) const;
	};

	/*
	* Sent by clicking on one of the Commander icons.
	* If valid, a CreateCommanderMessage will be sent to everyone, and only the player who sent the message,
	* will be able to use the Commander.
	* TODO: A NACK
	* @author Chris Williams
	*/
	class RequestCommanderMessage : public Message
	{
		friend class Game;

		private:

			long					m_lBaseControllerID; // the base associated with the Commander the player wishes to spawn

			RequestCommanderMessage();

		public:

			RequestCommanderMessage(long lBaseControllerID);
			~RequestCommanderMessage() {};

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);

			bool HandleMessage(Player *sender) const;
	};

	/**
	 * Message for when the game is over - contains the winner.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class GameOverMessage : public Message
	{
		friend class Game;

		private:

			long	m_lWinner;

			GameOverMessage();

		public:

			GameOverMessage( long lWinner );
			~GameOverMessage() {};

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);

			bool HandleMessage(Player *sender) const;
	};

	/**
	 * Message to create a sound in the world.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class WorldSoundMessage : public Message
	{
		friend class Game;

		private:

			Point3D							m_p3dPosition;
			float							m_fRange;
			String<kMaxResourceNameLength>	m_strSound;

			WorldSoundMessage();

		public:

			WorldSoundMessage( const char *pSound, Point3D& p3dPosition, float fRange );
			~WorldSoundMessage() {};

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);

			bool HandleMessage(Player *sender) const;
	};

	/*
	* Sent by clicking on one of the Building icons that pop up after selecting a plot.
	* If valid, a CreateBuildingMessage will be sent to everyone, and only the player who sent the message,
	* will be able to use the Building.
	* TODO: A NACK
	* @author Chris Williams
	*/
	class RequestBuildingMessage : public Message
	{
		friend class Game;

		private:

			long					m_lPlotControllerID; // the plot the user wants to build upon
			long					m_lBuildingID; // the type of building to build

			RequestBuildingMessage();

		public:

			RequestBuildingMessage(long lPlotControllerID, long lBuildingID);
			~RequestBuildingMessage() {};

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);

			bool HandleMessage(Player *sender) const;
	};

	/*
	* Spawns a projectile in the world.
	* @author Chris Williams
	*/
	class CreateProjectileMessage : public CreateModelMessage
	{
		friend class CreateModelMessage;
		friend class Game;

		private:

			long					m_lTeam;
			unsigned long			m_ulEntityID;
			float					m_fAzimuth;
			float					m_fTargetAlt;

			CreateProjectileMessage();

		public:

			CreateProjectileMessage(long lIndex, unsigned long ulEntityID, long lTeam, const Point3D &position, float fAzimuth, float fTargetAlt);
			~CreateProjectileMessage();

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	/*
	* Used with snapshot senders to updates a BTEntity's position.
	* BTEntity will construct this message, but it will NOT handle this message.
	* If a subclass of BTEntity wants to use this message, it must handle the message itself.
	* @author Chris Williams
	* @author - Frank Williams (demalus@gmail.com)
	*/
	class EntityUpdatePositionMessage : public ControllerMessage
	{
		friend class BTEntityController;

	private:

		Point3D		updatePosition;

		EntityUpdatePositionMessage(long controllerIndex);

	public:

		EntityUpdatePositionMessage(long controllerIndex, const Point3D& position);
		~EntityUpdatePositionMessage();

		const Point3D& GetUpdatePosition(void) const
		{
			return (updatePosition);
		}

		void Compress(Compressor& data) const;
		bool Decompress(Decompressor& data);
	};

	/*
	* Used with snapshot senders to updates a BTEntity's azimuth.
	* @author - Chris Williams
	*/
	class EntityUpdateAzimuthMessage : public ControllerMessage
	{
		friend class BTEntityController;

		private:

			float	m_fUpdateAzimuth;

			EntityUpdateAzimuthMessage(long controllerIndex);

		public:

			EntityUpdateAzimuthMessage(long controllerIndex, float& fAzimuth);
			~EntityUpdateAzimuthMessage();

			float GetUpdateAzimuth(void) const
			{
				return (m_fUpdateAzimuth);
			}

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};


	/*
	* The EntityAbilityMessage notifies the server that a player has triggered an ability.
	* @author - Chris Williams
	*/
	class EntityAbilityMessage : public ControllerMessage
	{
		friend class BTEntityController;

		private:

			long						m_lAbility;	// corresponds with kMotionAbility1, 2, 3 etc. in EntityMotion in BTControllers.h

			EntityAbilityMessage(long controllerIndex);

		public:

			EntityAbilityMessage(long controllerIndex, long lAbility);
			~EntityAbilityMessage();

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	/**
	* CreateBuildingMessage
	* This message constructs a building object (on top of a plot)
	*/
	class CreateBuildingMessage : public CreateModelMessage
	{
		friend class CreateModelMessage;

		private:

			long				m_lTeam;
			long				m_lPlot;		// controller index
			long				m_lTime;		// construction time

			float				m_fInitialAzimuth;

			unsigned long		m_ulEntityID;

			CreateBuildingMessage();

		public:

			CreateBuildingMessage(ModelMessageType type, long index, unsigned long ulEntityID, const Point3D& position, float azm, long lTeam, long plotIndex, long lTime);
			~CreateBuildingMessage();

			float GetInitialAzimuth(void) const
			{
				return (m_fInitialAzimuth);
			}

			unsigned long GetEntityID(void) const
			{
				return (m_ulEntityID);
			}

			long GetTeam( void ) const
			{
				return (m_lTeam);
			}

			long GetPlot( void ) const
			{
				return (m_lPlot);
			}

			long GetTime( void ) const
			{
				return (m_lTime);
			}

			// for sending over networks
			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	/**
	 * The EntityPlayAnimationMessage is used to sync animations for clients.
	 * @author - Chris Williams
	 */
	class EntityPlayAnimationMessage : public ControllerMessage
	{
		friend class BTEntityController;

		private:

			long					m_lEntityMotion; // see EntityMotion in BTControllers.h

			EntityPlayAnimationMessage( long lControllerIndex );

		public:

			EntityPlayAnimationMessage( long lControllerIndex, long lEntityMotion );

			long GetMotion( void ) const
			{
				return (m_lEntityMotion);
			}

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	/**
	 * The EntityUpdateStatsMessage syncs entity stats for clients.
	 * @author - Chris Williams
	 */
	class EntityUpdateStatsMessage : public ControllerMessage
	{
		friend class BTEntityController;

		private:

			unsigned long			m_ulDirtyStats; // see StatsProtocol.h for IDs
			Array<float>			m_arrStats; // must be ordered starting at the value represented by the first bit ( 1 << 0 )

			EntityUpdateStatsMessage( long lControllerIndex );

		public:

			EntityUpdateStatsMessage( long lControllerIndex, unsigned long ulDirtyStats, Array<float>& arrStats );

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};
}


#endif
