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
#include "BTEffects.h"

using namespace C4;


/* -------------- Ability AOE -------------- */

AbilityAOE::AbilityAOE( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, float fRange ) :
		Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
		m_fRange( fRange ),
		m_bEffectSelf( false ),
		m_bApplyToEnemies( true )
{

}

AbilityAOE::~AbilityAOE()
{

}

bool AbilityAOE::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		StatEffect *effect = CreateStatEffect();
		if( effect == nullptr )
			return (true);

		World *world = TheWorldMgr->GetWorld();
		if( world == nullptr )
			return (false);

		Point3D position( m_pOwner->GetTargetNode()->GetWorldPosition() );

		Array<long> unitsInRange;
		Array<long> ignoreConts;

		long teamID = m_pOwner->GetTeam();
		if( m_bApplyToEnemies ) {

			if( teamID == 1 ) teamID = 2;
			else teamID = 1;
		}

		TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheUnits, teamID ), ignoreConts, false );
		TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheCommanders, teamID ), ignoreConts, false );

		if( m_bEffectSelf == false ) {
			
			long index = unitsInRange.FindElement( m_pOwner->GetControllerIndex() );
			if( index != -1 ) unitsInRange.RemoveElement( m_pOwner->GetControllerIndex() );
		}

		for( long x = 0; x < unitsInRange.GetElementCount(); x++ )
		{
			BTEntityController *controller = static_cast<BTEntityController *>( world->GetController( unitsInRange[x] ) );

			if( controller && controller->GetStats() ) {

				StatEffect *clone = effect->Clone();
				clone->SetOwner( controller );

				CreateEffect( *controller );
				controller->GetStats()->ApplyEffect( clone );
			}
		}

		TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( m_pOwner->GetControllerIndex(), m_tableInfo.m_ulAbilityID ) );

		delete effect;
	}

	return (true);
}

Ability *AbilityAOE::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable );
	if( table ) {

		AbilityAOE *ability = new AbilityAOE( pOwner, tableInfo,
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DELA' ))),
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
			Text::StringToFloat( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'RANG' ))));

		return (ability);
	}

	return (nullptr);
}

StatEffect *AbilityAOE::CreateStatEffect( void )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( m_tableInfo.m_idStringTable );
	if( table && m_pOwner && m_pOwner->GetStats() ) {

		const char *statEffectType = table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "HEAL" ) ) {

			m_bEffectSelf = true;
			m_bApplyToEnemies = false;

			return new AOEHealEffect( nullptr, m_pOwner->GetControllerIndex(),
						Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'AMT' ))));
		}

		else if( Text::CompareText( statEffectType, "WHIRLPOOL" ) ) {

			m_bEffectSelf = false;
			m_bApplyToEnemies = true;

			float dmg = Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'AMT' )));
			dmg += dmg * ModDmg_Str( m_pOwner->GetStats()->GetStrength() );


			return new AOEHealEffect( nullptr, m_pOwner->GetControllerIndex(), -dmg );
		}

		else if( Text::CompareText( statEffectType, "RECOVERY" ) ) {

			m_bEffectSelf = true;
			m_bApplyToEnemies = false;

			return new BadNameEffect( nullptr, m_pOwner->GetControllerIndex(),
						Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'AMT' ))),
						Text::StringToFloat( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'POW' ))),
						Text::StringToInteger( table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'DUR' ))));
		}
	}

	return (nullptr);
}

void AbilityAOE::CreateEffect( BTEntityController& entity )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( m_tableInfo.m_idStringTable );
	if( table && m_pOwner ) {

		const char *statEffectType = table->GetString(StringID( m_tableInfo.m_ulEntityID, m_tableInfo.m_ulAbilityID, 'STAT' ));

		if( Text::CompareText( statEffectType, "HEAL" ) ) {

			TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessageHeal, -1, entity.GetControllerIndex() ) );
		}

		else if( Text::CompareText( statEffectType, "WHIRLPOOL" ) ) {

		}

		else if( Text::CompareText( statEffectType, "RECOVERY" ) ) {

			TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessageHeal, -1, entity.GetControllerIndex() ) );
		}
	}
}




