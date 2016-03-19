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


/* -------------- Ability Blade Counter -------------- */

AbilityBladeCounter::AbilityBladeCounter( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulDuration, float fPercentReturned ) :
		Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
		m_ulDuration( ulDuration ),
		m_fPercentReturned( fPercentReturned ),
		m_lEffectControllerIndex( -1 ),
		m_bActive( false )
{

}

AbilityBladeCounter::~AbilityBladeCounter()
{
	if( m_lEffectControllerIndex != -1 )
	{
		TheMessageMgr->SendMessageAll( ControllerMessage( Controller::kControllerMessageDeleteNode, m_lEffectControllerIndex ) );
	}
}

void AbilityBladeCounter::Update( unsigned long ulTime )
{
	Ability::Update( ulTime );

	if( TheMessageMgr->Server() )
	{
		if( m_pOwner && (m_bActive) )
		{
			if( (ulTime - m_ulLastUsed) >= m_ulDuration )
			{
				if( m_lEffectControllerIndex != -1 )
				{
					TheMessageMgr->SendMessageAll( ControllerMessage( Controller::kControllerMessageDeleteNode, m_lEffectControllerIndex ) );
				}

				m_lEffectControllerIndex = -1;
				m_bActive = false;
			}
		}
	}
}

void AbilityBladeCounter::HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData  )
{
	switch( event )
	{
		case kAbilityEventDamaged:
		{
			if( TheMessageMgr->Server() )
			{
				if( m_pOwner && m_bActive && pData )
				{
					EntityDamaged *data = static_cast<EntityDamaged *>( pData );

					World *world = TheWorldMgr->GetWorld();
					if( world )
					{

						BTEntityController *controller = static_cast<BTEntityController *>( world->GetController( data->m_lSource ) );
						if( controller && controller->GetBaseControllerType() == kControllerEntity )
						{
							// -1: if both entities had blade counter, the damage would keep bouncing back
							controller->Damage( data->m_lDamage * m_fPercentReturned, -1 );
						}
					}

					/* Uncomment these to have the blade counter dissipate when attacked
					if( m_lEffectControllerIndex != -1 )
					{
						TheMessageMgr->SendMessageAll( ControllerMessage( Controller::kControllerMessageDeleteNode, m_lEffectControllerIndex ) );
					}

					m_lEffectControllerIndex = -1;
					m_bActive = false;
					*/
				}
			}

			break;
		}
	}

	Ability::HandleEvent( event, ulAbilityID, pData );
}

bool AbilityBladeCounter::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		m_lEffectControllerIndex = -1;
		World *world = TheWorldMgr->GetWorld();

		if( world )
		{
			long index = m_pOwner->GetControllerIndex();
			TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( index, m_tableInfo.m_ulAbilityID ) );

			m_lEffectControllerIndex = world->NewControllerIndex();
			TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessageBladeCounter, m_lEffectControllerIndex, index ) );

			m_bActive = true;
		}
	}

	return (true);
}

Ability *AbilityBladeCounter::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable);
	if( table ) {

		AbilityBladeCounter *ability = new AbilityBladeCounter( pOwner, tableInfo,
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DELA' ))),
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
			Text::StringToInteger( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DUR' ))),
			Text::StringToFloat( table->GetString(StringID( tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'PERC' ))));

		return (ability);
	}

	return (nullptr);
}



