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


#ifndef BTScript_Activation_h
#define BTScript_Activation_h

#include "BTScript.h"
#include "C4StringTable.h"

namespace C4
{

	/**
	 * This script activates a specified base.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_ActivateBase : public EventScript
	{
		public:

			EventScript_ActivateBase( StringTable *pTable, unsigned long ulID );
			~EventScript_ActivateBase();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script deactivates a specified base.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_DeactivateBase : public EventScript
	{
		public:

			EventScript_DeactivateBase( StringTable *pTable, unsigned long ulID );
			~EventScript_DeactivateBase();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script activates the ability to spawn a commander.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_ActivateCommSpawn : public EventScript
	{
		public:

			EventScript_ActivateCommSpawn( StringTable *pTable, unsigned long ulID );
			~EventScript_ActivateCommSpawn();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script deactivates the ability to spawn a commander.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_DeactivateCommSpawn : public EventScript
	{
		public:

			EventScript_DeactivateCommSpawn( StringTable *pTable, unsigned long ulID );
			~EventScript_DeactivateCommSpawn();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script activates the background of the message interface.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_ActivateMsgBG : public EventScript
	{
		public:

			EventScript_ActivateMsgBG( StringTable *pTable, unsigned long ulID );
			~EventScript_ActivateMsgBG();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script deactivates the background of the message interface.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_DeactivateMsgBG : public EventScript
	{
		public:

			EventScript_DeactivateMsgBG( StringTable *pTable, unsigned long ulID );
			~EventScript_DeactivateMsgBG();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script activates the help tips in game.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_ActivateHelp : public EventScript
	{
		public:

			EventScript_ActivateHelp( StringTable *pTable, unsigned long ulID );
			~EventScript_ActivateHelp();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};

	/**
	 * This script deactivates the help tips in game.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class EventScript_DeactivateHelp : public EventScript
	{
		public:

			EventScript_DeactivateHelp( StringTable *pTable, unsigned long ulID );
			~EventScript_DeactivateHelp();

			// Begin script events.
			virtual void Start( void );

			// Return true if script is done, false otherwise.
			virtual bool CheckConditions( void );
	};
}


#endif
