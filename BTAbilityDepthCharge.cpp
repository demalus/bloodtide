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

#include "BTAbility.h"
#include "BTControllers.h"
#include "MGMultiplayer.h"
#include "BTStatEffects.h"
#include "BTEffects.h"
#include "C4World.h"

using namespace C4;


/* -------------- Ability Depth Charge -------------- */


AbilityDepthCharge::AbilityDepthCharge( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulProjectileID, float fDamage, float fRange, unsigned long ulDuration ) :
		Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
		m_ulProjectileID( ulProjectileID ),
		m_fDamage( fDamage ),
		m_fRange( fRange ),
		m_ulDuration( ulDuration )
{

}


AbilityDepthCharge::~AbilityDepthCharge()
{

}

bool AbilityDepthCharge::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		// Spawn the projectile on all clients/server
		const Point3D& position = m_pOwner->GetTargetNode( )->GetWorldPosition( );
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();

		TheMessageMgr->SendMessageAll( CreateProjectileMessage(index, m_ulProjectileID, m_pOwner->GetTeam(), position, m_pOwner->GetAzimuth(), K::pi_over_2), true );

		// Attach this ability's stat effect to the projectile
		BTProjectileController *projectile = static_cast<BTProjectileController *>( TheWorldMgr->GetWorld()->GetController( index ) );
		if( projectile && projectile->GetControllerType() == kControllerBTProjectile )
		{
			projectile->SetProjectileFlags( projectile->GetProjectileFlags() | kProjectileNoImpact );
			projectile->SetDuration( m_ulDelay );
			projectile->SetStatEffect( new DepthChargeEffect( nullptr, m_pOwner->GetControllerIndex(), m_fDamage + (m_fDamage * ModDmg_Str( m_pOwner->GetStats()->GetStrength() )), m_fRange, m_ulDuration ) );
		}

		TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( m_pOwner->GetControllerIndex(), m_tableInfo.m_ulAbilityID ) );
	}

	return (true);
}

Ability *AbilityDepthCharge::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable);
	if( table ) {

		AbilityDepthCharge *ability = new AbilityDepthCharge( pOwner, tableInfo,
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DELA' ))),
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
			Text::StringToType( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'PROJ' ))),
			Text::StringToFloat( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DMG' ))),
			Text::StringToFloat( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'RANG' ))),
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DUR' ))));

		return (ability);
	}

	return (nullptr);
}

void AbilityDepthCharge::ImpactEventCallback( BTCollisionData *collision, void *selfData )
{

}


