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
#include "StatsProtocol.h"
#include "C4Time.h"
#include "MGMultiplayer.h"
#include "BTControllers.h"

using namespace C4;

/* -------------- Ability -------------- */

Ability::Ability( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown  ) :
	m_pOwner( pOwner ),
	m_tableInfo( tableInfo ),
	m_ulDelay( ulDelay ),
	m_ulLastUsed( 0 ),
	m_ulCooldown( ulCooldown ),
	m_AbilityState( kAbilityStateNone )
{

}

Ability::~Ability()
{

}

bool Ability::Execute( void )
{
	if( !m_pOwner || !m_pOwner->GetStats() )
		return (false);

	unsigned long curTime = TheTimeMgr->GetAbsoluteTime();

	if( m_AbilityState == kAbilityStateGlobal || m_AbilityState == kAbilityStateCooldown )
		return (false);

	m_ulLastUsed = curTime;
	m_AbilityState = kAbilityStateCooldown;

	return (true);
}

void Ability::Update( unsigned long ulTime )
{
	if( m_AbilityState == kAbilityStateCooldown ) {

		if( (TheTimeMgr->GetAbsoluteTime() - m_ulLastUsed) >= m_ulCooldown ) {

			m_AbilityState = kAbilityStateNone;
		}
	}
	else if( m_AbilityState == kAbilityStateGlobal ) {

		if( (TheTimeMgr->GetAbsoluteTime() - m_ulLastUsed) >= GLOBAL_COOLDOWN ) {

			m_AbilityState = kAbilityStateNone;
		}
	}
}

void Ability::HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData )
{
	switch( event )
	{
		case kAbilityEventTriggered:
		{
			if( ulAbilityID == m_tableInfo.m_ulAbilityID )
			{
				m_ulLastUsed = TheTimeMgr->GetAbsoluteTime();
				m_AbilityState = kAbilityStateCooldown;
			}

			// if the time left on the cooldown is less than the global cooldown,
			// set the cooldown to be the global cooldown
			bool global = true;

			unsigned long curTime = TheTimeMgr->GetAbsoluteTime();

			if( m_AbilityState == kAbilityStateCooldown )
			{
				unsigned long diffTime = curTime - m_ulLastUsed;

				if( diffTime < m_ulCooldown ) {

					unsigned long timeLeft = m_ulCooldown - diffTime;

					if( timeLeft > GLOBAL_COOLDOWN ) {

						global = false;
					}
				}
			}

			if( global )
			{
				m_ulLastUsed = curTime;
				m_AbilityState = kAbilityStateGlobal;
			}

			break;
		}
	}
}

Ability *Ability::Construct( BTEntityController *pOwner, TableID idStringTable, unsigned long ulEntityID, unsigned long ulAbilityID )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( idStringTable );
	if( table ) {
		
		const char *pSound = table->GetString( StringID(ulEntityID, ulAbilityID, 'SOND') );

		StringTableInfo tableInfo( idStringTable, ulEntityID, ulAbilityID, pSound );

		const char *abilityType = table->GetString( StringID( ulEntityID, ulAbilityID, 'TYPE' ) );
		
		if( Text::CompareText( abilityType, "MELEE" ) ) {

			return AbilityMelee::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "PROJECTILE" ) ) {

			return AbilityProjectile::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "MULTISHOT" ) ) {

			return AbilityMultiShot::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "STEALTH" ) ) {

			return AbilityStealth::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "AOE" ) ) {

			return AbilityAOE::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "RIPTIDE" ) ) {

			return AbilityRiptide::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "LASER" ) ) {

			return AbilityLaser::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "BARRIER" ) ) {

			return AbilityBarrier::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "DEPTHCHARGE" ) ) {

			return AbilityDepthCharge::Construct( pOwner, tableInfo );
		}

		else if( Text::CompareText( abilityType, "BLADECOUNTER" ) ) {

			return AbilityBladeCounter::Construct( pOwner, tableInfo );
		}
	}

	return (nullptr); // TODO: return a default Ability that does nothing?
}