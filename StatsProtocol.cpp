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

#include "StatsProtocol.h"
#include "MGMultiplayer.h"
#include "StatEffect.h"
#include "MGGame.h"

using namespace C4;

StatsProtocol::StatsProtocol(float fMaxHealth, float fHealth, float fStrength, float fArmor, float fMovementSpeed, float fAttackRange) :
	m_pOwner( nullptr )
{	
	m_fMaxHealth = fMaxHealth;
	m_fHealth = fHealth;
	m_fStrength = fStrength;
	m_fArmor = fArmor;
	m_fMovementSpeed = fMovementSpeed; 
	m_fAttackRange = fAttackRange;

	m_fEffectOnMaxHealth = 0.0F;
	m_fEffectOnStrength = 0.0F;
	m_fEffectOnArmor = 0.0F;
	m_fEffectOnMovementSpeed = 0.0F;
	m_fEffectOnAttackRange = 0.0F;

	m_ulDirtyStats = 0;
}

StatsProtocol::~StatsProtocol()
{
	long size = m_arrEffects.GetElementCount();
	for( long x = 0; x < size; x++ ) {

		StatEffect *statEffect = m_arrEffects[x];
		delete statEffect;
	}

	m_arrEffects.Purge();
}

/* ----------------- Mutate Base Stats ----------------*/

void StatsProtocol::SetBaseMaxHealth( float fNewMaxHealth )
{
	if( fNewMaxHealth != m_fMaxHealth )
	{
		m_fMaxHealth = Fmax( fNewMaxHealth, 0.0F );
		m_ulDirtyStats |= kStatMaxHealth;
	}
}

void StatsProtocol::SetHealth( float fNewHealth )
{
	if( fNewHealth != m_fHealth )
	{
		m_fHealth = Fmax( fNewHealth, 0.0F );
		m_ulDirtyStats |= kStatHealth;
	}
}

void StatsProtocol::AddHealth( float fAmount )
{
	if( fAmount != 0.0F )
	{
		m_fHealth = Fmax( Fmin( m_fHealth + fAmount, m_fMaxHealth ), 0.0F );
		m_ulDirtyStats |= kStatHealth;
	}
}

void StatsProtocol::RemoveHealth( float fAmount )
{
	if( fAmount != 0.0F )
	{
		m_fHealth = Fmax( Fmin( m_fHealth - fAmount, m_fMaxHealth ), 0.0F );
		m_ulDirtyStats |= kStatHealth;
	}
}

void StatsProtocol::SetBaseStrength(float fNewStrength)
{
	if( fNewStrength != m_fStrength )
	{
		m_fStrength = Fmax( fNewStrength, 0.0F );
		m_ulDirtyStats |= kStatStrength;
	}
}

void StatsProtocol::SetBaseArmor( float fNewArmor)
{
	if( fNewArmor != m_fArmor )
	{
		m_fArmor = Fmax( fNewArmor, 0.0F );
		m_ulDirtyStats |= kStatArmor;
	}
}

void StatsProtocol::SetBaseMovementSpeed( float fNewMovementSpeed )
{
	if( fNewMovementSpeed != m_fMovementSpeed )
	{
		m_fMovementSpeed = Fmax( fNewMovementSpeed, 0.0F );
		m_ulDirtyStats |= kStatMoveSpeed;
	}
}

void StatsProtocol::SetBaseAttackRange( float fNewAttackRange )
{
	if( fNewAttackRange != m_fAttackRange )
	{
		m_fAttackRange = Fmax( fNewAttackRange, 0.0F );
		m_ulDirtyStats |= kStatAttackRange;
	}
}

/* ------------------ Mutate Effects ---------------- */

void StatsProtocol::SetEffectOnMaxHealth( float fNewEffect ) 
{
	if( fNewEffect != m_fEffectOnMaxHealth )
	{
		m_fEffectOnMaxHealth = fNewEffect;
		m_ulDirtyStats |= kStatMaxHealthEffect;
	}
}

void StatsProtocol::SetEffectOnStrength( float fNewEffect )
{
	if( fNewEffect != m_fEffectOnStrength )
	{
		m_fEffectOnStrength = fNewEffect;
		m_ulDirtyStats |= kStatStrengthEffect;
	}
}

void StatsProtocol::SetEffectOnArmor( float fNewEffect)
{
	if( fNewEffect != m_fEffectOnArmor )
	{
		m_fEffectOnArmor = fNewEffect;
		m_ulDirtyStats |= kStatArmorEffect;
	}
}

void StatsProtocol::SetEffectOnMovementSpeed( float fNewEffect )
{
	if( fNewEffect != m_fEffectOnMovementSpeed )
	{
		m_fEffectOnMovementSpeed = fNewEffect;
		m_ulDirtyStats |= kStatMoveSpeedEffect;
	}
}

void StatsProtocol::SetEffectOnAttackRange( float fNewEffect )
{
	if( fNewEffect != m_fEffectOnAttackRange )
	{
		m_fEffectOnAttackRange = Fmax( fNewEffect, 0.0F );
		m_ulDirtyStats |= kStatAttackRangeEffect;
	}
}

// Effects

bool StatsProtocol::ApplyEffect( StatEffect* pEffect )
{
	if( pEffect )
	{
		m_arrEffects.AddElement(pEffect);
		pEffect->OnApplyEffect();

		return (true);
	}

	return (false);
}

bool StatsProtocol::RemoveEffect( StatEffect* pEffect )
{
	if( pEffect )
	{
		long index = m_arrEffects.FindElement(pEffect);
		if( index >= 0 ) {

			pEffect->OnRemoveEffect();

			m_arrEffects.RemoveElement(index);
			return (true);
		}
	}

	return (false);
}

void StatsProtocol::UpdateEffects( unsigned long ulTime )
{
	long size = m_arrEffects.GetElementCount();
	long newSize; 
	for( long x = 0; x < size; x++ ) {

		StatEffect *statEffect = m_arrEffects[x];

		statEffect->UpdateEffect( ulTime );

		// Adjust for the cases when effects remove themselves
		newSize = m_arrEffects.GetElementCount();
		if( newSize != size )
		{
			x--;
			size = newSize;
		}
	}
}

void StatsProtocol::Synchronize( long lControllerIndex, bool bFullSync )
{
	if( bFullSync ) {

		m_ulDirtyStats = 0xFFFFFFFF;
	}

	if( m_ulDirtyStats != 0  )
	{
		Array<float> fStats;

		if( m_ulDirtyStats & kStatMaxHealth )
			fStats.AddElement( GetBaseMaxHealth() );

		else if( m_ulDirtyStats & kStatHealth )
			fStats.AddElement( GetHealth() );

		else if( m_ulDirtyStats & kStatStrength )
			fStats.AddElement( GetBaseStrength() );

		else if( m_ulDirtyStats & kStatArmor )
			fStats.AddElement( GetBaseArmor() );

		else if( m_ulDirtyStats & kStatAttackRange )
			fStats.AddElement( GetBaseAttackRange() );

		else if( m_ulDirtyStats & kStatMaxHealthEffect )
			fStats.AddElement( GetEffectOnMaxHealth() );

		else if( m_ulDirtyStats & kStatStrengthEffect )
			fStats.AddElement( GetEffectOnStrength() );

		else if( m_ulDirtyStats & kStatArmorEffect )
			fStats.AddElement( GetEffectOnArmor() );

		else if( m_ulDirtyStats & kStatMoveSpeedEffect )
			fStats.AddElement( GetEffectOnMovementSpeed() );

		else if( m_ulDirtyStats & kStatAttackRangeEffect )
			fStats.AddElement( GetEffectOnAttackRange() );


		TheMessageMgr->SendMessageAll( EntityUpdateStatsMessage( lControllerIndex, m_ulDirtyStats, fStats ) );


		m_ulDirtyStats = 0;
	}
}

void StatsProtocol::UpdateStats( unsigned long ulDirtyStats, const Array<float>& arrStats )
{
	// TODO: valid number of stats?

	long val;
	long numBits = sizeof(long) * 8;

	int y = 0;
	for( long x = 0; x < numBits; x++ )
	{
		val = 1 << x;
		if( ulDirtyStats & val )
		{
			UpdateStat( val, arrStats[y++] );
		}
	}
}

void StatsProtocol::UpdateStat( long lStatID, float fValue )
{
	if( lStatID & kStatMaxHealth ) {
		if( !TheMessageMgr->Server() ) SetBaseMaxHealth( fValue );
	}
	else if( lStatID & kStatHealth ) {

		// Display message for entity under attack.
		if( !TheMessageMgr->Server() && m_pOwner && fValue < GetHealth() ) {
			TheGame->UnderAttackMessage( m_pOwner->GetTeam(), m_pOwner->GetControllerType() );
		}

		if( !TheMessageMgr->Server() ) SetHealth( fValue );
	}
	else if( lStatID & kStatStrength ) {
		if( !TheMessageMgr->Server() ) SetBaseStrength( fValue );
	}
	else if( lStatID & kStatArmor ) {
		if( !TheMessageMgr->Server() ) SetBaseArmor( fValue );
	}
	else if( lStatID & kStatAttackRange ) {
		if( !TheMessageMgr->Server() ) SetBaseAttackRange( fValue );
	}
	else if( lStatID & kStatMaxHealthEffect ) {
		if( !TheMessageMgr->Server() ) SetEffectOnMaxHealth( fValue );
	}
	else if( lStatID & kStatStrengthEffect ) {
		if( !TheMessageMgr->Server() ) SetEffectOnStrength( fValue );
	}
	else if( lStatID & kStatArmorEffect ) {
		if( !TheMessageMgr->Server() ) SetEffectOnArmor( fValue );
	}
	else if( lStatID & kStatMoveSpeedEffect ) {
		if( !TheMessageMgr->Server() ) SetEffectOnMovementSpeed( fValue );
	}
	else if( lStatID & kStatAttackRangeEffect ) {
		if( !TheMessageMgr->Server() ) SetEffectOnAttackRange( fValue );
	}
}