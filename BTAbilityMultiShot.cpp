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
#include "C4World.h"

using namespace C4;


/* -------------- Ability Projectile -------------- */


AbilityMultiShot::AbilityMultiShot( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulProjectileID, long lNumProjectiles, float fAngle ) :
		Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
		m_ulProjectileID( ulProjectileID ),
		m_lNumProjectiles( lNumProjectiles ),
		m_fAngle( fAngle )
{

}


AbilityMultiShot::~AbilityMultiShot()
{

}

bool AbilityMultiShot::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		// Spawn the projectile on all clients/server
		const Point3D& position = m_pOwner->GetTargetNode( )->GetWorldPosition( );
		float azimuthOffset = 0.0F;
		bool left = true;

		for( long x = 0; x < m_lNumProjectiles; x++ )
		{
			long index = TheWorldMgr->GetWorld()->NewControllerIndex();

			TheMessageMgr->SendMessageAll( CreateProjectileMessage(index, m_ulProjectileID, m_pOwner->GetTeam(), position, m_pOwner->GetAzimuth() + azimuthOffset, m_pOwner->GetTargetAltitude()), true );

			// Attach this ability's stat effect to the projectile
			BTProjectileController *projectile = static_cast<BTProjectileController *>( TheWorldMgr->GetWorld()->GetController( index ) );
			if( projectile && projectile->GetControllerType() == kControllerBTProjectile )
			{
				projectile->SetStatEffect( CreateStatEffect() );

				StringTableInfo *info = new StringTableInfo( m_tableInfo.m_idStringTable, m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, m_tableInfo.m_pSound );
				projectile->SetImpactCallback( &ImpactEventCallback, info );
			}

			if( left ) {

				azimuthOffset += m_fAngle;
				azimuthOffset *= -1;

			} else {
				
				azimuthOffset -= m_fAngle;
				azimuthOffset *= -1;
			}

			left = !left;
		}

		TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( m_pOwner->GetControllerIndex(), m_tableInfo.m_ulAbilityID ) );
	}

	return (true);
}

Ability *AbilityMultiShot::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable);
	if( table ) {

		AbilityMultiShot *ability = new AbilityMultiShot( pOwner, tableInfo,
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DELA' ))),
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
			Text::StringToType( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'PROJ' ))),
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'NUM' ))),
			K::radians * Text::StringToFloat( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'ANGL' ))));

		return (ability);
	}

	return (nullptr);
}

StatEffect *AbilityMultiShot::CreateStatEffect( void )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( m_tableInfo.m_idStringTable );
	if( table && m_pOwner && m_pOwner->GetStats() ) {

		const char *statEffectType = table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "REGULAR" ) ) {

			float dmg =  Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DMG' )));
			dmg += dmg * ModDmg_Str( m_pOwner->GetStats()->GetStrength() );

			return new ProjectileBasicEffect( nullptr, m_pOwner->GetControllerIndex(), dmg );
		}
	}

	return (nullptr);
}

void AbilityMultiShot::ImpactEventCallback( BTCollisionData *collision, void *selfData )
{
	if( (collision == nullptr) || (selfData == nullptr) )
		return;

	StringTableInfo *tableInfo = static_cast<StringTableInfo *>( selfData );

	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo->m_idStringTable );
	if( table ) {

		const char *statEffectType = table->GetString(StringID( tableInfo->m_ulEntityID, tableInfo->m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "REGULAR" ) ) {

			if( collision->m_pController )
			{
				TheMessageMgr->SendMessageAll( CreatePositionalEffectMessage( kPositionalEffectMessageShockwave, -1, collision->m_pController->GetPosition() ) );
			}
		}
	}
}




