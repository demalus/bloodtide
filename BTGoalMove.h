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


#ifndef BTGoalMove_h
#define BTGoalMove_h

#include "BTGoal.h"
#include "C4Vector3D.h"
#include "BTUnit.h"

#define DEFAULT_MAX_RADIUS_MULT		4.0F

namespace C4
{
	/** 
	 * Goal for moving to some position in the world.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTGoalMove : public BTGoal
	{
		private:

			Point3D m_p3dTarget;
			Point3D m_p3dInitialTarget;
			Link<UnitController> m_pOwner;

			float m_fMaxRadiusMult;

			void EvaluateTarget( void );

		public:

			BTGoalMove( Point3D& p3dTarget, UnitController *pOwner, float fMaxRadiusMult = DEFAULT_MAX_RADIUS_MULT );
			~BTGoalMove();

			virtual void Enter( void );
			virtual void Exit( void );

			virtual long Process( void );

			Point3D GetTarget( void ) const
			{
				return ( m_p3dTarget );
			}

			void SetTarget( Point3D& p3dTarget )
			{
				m_p3dTarget = p3dTarget;
			}
	};
}

#endif