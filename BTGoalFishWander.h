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


#ifndef BTGoalFishWander_h
#define BTGoalFishWander_h

#include "BTGoal.h"
#include "C4Link.h"
#include "C4Vector3D.h"

namespace C4
{
	// Forward declarations
	class SimpleFishController;

	/** 
	 * Goal for Wandering some entity.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTGoalFishWander : public BTGoal
	{
		private:

			SimpleFishController *m_pOwner;
			Point3D m_p3dTarget;

		public:

			BTGoalFishWander( SimpleFishController *pOwner );
			~BTGoalFishWander();

			virtual void Enter( void );
			virtual void Exit( void );

			virtual long Process( void );
	};
}

#endif