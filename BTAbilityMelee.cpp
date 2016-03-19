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
#include "MGGame.h"

using namespace C4;

AbilityMelee::AbilityMelee( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, float fDamage ) :
	Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
	m_fDamage( fDamage )
{
}

AbilityMelee::~AbilityMelee()
{
}

// Perform a melee attack from m_pOwner to 
// the specified target contained in the 
// AbilityInfoMelee.
bool AbilityMelee::Execute( void )
{
	if( !m_pOwner ) {
		return (false);
	}

	if( Ability::Execute() == false ) {

		return (false);
	}

	BTCylinderCollider *col = static_cast<BTCylinderCollider*>( m_pOwner->GetCollider() );
	if( !col ) {
		return false;
	}
	
	float heading = m_pOwner->GetAzimuth();

	long opposingTeam = 2;
	if( m_pOwner->GetTeam() == 2 ) 
		opposingTeam = 1;

	BTCollisionData data;
	Array<long> ignoreConts;
	ignoreConts.AddElement( m_pOwner->GetControllerIndex() );
	float closest = 100.0F;

	TheGame->DetectEntityCollision( m_pOwner, data, heading, 2.0F, TheGameCacheMgr->GetGameCache(kCacheUnits, opposingTeam), closest, ignoreConts, false );
	TheGame->DetectEntityCollision( m_pOwner, data, heading, 2.0F, TheGameCacheMgr->GetGameCache(kCacheCommanders, opposingTeam), closest, ignoreConts, false );
	TheGame->DetectEntityCollision( m_pOwner, data, heading, 2.0F, TheGameCacheMgr->GetGameCache(kCacheBuildings, opposingTeam), closest, ignoreConts, false );
	TheGame->DetectEntityCollision( m_pOwner, data, heading, 2.0F, TheGameCacheMgr->GetGameCache(kCacheBases, opposingTeam), closest, ignoreConts, false );

	if( data.m_pController ) {
		// Calculate damage
		float dmg = m_fDamage;
		if( m_pOwner->GetStats() ) {
			dmg += dmg * ModDmg_Str( m_pOwner->GetStats()->GetStrength() );
		}

		// Apply damage
		data.m_pController->Damage( m_fDamage, m_pOwner->GetControllerIndex() );
	}

	TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( m_pOwner->GetControllerIndex(), m_tableInfo.m_ulAbilityID ) );

	return (true);
}

Ability *AbilityMelee::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable );
	if( table ) {

		AbilityMelee *ability = new AbilityMelee( pOwner, tableInfo,
				Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID,tableInfo.m_ulAbilityID, 'DELA' ))),
				Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
				Text::StringToFloat( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DMG' ))));

		return (ability);
	}

	return (nullptr);
}

