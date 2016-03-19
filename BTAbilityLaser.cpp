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
#include "MGGame.h"
#include "C4World.h"
#include "BTMath.h"

using namespace C4;


/* -------------- Ability Laser -------------- */

AbilityLaser::AbilityLaser( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown ) :
		Ability( pOwner, tableInfo, ulDelay, ulCooldown )
{

}

AbilityLaser::~AbilityLaser()
{

}

bool AbilityLaser::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		World *world = TheWorldMgr->GetWorld();
		if( world )
		{
			float azimuth =  m_pOwner->GetAzimuth();
			float altitude = m_pOwner->GetTargetAltitude();

			float cp = Cos(altitude);
			float sp = Sin(altitude);

			float ct = Cos(azimuth);
			float st = Sin(azimuth);

			Vector3D view(ct * cp, st * cp, sp);

			Point3D start = m_pOwner->GetPosition();
			Point3D end = start + (view * 30.0F); // TODO: add a max range to string table
			float zo = start.z - end.z;
			end.z += (zo * 2.0F);

			BTCollisionData data;

			Array<long> ignoreColliders;
			ignoreColliders.AddElement( m_pOwner->GetControllerIndex() );

			float closestSoFar = K::infinity;

			long opp = m_pOwner->GetTeam() == 1 ? 2 : 1;
			TheGame->DetectEntity( start, end, 2.0F, data, TheGameCacheMgr->GetGameCache( kCacheUnits, opp), closestSoFar, ignoreColliders );
			TheGame->DetectEntity( start, end, 2.0F, data, TheGameCacheMgr->GetGameCache( kCacheCommanders, opp), closestSoFar, ignoreColliders );
			TheGame->DetectEntity( start, end, 2.0F, data, TheGameCacheMgr->GetGameCache( kCacheBases, opp), closestSoFar, ignoreColliders );
			TheGame->DetectEntity( start, end, 2.0F, data, TheGameCacheMgr->GetGameCache( kCacheBuildings, opp), closestSoFar, ignoreColliders );


			if( data.m_pController != nullptr ) 
			{
				end = data.m_pController->GetPosition();

				StatsProtocol *stats = data.m_pController->GetStats();
				if( stats )
				{
					StatEffect *effect = CreateStatEffect();
					effect->SetOwner( data.m_pController );

					stats->ApplyEffect( effect );
				}
			}

			CreateEffect( start, end );
		}

		long index = m_pOwner->GetControllerIndex();
		TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( index, m_tableInfo.m_ulAbilityID ) );
	}

	return (true);
}

Ability *AbilityLaser::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable);
	if( table ) {

		AbilityLaser *ability = new AbilityLaser( pOwner, tableInfo,
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DELA' ))),
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))));

		return (ability);
	}

	return (nullptr);
}

StatEffect *AbilityLaser::CreateStatEffect( void )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( m_tableInfo.m_idStringTable );
	if( table && m_pOwner && m_pOwner->GetStats() ) {

		const char *statEffectType = table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "LASER" ) ) {

			float dmg =  Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DMG' )));
			dmg += dmg * ModDmg_Str( m_pOwner->GetStats()->GetStrength() );

			return new LaserTridentEffect( nullptr, m_pOwner->GetControllerIndex(), dmg,
						Text::StringToInteger( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DUR' ))));
		}

		else if( Text::CompareText( statEffectType, "WAVECANNON" ) ) {

			float dmg =  Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DMG' )));
			dmg += dmg * ModDmg_Str( m_pOwner->GetStats()->GetStrength() );


			return new ProjectileBasicEffect( nullptr, m_pOwner->GetControllerIndex(), dmg );
		}

	}

	return (nullptr);
}

void AbilityLaser::CreateEffect( Point3D& p3dStart, Point3D& p3dEnd )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( m_tableInfo.m_idStringTable );
	if( table && m_pOwner ) {

		const char *statEffectType = table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "LASER" ) ) {

			float distance = Calculate2DDistance( p3dStart, p3dEnd );

			TheMessageMgr->SendMessageAll( CreateBeamEffectMessage( p3dStart, m_pOwner->GetTargetAltitude(), m_pOwner->GetAzimuth(), distance ) );
		}

		else if( Text::CompareText( statEffectType, "WAVECANNON" ) ) {

			float distance = Calculate2DDistance( p3dStart, p3dEnd );

			TheMessageMgr->SendMessageAll( CreateBeamEffectMessage( p3dStart, m_pOwner->GetTargetAltitude(), m_pOwner->GetAzimuth(), distance, 1 ) );
		}

	}
}



