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


#include "BTControllers.h"
#include "MGMultiplayer.h"
#include "MGGame.h"
#include "BTMath.h"
#include "BTStatEffects.h"

using namespace C4;

BTEntityController::BTEntityController(ControllerType contType) : 
	Controller(contType),
	m_pStats( nullptr ),
	m_pCollider( nullptr ),
	m_fAzimuth( 0.0F ),
	m_lEntityType( kEntityNone ),
	m_lTeam( 0 ),
	m_EntityMotion( kMotionNone ),
	m_fTargetAltitude( 0.0F ),
	m_ulStringID( 0 ),
	m_lTableID( kStringTableNone ),
	m_ulEntityFlags( 0 ),
	m_pSelectionRing( nullptr ),
	m_pMiniHealthBar( nullptr ),
	m_fOriMult( 0.0F )
{
	SetBaseControllerType( kControllerEntity );
}

BTEntityController::BTEntityController(ControllerType contType, EntityType lEntityType, StatsProtocol *pStats, long lTeam, unsigned long ulStringID, TableID lTableID, float fOriMult ) : 
	Controller( contType ),
	m_pStats( pStats ),
	m_pCollider( nullptr ),
	m_fAzimuth( 0.0F ),
	m_lEntityType( lEntityType ),
	m_lTeam( lTeam ),
	m_fTargetAltitude( 0.0F ),
	m_ulStringID( ulStringID ),
	m_lTableID( lTableID ),
	m_ulEntityFlags( 0 ),
	m_pSelectionRing( nullptr ),
	m_pMiniHealthBar( nullptr ),
	m_fOriMult( fOriMult )
{
	SetBaseControllerType( kControllerEntity );
}

BTEntityController::BTEntityController(ControllerType contType, EntityType lEntityType) : 
	Controller( contType ),
	m_pStats( nullptr ),
	m_pCollider( nullptr ),
	m_fAzimuth( 0.0F ),
	m_lEntityType( lEntityType ),
	m_fTargetAltitude( 0.0F ),
	m_ulStringID( 0 ),
	m_lTableID( kStringTableNone ),
	m_ulEntityFlags( 0 ),
	m_pSelectionRing( nullptr ),
	m_pMiniHealthBar( nullptr ),
	m_fOriMult( 0.0F )
{
	SetBaseControllerType( kControllerEntity );
}

BTEntityController::~BTEntityController()
{
	// Play death sound
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	const StringTable *st = TheStringTableMgr->GetStringTable( m_lTableID );
	if( world && st ) {
		const char *sound = st->GetString( StringID(m_ulStringID, 'DSND') );
		if( sound && !Text::CompareTextCaseless(sound, "<missing>") ) {
			TheMessageMgr->SendMessageAll( WorldSoundMessage(sound, m_pCollider->GetPosition(), 15.0F) );
		}
	}

	// Cleanup

	if( m_pStats ) {
		delete m_pStats;
	}

	if( m_pCollider ) {
		delete m_pCollider;
	}

	if( m_pSelectionRing ) {
		delete m_pSelectionRing;
	}

	if( m_pMiniHealthBar ) {
		delete m_pMiniHealthBar;
	}
}

void BTEntityController::Pack(Packer& data, unsigned long packFlags) const
{
	Controller::Pack(data, packFlags);
}

void BTEntityController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	Controller::Unpack(data, unpackFlags);
}

void BTEntityController::Preprocess(void)
{
	Controller::Preprocess();

	CreateSelectionRing();
	CreateMiniHealthBar();
}

void BTEntityController::CreateSelectionRing( void )
{
	if( GetTargetNode() )
	{
		const StringTable *table = TheStringTableMgr->GetStringTable( m_lTableID );
		if( table ) 
		{
			Point3D posOffset( Text::StringToFloat(table->GetString( StringID(m_ulStringID, 'RING', 'X') )),
							   Text::StringToFloat(table->GetString( StringID(m_ulStringID, 'RING', 'Y') )),
							   Text::StringToFloat(table->GetString( StringID(m_ulStringID, 'RING', 'Z') )));

			float radius = Text::StringToFloat(table->GetString( StringID(m_ulStringID, 'RING', 'RAD') ));

			m_pSelectionRing = new SelectionRing( posOffset, radius );
			m_pSelectionRing->Preprocess();

			GetTargetNode()->GetSuperNode()->AddSubnode( m_pSelectionRing );

			m_pSelectionRing->Disable();
		}
	}
}

void BTEntityController::UpdateSelectionRing( void )
{
	if( (m_ulEntityFlags & kEntitySelected) && m_pSelectionRing && m_pCollider )
	{
		if( m_pStats )
		{
			m_pSelectionRing->UpdateHealth( m_pStats->GetHealth() / m_pStats->GetMaxHealth() );
		}

		m_pSelectionRing->SetNodePosition( m_pCollider->GetPosition() + m_pSelectionRing->GetRingOffset() );
		m_pSelectionRing->Invalidate();

		m_pSelectionRing->Enable();
	}
}

void BTEntityController::CreateMiniHealthBar( void )
{
	if( GetTargetNode() )
	{
		const StringTable *table = TheStringTableMgr->GetStringTable( m_lTableID );
		if( table ) 
		{
			Point3D posOffset( Text::StringToFloat(table->GetString( StringID(m_ulStringID, 'HBAR', 'X') )),
				Text::StringToFloat(table->GetString( StringID(m_ulStringID, 'HBAR', 'Y') )),
				Text::StringToFloat(table->GetString( StringID(m_ulStringID, 'HBAR', 'Z') )));

			m_pMiniHealthBar = new MiniHealthBar( posOffset, 5.0F, 1.0F );
			m_pMiniHealthBar->Preprocess();

			//GetTargetNode()->GetSuperNode()->AddSubnode( m_pMiniHealthBar );

			//m_pMiniHealthBar->Disable();
			
			//TheInterfaceMgr->AddElement( m_pMiniHealthBar );
		}
	}
}

void BTEntityController::UpdateMiniHealthBar( void )
{
	//if( m_pMiniHealthBar && m_pStats && m_pCollider )
	//{
		//m_pMiniHealthBar->Update( m_pCollider->GetPosition(), (m_pStats->GetHealth() / m_pStats->GetMaxHealth()) );
	//}

	if( m_pMiniHealthBar && m_pCollider )
	{
		if( m_pStats )
		{
			m_pMiniHealthBar->Update( Point3D(0,0,0), m_pStats->GetHealth() / m_pStats->GetMaxHealth() );
		}

		//GetTargetNode()->GetSuperNode()->RemoveSubnode( m_pMiniHealthBar );
		//GetTargetNode()->GetSuperNode()->AddSubnode( m_pMiniHealthBar );

		//m_pMiniHealthBar->SetNodePosition( m_pCollider->GetPosition() + m_pMiniHealthBar->GetBarOffset() );
		//m_pMiniHealthBar->Invalidate();

		//m_pMiniHealthBar->Enable();
	}
}

void BTEntityController::SetSelected( bool selected )
{
	if( selected )
	{
		m_ulEntityFlags |= kEntitySelected;
	}
	else
	{
		m_ulEntityFlags &= ~kEntitySelected;
		m_pSelectionRing->Disable();
	}
}

void BTEntityController::Move( void )
{

}

void BTEntityController::Travel( void )
{
	if( TheMessageMgr->Server() )
	{
		if( m_pStats ) m_pStats->Synchronize( GetControllerIndex(), false );
	}
	
	UpdateSelectionRing();
	UpdateMiniHealthBar();
}

void BTEntityController::PostTravel( void )
{
	if( !(GetEntityFlags() & kEntityDead) )
	{
		if( m_pStats ) 
		{
			m_pStats->UpdateEffects( TheTimeMgr->GetAbsoluteTime() );

			if( TheMessageMgr->Server() )
			{
				if( m_pStats->GetHealth() < 1 ) {
					Kill();
				}
			}
		}
	}
}

// Calculate the final damage and apply it.
// fDamage - Initial damage (before defense considered).
// lEnemyIndex - Controller index of the attacker.
void BTEntityController::Damage( float fDamage, long lEnemyIndex )
{
	if( !TheMessageMgr->Server() ) {
		return;
	}

	if( !m_pStats ) {
		return;
	}

	// if this entity is invulnerable or is already dead (playing its death animation), don't deal damage
	if( (m_ulEntityFlags & kEntityInvulnerable) || (m_ulEntityFlags & kEntityDead) ) {
		return;
	}

	// Display message for entity under attack.
	TheGame->UnderAttackMessage( GetTeam(), GetControllerType() );

	// Calculate damage
	float dmg = fDamage;
	if( GetStats() ) {
		dmg -= dmg * ModDmg_Arm( GetStats()->GetArmor() );
	}

	dmg = Fmax( dmg, 0.0F );
	m_pStats->RemoveHealth( dmg );

	TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessageSparks, -1, GetControllerIndex() ) );
}

void BTEntityController::Kill( void )
{
	if( TheMessageMgr->Server() ) {
		TheMessageMgr->SendMessageAll( ControllerMessage(kEntityMessageDeath, GetControllerIndex()) );
	}
}

void BTEntityController::Destroy( void )
{
	if( GetTargetNode() )  {
		delete GetTargetNode();
	}
}

ControllerMessage *BTEntityController::ConstructMessage(ControllerMessageType type) const
{
	switch (type)
	{
		case kEntityMessageDeath:

			return (new ControllerMessage( type, GetControllerIndex() ));

		case kEntityMessageUpdatePosition:

			return (new EntityUpdatePositionMessage(GetControllerIndex()));

		case kEntityMessageUpdateAzimuth:

			return (new EntityUpdateAzimuthMessage(GetControllerIndex()));

		case kEntityMessageAbility:

			return (new EntityAbilityMessage(GetControllerIndex()));

		case kEntityMessagePlayAnimation:

			return (new EntityPlayAnimationMessage(GetControllerIndex()));

		case kEntityMessageUpdateStat:

			return (new EntityUpdateStatsMessage(GetControllerIndex()));

		case kEntityMessageStealth:

			return (new ControllerMessage(kEntityMessageStealth, GetControllerIndex()));

		case kEntityMessageUnstealth:

			return (new ControllerMessage(kEntityMessageUnstealth, GetControllerIndex()));

		case kEntityMessageStun:

			return (new ControllerMessage(kEntityMessageStun, GetControllerIndex()));

		case kEntityMessageUnstun:

			return (new ControllerMessage(kEntityMessageUnstun, GetControllerIndex()));
	
		default:

			return (Controller::ConstructMessage( type ));
	}

}

void BTEntityController::ReceiveMessage(const ControllerMessage *message)
{
	switch (message->GetControllerMessageType())
	{
		case kEntityMessageDeath:
		{
			Destroy();
			break;
		}

		case kEntityMessageUpdatePosition:
		{
			const EntityUpdatePositionMessage *m = static_cast<const EntityUpdatePositionMessage *>(message);
			GetCollider()->GetPosition() = m->GetUpdatePosition();
			break;
		}

		case kEntityMessageUpdateAzimuth:
		{
			const EntityUpdateAzimuthMessage *m = static_cast<const EntityUpdateAzimuthMessage *>(message);
			m_fAzimuth = m->GetUpdateAzimuth();
			break;
		}

		case kEntityMessageAbility:
		{
			const EntityAbilityMessage *m = static_cast<const EntityAbilityMessage *>( message );

			PerformAbility( m->m_lAbility );
			break;
		}

		case kEntityMessagePlayAnimation:
		{
			const EntityPlayAnimationMessage *m = static_cast<const EntityPlayAnimationMessage *>( message );

			SetAnimation( kMotionNone );
			SetAnimation( (EntityMotion) m->m_lEntityMotion );
			break;
		}

		case kEntityMessageUpdateStat:
		{
			const EntityUpdateStatsMessage *m = static_cast<const EntityUpdateStatsMessage *>( message );

			if( m_pStats ) 
			{
				m_pStats->UpdateStats( m->m_ulDirtyStats, m->m_arrStats );
			}
			break;
		}

		case kEntityMessageStealth:
		{
			SetEntityFlags( GetEntityFlags() | kEntityInvisible );

			GamePlayer *player = static_cast<GamePlayer *>( TheMessageMgr->GetLocalPlayer() );
			if( player )
			{
				if( m_lTeam == player->GetPlayerTeam() )
				{
					StealthEffect *effect = new StealthEffect( this, GetControllerIndex(), false );
					m_pStats->ApplyEffect( effect );
				}
				else
				{
					//StealthEffect *effect = new StealthEffect( this, GetControllerIndex(), true );
					//m_pStats->ApplyEffect( effect );
					SetPerspectiveExclusionMask( 1 );
				}
			}
			break;
		}

		case kEntityMessageUnstealth:
		{
			SetEntityFlags( GetEntityFlags() & ~kEntityInvisible );
			SetPerspectiveExclusionMask( 0 );
			break;
		}

		case kEntityMessageStun:
		{
			SetEntityFlags( GetEntityFlags() | kEntityStunned );
			break;
		}
			
		case kEntityMessageUnstun:
		{
			SetEntityFlags( GetEntityFlags() & ~kEntityStunned );
			break;
		}

		default:

			Controller::ReceiveMessage(message);
			break;
	}
}

void BTEntityController::PerformAbility( long lAbilityNumber )
{
	// Set the next attack time based on attack speed (converted to seconds).
	//if( m_pStats ) {
		//m_ulGlobalCooldown  = TheTimeMgr->GetAbsoluteTime() + (m_pStats->GetAttackSpeed() * 1000 );
	//}

	// Set the animation.
	// EntityMotion motion = static_cast<EntityMotion>( kMotionAbility1 + (lAbilityNumber - 1) );
	// SetAnimation( motion );
}


void BTEntityController::SetPerspectiveExclusionMask(unsigned long mask) const
{
	Node *root = GetTargetNode();
	Node *node = root;

	while( node ) 
	{
		if (node->GetNodeType() == kNodeGeometry)
		{
			Geometry *geometry = static_cast<Geometry *>(node);
			geometry->SetPerspectiveExclusionMask(mask);
					
			if (mask == 0)
			{
				geometry->SetMinDetailLevel(0);
			}
			else geometry->SetMinDetailLevel(geometry->GetObject()->GetGeometryLevelCount() - 1);
		}
		node = root->GetNextNode(node);
	}
}

bool BTEntityController::CloseToPosition( Point3D& p3dPosition ) {
	if( !m_pCollider ) {
		return false;
	}
	
	if( Fabs(Calculate3DDistance( m_pCollider->GetPosition(), p3dPosition)) <= CLOSE_TO_TARGET ) {
		return true;
	}

	return false;
}

bool BTEntityController::CloseToPosition2D( Point3D& p3dPosition ) {
	if( !m_pCollider ) {
		return false;
	}

	if( Fabs(Calculate2DDistance( m_pCollider->GetPosition(), p3dPosition)) <= CLOSE_TO_TARGET ) {
		return true;
	}

	return false;
}

// Decide if the enemy can be attacked.
bool BTEntityController::CanAttack( BTEntityController *pEnemy )
{
	if( !pEnemy ) {
		return false;
	}

	if( GetTeam() == pEnemy->GetTeam() ) {
		return false;
	}

	if( pEnemy->GetEntityFlags() & kEntityInvisible ) {
		return false;
	}

	if( GetEntityFlags() & kEntityDead ) {
		return false;
	}

	if( pEnemy->GetEntityFlags() & kEntityDead  ) {
		return false;
	}

	return true;
}

