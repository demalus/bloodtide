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


#ifndef BTGoal_h
#define BTGoal_h

#include "C4Array.h"

namespace C4
{

	enum GoalState
	{
		kGoalFailed		= 0,
		kGoalCompleted,
		kGoalInProgress,
		kGoalNotStarted
	};

	enum GoalType
	{
		kGoalTypeBasic		= 0,
		kGoalTypeAttack,
		kGoalTypeDefend,
		kGoalTypeFishWander,
		kGoalTypeMove,
		kGoalTypeThink,
		kGoalTypeFollow,
		kGoalTypeAttackTo,
		kGoalTypeTowerDefend
	};

	/** 
	 * Goals are behaviors and intentions that AI entities have.
	 * A goal may or may not have actions and subgoals associated with it.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTGoal
	{
		protected:

			// A goal can have any number of subgoals.
			Array<BTGoal*> m_arrSubGoals;

			long m_lStatus;

			GoalType m_ulGoalType;

		public:

			BTGoal( GoalType ulGoalType );
			virtual ~BTGoal();

			virtual void Enter( void );
			virtual void Exit( void );

			virtual long Process( void );

			GoalType GetGoalType( void )
			{
				return (m_ulGoalType);
			}

			virtual void AddSubgoal( BTGoal *pGoal );
			virtual void RemoveSubgoal( BTGoal *pGoal );
			virtual void RemoveAllSubgoals( void );

			long GetStatus( void )
			{
				return ( m_lStatus );
			}

			bool HasSubgoals( void )
			{
				return ( m_arrSubGoals.GetElementCount() > 0 );
			}
	};
}

#endif