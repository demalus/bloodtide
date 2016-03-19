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


#include "BTGoalTowerDefend.h"
#include "BTUnit.h"
#include "MGGame.h"
#include "C4Vector3D.h"
#include "BTMath.h"
#include "BTGoalMove.h"

using namespace C4;

BTGoalTowerDefend::BTGoalTowerDefend( BTEntityController *pOwner, float fRange, float fSpeed, float fDamage, const char *pSound ) :
	BTGoal( kGoalTypeTowerDefend ),
	m_pOwner( pOwner ),
	m_fRange( fRange ),
	m_fSpeed( fSpeed ),
	m_fDamage( fDamage ),
	m_pTarget( nullptr ),
	m_ulNextAttack( 0 ),
	m_pSound( pSound )
{
}

BTGoalTowerDefend::~BTGoalTowerDefend()
{

}

void BTGoalTowerDefend::Enter( void )
{
	BTGoal::Enter();

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return;
	}

	m_ulNextAttack = TheTimeMgr->GetAbsoluteTime();
}

void BTGoalTowerDefend::Exit( void )
{
	BTGoal::Exit();
}

long BTGoalTowerDefend::Process( void )
{
	BTGoal::Process();

	if( m_lStatus != kGoalInProgress ) {
		return m_lStatus;
	}

	if( !m_pOwner ) {
		m_lStatus = kGoalFailed;
		return m_lStatus;
	}

	unsigned long time = TheTimeMgr->GetAbsoluteTime();

	// Check to see if this building can attack now.
	if( time < m_ulNextAttack ) {
		return m_lStatus;
	}

	// If this tower no longer has a target or the target can't be attacked, pick a new one.
	if( !m_pTarget || !ValidTarget() ) {

		float closestEnemy = K::infinity;
		int enemyTeam = 3 - m_pOwner->GetTeam();
		Array<long> rIgnoreConts;
		BTCollisionData data;
		TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheUnits, enemyTeam), closestEnemy, m_fRange, rIgnoreConts, false );
		TheGame->DetectEntity( m_pOwner->GetPosition(), data, TheGameCacheMgr->GetGameCache(kCacheCommanders, enemyTeam), closestEnemy, m_fRange, rIgnoreConts, false );

		if( data.m_State == kCollisionStateCollider && data.m_pController != nullptr ) {
			m_pTarget = data.m_pController;
		}
		else {
			return m_lStatus;
		}
	}

	if( !m_pTarget ) {
		return m_lStatus;
	}

	// Set tower heading.
	Vector3D toEnemy = m_pTarget->GetPosition() - m_pOwner->GetPosition();
	float newAzi = atan2f( toEnemy.y, toEnemy.x );
	TheMessageMgr->SendMessageAll(EntityUpdateAzimuthMessage( m_pOwner->GetControllerIndex(), newAzi));

	// Play associated sound
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( world && m_pSound ) {
		TheMessageMgr->SendMessageAll( WorldSoundMessage(m_pSound, m_pOwner->GetPosition(), 16.0F) );
	}

	// Play the associated effect
	if( m_pOwner->GetControllerType() == kControllerBuilding )
	{
		BTCollisionData data;
		data.m_pController = m_pTarget;

		static_cast<BuildingController *>( m_pOwner.GetTarget() )->AttackEffect( data );
	}

	// Attack
	m_pTarget->Damage( m_fDamage, m_pOwner->GetControllerIndex() );

	// Set the next attack time.
	unsigned long ds = (m_fSpeed * 1000); // convert speed from seconds to milliseconds.
	m_ulNextAttack = time + ds;

	return m_lStatus;
}

bool BTGoalTowerDefend::ValidTarget( void )
{
	if( !m_pOwner ) {
		return false;
	}

	if( !m_pTarget ) {
		return false;
	}

	if( Calculate2DDistance(m_pTarget->GetPosition(), m_pOwner->GetPosition()) > m_fRange ) {
		return false;
	}

	if( !m_pOwner->CanAttack( m_pTarget ) ) {
		return false;
	}

	return true;
}

