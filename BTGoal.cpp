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


#include "BTGoal.h"


using namespace C4;

BTGoal::BTGoal( GoalType ulGoalType ) :
	m_lStatus( kGoalNotStarted ),
	m_ulGoalType( ulGoalType )
{
	m_arrSubGoals.SetElementCount( 0 );
}

BTGoal::~BTGoal()
{
	RemoveAllSubgoals();
}

void BTGoal::Enter( void )
{
	m_lStatus = kGoalInProgress;
}

void BTGoal::Exit( void )
{
	RemoveAllSubgoals();
}

// Processes all subgoals.
long BTGoal::Process( void )
{
	if( m_lStatus == kGoalNotStarted ) {
		Enter();
	}

	return m_lStatus;
}

void BTGoal::AddSubgoal( BTGoal *pGoal )
{
	m_arrSubGoals.AddElement( pGoal );
}

void BTGoal::RemoveSubgoal( BTGoal *pGoal )
{
	long index = m_arrSubGoals.FindElement( pGoal );
	if( index >= 0 ) {
		if( pGoal->GetStatus() != kGoalNotStarted ) {
			pGoal->Exit();
		}

		delete m_arrSubGoals[index];
		m_arrSubGoals.RemoveElement( index );
	}
}

void BTGoal::RemoveAllSubgoals( void )
{
	int nLength = m_arrSubGoals.GetElementCount();
	for( int x = 0; x < nLength; x++ ) {
		if( m_arrSubGoals[x]->GetStatus() != kGoalNotStarted ) {
			m_arrSubGoals[x]->Exit();
		}

		delete m_arrSubGoals[x];
	}

	m_arrSubGoals.Purge();
}

