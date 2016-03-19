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

#include "BTEffects.h"
#include "C4Zones.h"
#include "C4World.h"
#include "BTControllers.h"

#define ROT_PER_SECOND 3000.0F

using namespace C4;

BeamEffectController::BeamEffectController( unsigned long ulDuration ) : Controller( kControllerBeamEffect ),
		m_ulDuration( ulDuration )
{

}

void BeamEffectController::Preprocess( void )
{
	Controller::Preprocess();

	m_ulStartTime = TheTimeMgr->GetAbsoluteTime();

	BeamEffect *beam = static_cast<BeamEffect *>( GetTargetNode() );
	if( beam == nullptr || beam->GetNodeType() != kNodeEffect || beam->GetEffectType() != kEffectBeam ) {

		delete this;
		return;
	}
}

void BeamEffectController::Move( void )
{
	BeamEffect *beam = static_cast<BeamEffect *>( GetTargetNode() );
	if( beam == nullptr ) {

		delete this;
		return;
	}

	unsigned long elapsed = TheTimeMgr->GetAbsoluteTime() - m_ulStartTime;

	if( elapsed >= m_ulDuration )
	{
		delete GetTargetNode();
		return;
	}

	BeamEffectObject *obj = beam->GetObject();

	unsigned long fadeStart = m_ulDuration * 0.8F;
	if( elapsed < fadeStart )
	{
		ColorRGBA color = obj->GetBeamColor();
		color.alpha = (elapsed / fadeStart) + 0.2F;
		obj->SetBeamColor( color );

		beam->ProcessObjectSettings();
	}
	else
	{
		ColorRGBA color = obj->GetBeamColor();
		color.alpha = (elapsed - fadeStart) / fadeStart;
		obj->SetBeamColor( color );

		beam->ProcessObjectSettings();
	}
}

	//Array<Model *>				m_arrBlades;


BladeCounterEffectController::BladeCounterEffectController( BTEntityController *pEntity ) : AttachToEntityController( pEntity ),
		m_fRotation( 0.0F )
{

}

void BladeCounterEffectController::Preprocess( void )
{
	AttachToEntityController::Preprocess();
}

void BladeCounterEffectController::Move( void )
{
	static float rot_per_sec = ROT_PER_SECOND / K::two_pi;

	m_fRotation += (rot_per_sec * (TheTimeMgr->GetFloatDeltaTime() / 1000.0F));

	if( m_fRotation > K::two_pi )
		m_fRotation -= K::two_pi;

	Node *target = GetTargetNode();
	if( target	)
	{
		target->SetNodeMatrix3D( Matrix3D().SetRotationAboutZ( m_fRotation ) );
		target->Invalidate();
	}

	AttachToEntityController::Move();
}



AttachToEntityController::AttachToEntityController( BTEntityController *pEntity, bool bMantainRotation )
{
	m_pEntity = pEntity;
	m_p3dOffset = Point3D( 0.0F, 0.0F, 0.0F );
	m_bMantainRotation = bMantainRotation;
}

AttachToEntityController::~AttachToEntityController()
{

}

void AttachToEntityController::SetOffset( const Point3D& p3dOffset )
{
	m_p3dOffset = p3dOffset;
}

void AttachToEntityController::Preprocess( void )
{
	Controller::Preprocess();

	if( (m_pEntity == nullptr) || (m_pEntity->GetTargetNode() == nullptr) || (GetTargetNode() == nullptr) )
	{
		if( GetTargetNode() )
		{
			delete GetTargetNode();
		}
		else
		{
			delete this;
		}

		return;
	}
}

void AttachToEntityController::Move( void )
{
	if( (m_pEntity == nullptr) || (m_pEntity->GetTargetNode() == nullptr) || (GetTargetNode() == nullptr) )
	{
		if( GetTargetNode() )
		{
			delete GetTargetNode();
		}
		else
		{
			delete this;
		}

		return;
	}


	Zone *zone = m_pEntity->GetTargetNode()->GetOwningZone();
	if( zone )
	{
		if( m_bMantainRotation )
		{
			GetTargetNode()->SetNodeMatrix3D( m_pEntity->GetTargetNode()->GetNodeTransform().GetMatrix3D() );
		}

		GetTargetNode()->SetNodePosition( zone->GetInverseWorldTransform() * (m_pEntity->GetPosition() + m_p3dOffset) );
		zone->AddSubnode( GetTargetNode() );
	}
}

