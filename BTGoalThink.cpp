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


#include "BTGoalThink.h"

using namespace C4;

BTGoalThink::BTGoalThink() :
	BTGoal( kGoalTypeThink )
{

}

BTGoalThink::~BTGoalThink()
{

}

void BTGoalThink::Enter( void )
{
	BTGoal::Enter();
}

void BTGoalThink::Exit( void )
{
	BTGoal::Exit();
}

// Processes only the first subgoal.
long BTGoalThink::Process( void )
{
	BTGoal::Process();

	if( m_lStatus != kGoalInProgress ) {
		return m_lStatus;
	}

	if( HasSubgoals() ) {
		long status = m_arrSubGoals[0]->Process();

		if( status == kGoalFailed || status == kGoalCompleted ) {
			RemoveSubgoal( m_arrSubGoals[0] );
		}
	}

	return m_lStatus;
}
