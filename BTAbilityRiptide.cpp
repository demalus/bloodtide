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

using namespace C4;


/* -------------- Ability Riptide -------------- */

AbilityRiptide::AbilityRiptide( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, float fRange, unsigned long ulDuration ) :
		Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
		m_fRange( fRange ),
		m_ulDuration( ulDuration ),
		m_pStatEffect( nullptr )
{

}

AbilityRiptide::~AbilityRiptide()
{
	if( m_pStatEffect )
		delete m_pStatEffect;
}

void AbilityRiptide::Update( unsigned long ulTime )
{
	Ability::Update( ulTime );

	if( TheMessageMgr->Server() )
	{
		if( m_pOwner && (m_pOwner->GetEntityFlags() & kEntityInvisible) )
		{
			if( (ulTime - m_ulLastUsed) >= m_ulDuration )
			{
				TheMessageMgr->SendMessageAll( ControllerMessage( BTEntityController::kEntityMessageUnstealth, m_pOwner->GetControllerIndex() ) );
			}
		}
	}
}

void AbilityRiptide::HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData  )
{
	switch( event )
	{
		case kAbilityEventServerTriggered:
		{
			if( TheMessageMgr->Server() )
			{
				if( m_pOwner && (m_pOwner->GetEntityFlags() & kEntityInvisible) )
				{
					TheMessageMgr->SendMessageAll( ControllerMessage( BTEntityController::kEntityMessageUnstealth, m_pOwner->GetControllerIndex() ) );
				}
			}

			break;
		}
	}

	Ability::HandleEvent( event, ulAbilityID, pData  );
}

bool AbilityRiptide::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		if( m_pStatEffect ) {

			Point3D position( m_pOwner->GetTargetNode()->GetWorldPosition() );

			Array<long> unitsInRange;
			Array<long> ignoreConts;

			long teamID = m_pOwner->GetTeam();

			TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheUnits, teamID ), ignoreConts, false );
			TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheCommanders, teamID ), ignoreConts, false );

			for( long x = 0; x < unitsInRange.GetElementCount(); x++ )
			{
				World *world = TheWorldMgr->GetWorld();
				BTEntityController *controller = static_cast<BTEntityController *>( world->GetController( unitsInRange[x] ) );

				if( controller && controller->GetStats() ) {

					StatEffect *effect = m_pStatEffect->Clone();
					effect->SetOwner( controller );

					controller->GetStats()->ApplyEffect( effect );
				}
			}
		}

		long index = m_pOwner->GetControllerIndex();

		TheMessageMgr->SendMessageAll( ControllerMessage( BTEntityController::kEntityMessageStealth, index ) );
		TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( index, m_tableInfo.m_ulAbilityID ) );
		TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessageVanish, -1, index ) );
	}

	return (true);
}

Ability *AbilityRiptide::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable);
	if( table ) {

		AbilityRiptide *ability = new AbilityRiptide( pOwner, tableInfo,
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DELA' ))),
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
			Text::StringToFloat( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'RANG' ))),
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DUR' ))));

		if( ability && pOwner )
		{
			ability->m_pStatEffect = new MoveSpeedEffect( nullptr, pOwner->GetControllerIndex(),
				Text::StringToFloat( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'MOVE' ))),
				Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DUR' ))));
		}

		return (ability);
	}

	return (nullptr);
}



