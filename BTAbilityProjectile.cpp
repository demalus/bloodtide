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
#include "BTEffects.h"

using namespace C4;


/* -------------- Ability Projectile -------------- */


AbilityProjectile::AbilityProjectile( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulProjectileID ) :
	Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
	m_ulProjectileID( ulProjectileID )
{

}


AbilityProjectile::~AbilityProjectile()
{

}

bool AbilityProjectile::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		// Spawn the projectile on all clients/server
		const Point3D& position = m_pOwner->GetTargetNode( )->GetWorldPosition( );
		long index = TheWorldMgr->GetWorld()->NewControllerIndex();
		
		TheMessageMgr->SendMessageAll( CreateProjectileMessage(index, m_ulProjectileID, m_pOwner->GetTeam(), position, m_pOwner->GetAzimuth(), m_pOwner->GetTargetAltitude()), true );

		// Attach this ability's stat effect to the projectile
		BTProjectileController *projectile = static_cast<BTProjectileController *>( TheWorldMgr->GetWorld()->GetController( index ) );
		if( projectile && projectile->GetControllerType() == kControllerBTProjectile )
		{
			projectile->SetStatEffect( CreateStatEffect() );

			StringTableInfo *info = new StringTableInfo( m_tableInfo.m_idStringTable, m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, m_tableInfo.m_pSound );
			projectile->SetImpactCallback( &ImpactEventCallback, info );
		}

		TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( m_pOwner->GetControllerIndex(), m_tableInfo.m_ulAbilityID ) );
	}

	return (true);
}

Ability *AbilityProjectile::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable);
	if( table ) {

		AbilityProjectile *ability = new AbilityProjectile( pOwner, tableInfo,
				Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DELA' ))),
				Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
				Text::StringToType( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'PROJ' ))));

		return (ability);
	}

	return (nullptr);
}

StatEffect *AbilityProjectile::CreateStatEffect( void )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( m_tableInfo.m_idStringTable );
	if( table && m_pOwner && m_pOwner->GetStats() ) {

		const char *statEffectType = table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "REGULAR" ) ) {

			float dmg =  Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DMG' )));
			dmg += dmg * ModDmg_Str( m_pOwner->GetStats()->GetStrength() );

			return new ProjectileBasicEffect( nullptr, m_pOwner->GetControllerIndex(), dmg );
		}

		else if( Text::CompareText( statEffectType, "DOT" ) ) {

			float dmg =  Text::StringToInteger( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DOT' )));
			dmg += dmg * ModDmg_Str( m_pOwner->GetStats()->GetStrength() );

			return new ProjectileDOTEffect( nullptr, m_pOwner->GetControllerIndex(), dmg,
						Text::StringToInteger( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DUR' ))));
		}
	}

	return (nullptr);
}

void AbilityProjectile::ImpactEventCallback( BTCollisionData *collision, void *selfData )
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

		else if( Text::CompareText( statEffectType, "DOT" ) ) {

			if( collision->m_pController )
			{
				long index = collision->m_pController->GetControllerIndex();

				TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessagePoison, -1, index ) );
			}
		}
	}
}

void AbilityProjectile::CreateInitialEffect( BTEntityController& entity )
{
	// For intitial effets

	/*
	const StringTable *table = TheStringTableMgr->GetStringTable( m_tableInfo.m_idStringTable );
	if( table && m_pOwner ) {

		const char *statEffectType = table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "REGULAR" ) ) {

			TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kEffectMessageHeal, -1, entity.GetControllerIndex() ) );
		}

		else if( Text::CompareText( statEffectType, "DOT" ) ) {

			TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kEffectMessageSparks, -1, entity.GetControllerIndex() ) );
		}
	}
	*/
}




