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
#include "C4World.h"
#include "BTMath.h"
#include "MGGame.h"


using namespace C4;


BTProjectileController::BTProjectileController(ControllerType type, float radius, float height, unsigned long flags, const Point3D& position, float azimuth, float speed, float fDistance, long lTeam, float fAlt, float fOriMult, bool bSoundOnlyOnImpact, const char *pHitSound, unsigned long ulDuration ) :
	BTEntityController( kControllerBTProjectile, kEntityProjectile, nullptr, lTeam, 0, kStringTableProjectiles, fOriMult ), // TODO: zero for stringID
	m_objSteering( this ),
	projectileFlags( flags ),
	m_p3dInitialPosition( position ),
	m_fSpeed( speed ),
	m_ulDuration( ulDuration ),
	m_ulStartTime( 0 ),
	m_pStatEffect( nullptr ),
	m_fDistance( fDistance ),
	m_pDeathSound( pHitSound ),
	m_bDeathSoundOnlyOnImpact( bSoundOnlyOnImpact )
{
	SetAzimuth( azimuth );
	SetCollider( new BTCylinderCollider(position, this, radius, height) );
	m_fTargetAltitude = fAlt;
	m_pCallback = nullptr;
	m_pCallbackData = nullptr;

	SetBaseControllerType( kControllerLesserEntity );
}

BTProjectileController::~BTProjectileController()
{
	if( m_pStatEffect )
		delete m_pStatEffect;

	if( m_pCallbackData )
		delete m_pCallbackData;
}

void BTProjectileController::SetStatEffect( StatEffect *pStatEffect )
{
	m_pStatEffect = pStatEffect;
}

void BTProjectileController::SetImpactCallback( ProjectileImpactCallback pCallback, void *selfData )
{
	m_pCallback = pCallback;
	m_pCallbackData = selfData;
}

void BTProjectileController::Preprocess(void)
{
	BTEntityController::Preprocess();

	Point3D target;

	Vector3D direction( 1.0F, 0.0F, 0.0F );
	direction.RotateAboutY( m_fTargetAltitude );
	direction.RotateAboutZ( m_fAzimuth );

	target = m_p3dInitialPosition + ( direction * m_fDistance );

	m_objSteering.SetSpeed( m_fSpeed );
	m_objSteering.SetTarget( target );
	m_objSteering.SetMoveWithoutHeading( true ); // TODO: add to string table
	m_objSteering.SetSeekOn( true );
	m_objSteering.Set3DMovement( true );
	
	SetAnimation( kMotionMove );

	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();
}

void BTProjectileController::Move(void)
{
	BTEntityController::Move();

	float newAzimuth = GetAzimuth();
	Vector3D newVelocity;

	bool moved = m_objSteering.CalculateMove( GetPosition(), newAzimuth, newVelocity );

	if( TheMessageMgr->Server() ) {
		if( moved == false && !(projectileFlags & kProjectileNoImpact) ) {

			Kill();
			return;
		}
	}

	GetCollider()->GetPosition() += newVelocity * TheTimeMgr->GetDeltaTime();
	SetAzimuth( newAzimuth );

	Model *proj = static_cast<Model*>( GetTargetNode() );
	proj->Animate();

	Travel();	
}

void BTProjectileController::Travel(void)
{
	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	if( TheMessageMgr->Server() ) {

		if( projectileFlags & kProjectileNoImpact )
		{
			if( (TheTimeMgr->GetAbsoluteTime() - m_ulStartTime) >= m_ulDuration )
			{
				if( m_pStatEffect )
				{
					if( m_pCallback )
					{
						BTCollisionData data;
						data.m_pController = this;

						m_pCallback( &data, m_pCallbackData );
					}


					StatEffect *effect = m_pStatEffect->Clone();
					effect->SetOwner( this );
					effect->OnApplyEffect();
					effect->OnRemoveEffect();
				}

				// Play sound
				if( !m_bDeathSoundOnlyOnImpact && m_pDeathSound && !Text::CompareTextCaseless(m_pDeathSound, "<missing>") ) {
					TheMessageMgr->SendMessageAll( WorldSoundMessage(m_pDeathSound, GetPosition(), 16.0F) );
				}
				Kill();
				return;
			}
		}
		else
		{
			BTCollisionData data;
			Array<long> ignoreConts;
			TheGame->DetectEntityCollision( this, data, kCacheUnits | kCacheCommanders | kCacheBuildings | kCacheBases, GetAzimuth(), 0.0F, ignoreConts );
			if( data.m_State != kCollisionStateNone && data.m_pController != nullptr ) {
				if( data.m_pController->GetTeam() != m_lTeam ) {

					StatsProtocol *stats = data.m_pController->GetStats();

					if( stats && m_pStatEffect )
					{
						if( m_pCallback )
						{
							m_pCallback( &data, m_pCallbackData );
						}

						StatEffect *clone = m_pStatEffect->Clone();
						clone->SetOwner( data.m_pController );
						stats->ApplyEffect( clone );
					}

					// Play sound
					if( m_bDeathSoundOnlyOnImpact && m_pDeathSound && !Text::CompareTextCaseless(m_pDeathSound, "<missing>") ) {
						TheMessageMgr->SendMessageAll( WorldSoundMessage(m_pDeathSound, GetPosition(), 16.0F) );
					}
					Kill();
					return;
				}
			}
		}
	}

	Model *model = static_cast<Model*>( GetTargetNode() );

	const Point3D position( model->GetSuperNode()->GetInverseWorldTransform() * GetPosition() );
	model->SetNodePosition( position );
	model->SetNodeMatrix3D( model->GetSuperNode()->GetInverseWorldTransform() * Matrix3D().SetRotationAboutY( m_fTargetAltitude ) * Matrix3D().SetRotationAboutZ(m_fAzimuth + (m_fOriMult * K::pi)) );

	model->Invalidate();
}

// Destroy this projectile.
void BTProjectileController::Kill( void )
{
	BTEntityController::Kill();
}
