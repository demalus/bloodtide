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


#ifndef BTVictoryController_h
#define BTVictoryController_h

#include "BTControllers.h"

namespace C4
{
	/**
	 * This determines how a game is won.
	 * This victory condition checks to see if all of a team's bases are destroyed, if so the game is over.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BasesDestroyed_VictoryController : public Controller
	{
		private:

			bool	m_bCheckTeam1;
			bool	m_bCheckTeam2;

			// Replication for world editor.
			BasesDestroyed_VictoryController( const BasesDestroyed_VictoryController& pController );
			Controller *Replicate( void ) const;

		public:

			BasesDestroyed_VictoryController();
			~BasesDestroyed_VictoryController();

			// Which nodes this controller can be placed on.
			static bool ValidNode( const Node *node );

			// Writing/reading to file (a world file).
			void Pack( Packer& data, unsigned long packFlags ) const;
			void Unpack( Unpacker& data, unsigned long unpackFlags );

			// Settings
			long GetSettingCount( void ) const;
			Setting *GetSetting( long index ) const;
			void SetSetting( const Setting *setting );

			virtual long CheckVictoryConditions( void );
			virtual void Setup( void );
	};
}

#endif
