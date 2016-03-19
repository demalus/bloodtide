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

#include "BTStatEffects.h"
#include "BTControllers.h"
#include "MGGame.h"
#include "C4Time.h"
#include "C4Geometries.h"

using namespace C4;

/* -------------- Projectile Basic Effect -------------- */

ProjectileBasicEffect::ProjectileBasicEffect( BTEntityController *pOwner, long lSource, float fDamage ) : 
		StatEffect( pOwner, lSource ),
		m_fDamage( fDamage )
{

}

ProjectileBasicEffect::~ProjectileBasicEffect()
{

}

void ProjectileBasicEffect::OnApplyEffect( void )
{
	if( m_pOwner ) 
	{
		StatsProtocol *stats = m_pOwner->GetStats();

		if( stats )
		{
			m_pOwner->Damage( m_fDamage, m_lSource );
		}
	}
}

void ProjectileBasicEffect::UpdateEffect( unsigned long ulTime )
{
	m_pOwner->GetStats()->RemoveEffect( this );
}

void ProjectileBasicEffect::OnRemoveEffect( void )
{
	delete this;
}

StatEffect *ProjectileBasicEffect::Clone( void )
{
	return (new ProjectileBasicEffect( m_pOwner, m_lSource, m_fDamage ) );
}


/* -------------- Projectile DOT Effect -------------- */

ProjectileDOTEffect::ProjectileDOTEffect( BTEntityController *pOwner, long lSource, unsigned long ulDamage, unsigned long ulDuration ) : 
		StatEffect( pOwner, lSource ),
		m_ulDamage( ulDamage ),
		m_ulDuration( ulDuration ),
		m_ulBegin( 0 ),
		m_ulDamageAlreadyDone( 0 )
{

}

void ProjectileDOTEffect::OnApplyEffect( void )
{
	m_ulBegin = TheTimeMgr->GetAbsoluteTime();
}

void ProjectileDOTEffect::UpdateEffect( unsigned long ulTime )
{
	if( m_pOwner ) 
	{
		unsigned long elapse = (ulTime - m_ulBegin) / 1000;

		// only deal damage for the duration
		if( elapse > m_ulDuration ) elapse = m_ulDuration;

		unsigned long damage = elapse * m_ulDamage; // what is the total amount of damage that should be applied over the elapsed time?

		damage = damage - m_ulDamageAlreadyDone; // do not count the damage that was applied already
		m_ulDamageAlreadyDone += damage;

		if( elapse >= m_ulDuration )
		{
			StatsProtocol *stats = m_pOwner->GetStats();
			if( stats ) 
			{
				stats->RemoveEffect( this );
				return;
			}
		}

		m_pOwner->Damage( (float) damage, m_lSource );
	}
}

void ProjectileDOTEffect::OnRemoveEffect( void )
{
	delete this;
}

StatEffect *ProjectileDOTEffect::Clone( void )
{
	return (new ProjectileDOTEffect( m_pOwner, m_lSource, m_ulDamage, m_ulDuration) );
}

/* -------------- Stealth Effect -------------- */

StealthEffect::StealthEffect( BTEntityController *pOwner, long lSource, bool bFullStealth  ) : 
		StatEffect( pOwner, lSource ),
		m_bFullStealth( bFullStealth )
{

}

StealthEffect::~StealthEffect()
{
	long size = m_arrRevertGeometry.GetElementCount();
	for( long x = 0; x < size; x++ )
	{
		GeometryTuple *tuple = m_arrRevertGeometry[x];
		Geometry *geometry = tuple->m_pGeometry;
		if( geometry )
		{
			MaterialObject *const *shader = geometry->GetMaterialObjectPointer();
			(*shader)->Release();

			geometry->SetMaterialObjectPointer( tuple->m_pOldMaterial );
			geometry->SetRenderState( geometry->GetRenderState() & ~kRenderDepthInhibit );
			geometry->InvalidateShaderData();
		}

		delete tuple;
	}
}

void StealthEffect::OnApplyEffect( void )
{
	Node *node = m_pOwner->GetTargetNode();

	if( node )
	{
		node->EnumerateGeometries( node->GetNodePosition(), 100.0F, &StealthEffect::ApplyTransparency, this );
	}
}

void StealthEffect::OnRemoveEffect( void ) 
{
	delete this;
}

void StealthEffect::UpdateEffect( unsigned long ulTime ) 
{
	if( m_pOwner )
	{
		if( (m_pOwner->GetEntityFlags() & kEntityInvisible) == 0 )
		{
			StatsProtocol *stats = m_pOwner->GetStats();
			if( stats ) 
			{
				stats->RemoveEffect( this );
				return;
			}
		}
	}
}

bool StealthEffect::ApplyTransparency( Geometry *pNode, const Point3D& p3dPos, float fRadius, void *data )
{

	StealthEffect *stealthEffect = static_cast<StealthEffect *>( data );
	if( stealthEffect == nullptr ) return (false);

	MaterialObject *const *oldMaterial = pNode->GetMaterialObjectPointer();
	if( oldMaterial )
	{
		TextureMapAttribute *attr = static_cast<TextureMapAttribute *>( (*oldMaterial)->FindAttribute( kAttributeTextureMap, 0 ) );
		if( attr )
		{
			MaterialObject *const *shader = TheGame->GetStealthShader();
			if( shader )
			{
				stealthEffect->m_arrRevertGeometry.AddElement( new GeometryTuple( pNode, oldMaterial ) );

				pNode->SetMaterialObjectPointer( shader );
				pNode->SetRenderState( pNode->GetRenderState() | kRenderDepthInhibit );
				pNode->InvalidateShaderData();
			}
		}
	}

	return (true);
}

StatEffect *StealthEffect::Clone( void )
{
	return (new StealthEffect( m_pOwner, m_lSource, m_bFullStealth ) );
}

/* -------------- AOE Heal Effect -------------- */

AOEHealEffect::AOEHealEffect( BTEntityController *pOwner, long lSource, float fHealAmount ) : 
		StatEffect( pOwner, lSource ),
		m_fHealAmount( fHealAmount )
{

}

void AOEHealEffect::OnApplyEffect( void )
{
	if( m_pOwner ) 
	{
		StatsProtocol *stats = m_pOwner->GetStats();

		if( stats )
		{
			stats->AddHealth( m_fHealAmount );
		}
	}
}

void AOEHealEffect::UpdateEffect( unsigned long ulTime )
{
	m_pOwner->GetStats()->RemoveEffect( this );
}

void AOEHealEffect::OnRemoveEffect( void )
{
	delete this;
}

StatEffect *AOEHealEffect::Clone( void )
{
	return (new AOEHealEffect( m_pOwner, m_lSource, m_fHealAmount ) );
}

/* -------------- Bad Name Effect -------------- */

BadNameEffect::BadNameEffect( BTEntityController *pOwner, long lSource, float fHealAmount, float fStrengthBoost, unsigned long ulDuration ) : 
		StatEffect( pOwner, lSource ),
		m_fHealAmount( fHealAmount ),
		m_fStrengthBoost( fStrengthBoost ),
		m_ulDuration( ulDuration ),
		m_ulStartTime( 0 )
{

}

void BadNameEffect::OnApplyEffect( void )
{
	if( m_pOwner ) 
	{
		StatsProtocol *stats = m_pOwner->GetStats();

		if( stats )
		{
			stats->AddHealth( m_fHealAmount );
			stats->SetEffectOnStrength( stats->GetEffectOnStrength() + m_fStrengthBoost );
		}
	}

	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();
}

void BadNameEffect::UpdateEffect( unsigned long ulTime )
{
	if( (ulTime - m_ulStartTime) >= m_ulDuration )
		m_pOwner->GetStats()->RemoveEffect( this );
}

void BadNameEffect::OnRemoveEffect( void )
{
	if( m_pOwner && m_pOwner->GetStats() ) 
	{
		StatsProtocol *stats = m_pOwner->GetStats();

		stats->SetEffectOnStrength( stats->GetEffectOnStrength() - m_fStrengthBoost );
	}

	delete this;
}

StatEffect *BadNameEffect::Clone( void )
{
	return (new BadNameEffect( m_pOwner, m_lSource, m_fHealAmount, m_fStrengthBoost, m_ulDuration ));
}

/* -------------- Move Speed Effect -------------- */

MoveSpeedEffect::MoveSpeedEffect( BTEntityController *pOwner, long lSource, float fEffectOnMove, unsigned long ulDuration ) : 
		StatEffect( pOwner, lSource ),
		m_fEffectOnMove( fEffectOnMove ),
		m_ulDuration( ulDuration ),
		m_ulStartTime( 0 )
{

}

void MoveSpeedEffect::OnApplyEffect( void )
{
	if( m_pOwner ) 
	{
		StatsProtocol *stats = m_pOwner->GetStats();

		if( stats )
		{
			stats->SetEffectOnMovementSpeed( stats->GetEffectOnMovementSpeed() + m_fEffectOnMove);
		}
	}

	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();
}

void MoveSpeedEffect::UpdateEffect( unsigned long ulTime )
{
	if( (ulTime - m_ulStartTime) >= m_ulDuration )
		m_pOwner->GetStats()->RemoveEffect( this );
}

void MoveSpeedEffect::OnRemoveEffect( void )
{
	if( m_pOwner && m_pOwner->GetStats() ) 
	{
		StatsProtocol *stats = m_pOwner->GetStats();

		stats->SetEffectOnMovementSpeed( stats->GetEffectOnMovementSpeed() - m_fEffectOnMove);
	}

	delete this;
}

StatEffect *MoveSpeedEffect::Clone( void )
{
	return (new MoveSpeedEffect(m_pOwner, m_lSource, m_fEffectOnMove, m_ulDuration));
}

/* -------------- Laser Trident Effect -------------- */

LaserTridentEffect::LaserTridentEffect( BTEntityController *pOwner, long lSource, float fDamage, unsigned long ulDuration ) : 
		StatEffect( pOwner, lSource ),
		m_fDamage( fDamage ),
		m_ulDuration( ulDuration ),
		m_ulStartTime( 0 ),
		m_lEffectController( -1 )
{

}

LaserTridentEffect::~LaserTridentEffect()
{
}

void LaserTridentEffect::OnApplyEffect( void )
{
	if( m_pOwner && TheWorldMgr->GetWorld() ) 
	{
		m_lEffectController = TheWorldMgr->GetWorld()->NewControllerIndex();

		TheMessageMgr->SendMessageAll( CreateTargettedEffectMessage( kTargettedEffectMessageStun, m_lEffectController, m_pOwner->GetControllerIndex() ) );
		TheMessageMgr->SendMessageAll( ControllerMessage( BTEntityController::kEntityMessageStun, m_pOwner->GetControllerIndex() ) );
		m_pOwner->Damage( m_fDamage, m_lSource );
	}

	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();
}

void LaserTridentEffect::UpdateEffect( unsigned long ulTime )
{
	if( (ulTime - m_ulStartTime) >= m_ulDuration )
		m_pOwner->GetStats()->RemoveEffect( this );
}

void LaserTridentEffect::OnRemoveEffect( void )
{
	if( m_pOwner ) 
	{
		TheMessageMgr->SendMessageAll( ControllerMessage( BTEntityController::kEntityMessageUnstun, m_pOwner->GetControllerIndex() ) );

		if( m_lEffectController != -1 )
			TheMessageMgr->SendMessageAll( ControllerMessage( Controller::kControllerMessageDeleteNode, m_lEffectController ) );

		m_lEffectController = -1;
	}

	delete this;
}

StatEffect *LaserTridentEffect::Clone( void )
{
	return (new LaserTridentEffect(m_pOwner, m_lSource, m_fDamage, m_ulDuration));
}

/* -------------- Invulnerability Effect -------------- */

InvulnerabilityEffect::InvulnerabilityEffect( BTEntityController *pOwner, long lSource, unsigned long ulDuration ) : 
		StatEffect( pOwner, lSource ),
		m_ulDuration( ulDuration ),
		m_ulStartTime( 0 )
{

}

InvulnerabilityEffect::~InvulnerabilityEffect()
{
	if( m_pOwner ) 
	{
		m_pOwner->SetEntityFlags( m_pOwner->GetEntityFlags() & ~kEntityInvulnerable );
	}
}

void InvulnerabilityEffect::OnApplyEffect( void )
{
	if( m_pOwner ) 
	{
		m_pOwner->SetEntityFlags( m_pOwner->GetEntityFlags() | kEntityInvulnerable );
	}

	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();
}

void InvulnerabilityEffect::UpdateEffect( unsigned long ulTime )
{
	if( (ulTime - m_ulStartTime) >= m_ulDuration )
		m_pOwner->GetStats()->RemoveEffect( this );
}

void InvulnerabilityEffect::OnRemoveEffect( void )
{
	delete this;
}

StatEffect *InvulnerabilityEffect::Clone( void )
{
	return (new InvulnerabilityEffect(m_pOwner, m_lSource, m_ulDuration));
}

/* -------------- Depth Charge Effect -------------- */

DepthChargeEffect::DepthChargeEffect( BTEntityController *pOwner, long lSource, float fDamage, float fRange, unsigned long ulDuration ) : 
		StatEffect( pOwner, lSource ),
		m_fDamage( fDamage ),
		m_fRange( fRange ),
		m_ulDuration( ulDuration )
{

}

void DepthChargeEffect::OnApplyEffect( void )
{
	if( (m_pOwner == nullptr) || (m_pOwner->GetTargetNode() == nullptr) ) {

		return;
	}

	World *world = TheWorldMgr->GetWorld();
	if( world == nullptr )
		return;

	Point3D position( m_pOwner->GetTargetNode()->GetWorldPosition() );

	// Create an explosion at the position of the depth charge
	TheMessageMgr->SendMessageAll( CreatePositionalEffectMessage( kPositionalEffectMessageDepthCharge, -1, position ) );


	// Create a bolt to every unit in range of the dpeth charge
	Array<long> unitsInRange;
	Array<long> ignoreConts;

	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheUnits, 1 ), ignoreConts, false );
	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheUnits, 2 ), ignoreConts, false );
	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheCommanders, 1 ), ignoreConts, false );
	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheCommanders, 2 ), ignoreConts, false );
	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheBases, 1 ), ignoreConts, false );
	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheBases, 2 ), ignoreConts, false );
	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheBuildings, 1 ), ignoreConts, false );
	TheGame->DetectEntities( position, m_fRange, unitsInRange, TheGameCacheMgr->GetGameCache( kCacheBuildings, 2 ), ignoreConts, false );

	for( long x = 0; x < unitsInRange.GetElementCount(); x++ )
	{
		BTEntityController *controller = static_cast<BTEntityController *>( world->GetController( unitsInRange[x] ) );

		if( controller && controller->GetStats() ) {

			TheMessageMgr->SendMessageAll( CreateBoltEffectMessage( position, controller->GetControllerIndex() ) );

			StatEffect *effect = new LaserTridentEffect( controller, m_lSource, m_fDamage, m_ulDuration );
			controller->GetStats()->ApplyEffect( effect );
		}
	}
}

void DepthChargeEffect::UpdateEffect( unsigned long ulTime )
{
	m_pOwner->GetStats()->RemoveEffect( this );
}

void DepthChargeEffect::OnRemoveEffect( void )
{
	delete this;
}

StatEffect *DepthChargeEffect::Clone( void )
{
	return (new DepthChargeEffect(m_pOwner, m_lSource, m_fDamage, m_fRange, m_ulDuration));
}

