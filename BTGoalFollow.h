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


#ifndef BTGoalFollow_h
#define BTGoalFollow_h

#include "BTGoal.h"
#include "C4Vector3D.h"
#include "C4Link.h"
#include "BTControllers.h"

namespace C4
{

	/** 
	 * Units will follow a commander.
	 * If enemies are nearby, they will attack unless they are too far away from the commander.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTGoalFollow : public BTGoal
	{
		private:

			Link<BTEntityController> m_pOwner;		// Unit that is following
			Link<BTEntityController> m_pFollow;		// Entity that this unit is following.

			Link<BTEntityController> m_pCurEnemy;	// Current enemy being targeted.

			void CheckForEnemies( void );
			bool InRangeOfFollow( void );

		public:

			BTGoalFollow( BTEntityController *pOwner, BTEntityController *pFollow );
			~BTGoalFollow();

			virtual void Enter( void );
			virtual void Exit( void );

			virtual long Process( void );
	};
}

#endif