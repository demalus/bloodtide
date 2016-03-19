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


#ifndef BTGoalAttackTo_h
#define BTGoalAttackTo_h

#include "BTGoal.h"
#include "C4Vector3D.h"
#include "C4Link.h"
#include "BTControllers.h"

namespace C4
{

	/** 
	 * Units wait around unless an enemy gets close.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTGoalAttackTo : public BTGoal
	{
		private:

			Point3D m_p3dTarget;				// Position to move to in order to attack an enemy.
			Link<BTEntityController> m_pOwner;

			Link<BTEntityController> m_pCurEnemy;	// Current enemy being targeted.

			void EvaluateConditions();

		public:

			BTGoalAttackTo( BTEntityController *pOwner, Point3D p3dTarget );
			~BTGoalAttackTo();

			virtual void Enter( void );
			virtual void Exit( void );

			virtual long Process( void );
	};
}

#endif