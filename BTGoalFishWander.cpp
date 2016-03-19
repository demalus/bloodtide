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


#include "BTGoalFishWander.h"
#include "BTFish.h"
#include "PointFinder.h"
#include "BTSteering.h"
#include "BTGoalMove.h"
#include "BTFish.h"

using namespace C4;

BTGoalFishWander::BTGoalFishWander( SimpleFishController *pOwner ) :
	BTGoal( kGoalTypeFishWander ),
	m_pOwner( pOwner )
{
	if( m_pOwner ) {
		m_p3dTarget = m_pOwner->GetPosition();
	}
}

BTGoalFishWander::~BTGoalFishWander()
{

}

void BTGoalFishWander::Enter( void )
{
	BTGoal::Enter();

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	m_p3dTarget = m_pOwner->GetPosition();
}

void BTGoalFishWander::Exit( void )
{
	BTGoal::Exit();

	m_pOwner->GetSteering()->SetSeekOn( false );
}

long BTGoalFishWander::Process( void )
{
	BTGoal::Process();

	if( m_lStatus != kGoalInProgress ) {
		return m_lStatus;
	}

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
	}

	if( m_pOwner->CloseToPosition2D( m_p3dTarget ) ) {
		Array<Point2D> points;
		PointFinder::FindSafePoint_Random( 1, points );
		if( points.GetElementCount() <= 0 ) {
			m_lStatus = kGoalFailed;
			return m_lStatus;
		}

		m_p3dTarget.x = points[0].x;
		m_p3dTarget.y = points[0].y;
		m_pOwner->GetSteering()->SetTarget( m_p3dTarget );
		m_pOwner->GetSteering()->SetSeekOn( true );
	}
	

	return m_lStatus;
}

