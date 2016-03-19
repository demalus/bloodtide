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

using namespace C4;

AbilityStealth::AbilityStealth( BTEntityController *pOwner, StringTableInfo tableInfo, unsigned long ulDelay, unsigned long ulCooldown, unsigned long ulDuration ) :
		Ability( pOwner, tableInfo, ulDelay, ulCooldown ),
		m_ulDuration( ulDuration )
{

}

Ability *AbilityStealth::Construct( BTEntityController *pOwner, StringTableInfo tableInfo )
{
	const StringTable *table = TheStringTableMgr->GetStringTable( tableInfo.m_idStringTable );
	if( table ) {

		AbilityStealth *ability = new AbilityStealth( pOwner, tableInfo,
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID,tableInfo.m_ulAbilityID, 'DELA' ))),
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'COOL' ))),
			Text::StringToInteger( table->GetString(StringID(tableInfo.m_ulEntityID, tableInfo.m_ulAbilityID, 'DUR' ))));

		return (ability);
	}

	return (nullptr);
}

AbilityStealth::~AbilityStealth()
{

}

void AbilityStealth::Update( unsigned long ulTime )
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

void AbilityStealth::HandleEvent( AbilityEvent event, Type ulAbilityID, void *pData  )
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

bool AbilityStealth::Execute( void )
{
	if( TheMessageMgr->Server() )
	{
		if( Ability::Execute() == false ) {

			return (false);
		}

		long index = m_pOwner->GetControllerIndex();

		TheMessageMgr->SendMessageAll( ControllerMessage( BTEntityController::kEntityMessageStealth, index ));
		TheMessageMgr->SendMessageAll( EntityPlayAnimationMessage( index, m_tableInfo.m_ulAbilityID ) );
		TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessageVanish, -1, index ) );
	}

	return (true);
}