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


#include "BTFish.h"

using namespace C4;


SimpleFishController::SimpleFishController() : 
	BTEntityController( kControllerSimpleFish ),
	//m_pFrameAnimator( nullptr ),
	m_objSteering( this ),
	m_objBaseGoal( this )
{
	SetBaseControllerType( kControllerLesserEntity );
}

SimpleFishController::SimpleFishController(const SimpleFishController& pController) : 
	BTEntityController( kControllerSimpleFish ),
	m_objSteering( this ),
	m_objBaseGoal( this )
{
	SetBaseControllerType( kControllerLesserEntity );
}

Controller *SimpleFishController::Replicate(void) const
{
	return (new SimpleFishController(*this));
}

SimpleFishController::~SimpleFishController()
{
}

bool SimpleFishController::ValidNode( const Node *node )
{
	return ( node->GetNodeType() == kNodeMarker );
}

void SimpleFishController::Pack(Packer& data, unsigned long packFlags) const
{
	BTEntityController::Pack(data, packFlags);
}

void SimpleFishController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	BTEntityController::Unpack(data, unpackFlags);
}

void SimpleFishController::Preprocess(void)
{
	BTEntityController::Preprocess();

	SetCollider( new BTCylinderCollider( GetTargetNode()->GetNodePosition(), this, 0.0F, 0.0F ) );

	m_objBaseGoal.Enter();

	GetSteering()->SetMaxTurnRadius( 0.004F );
	GetSteering()->SetSpeed( 2.0F );
	GetSteering()->SetMoveForwardTurn( true );

	// Setup animator
	//Model *model = static_cast<Model*>( GetTargetNode() );
	//m_pFrameAnimator = new FrameAnimator(model);
	//model->SetRootAnimator(m_pFrameAnimator);
	//SetAnimation(kMotionMove);
}

void SimpleFishController::Move(void)
{
	BTEntityController::Move();
	long status = m_objBaseGoal.Process();

	if( status != kGoalInProgress ) {
		return;
	}

	float dt = TheTimeMgr->GetFloatDeltaTime();

	Vector3D impulse(0,0,0);
	float unitAzimuth = GetAzimuth();
	m_objSteering.CalculateMove( GetPosition(), unitAzimuth, impulse );

	impulse *= (dt/1000.0F);
	m_pCollider->GetPosition() += impulse;
	SetAzimuth( unitAzimuth );


	GetTargetNode()->Invalidate();
	const Point3D newPosition( GetTargetNode()->GetSuperNode()->GetInverseWorldTransform() * m_pCollider->GetPosition() );
	GetTargetNode()->SetNodeMatrix3D(GetTargetNode()->GetSuperNode()->GetInverseWorldTransform() * Matrix3D().SetRotationAboutZ(m_fAzimuth));
	GetTargetNode()->SetNodePosition( newPosition );
}

// The number of settings this controller will display in the world editor.
long SimpleFishController::GetSettingCount(void) const
{
	return (0);
}

// Get the value of the setting (from what was entered in the world editor).
Setting *SimpleFishController::GetSetting(long index) const
{
	return (nullptr);
}

// World editor uses this to go from text field in editor to actually stored by this controller.
void SimpleFishController::SetSetting(const Setting *setting)
{
}

//void SimpleFishController::SetAnimation( EntityMotion motion )
//{
//	if( m_EntityMotion == motion ) {
//		return;
//	}
//
//	BTEntityController::SetAnimation( motion );
//
//	const StringTable *pTable = TheStringTableMgr->GetStringTable(m_lTableID);
//	if( !pTable ) {
//		return;
//	}
//
//	FrameAnimator *animator = GetFrameAnimator();
//	Interpolator *interpolator = animator->GetFrameInterpolator();
//
//	switch (motion)
//	{
//	case kMotionNone:
//		{
//			interpolator->SetMode(kInterpolatorStop);
//			break;
//		}
//
//	case kMotionMove:
//		{
//			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'MDL', 'ANIM', 'move')) );
//			interpolator->SetMode(kInterpolatorForward | kInterpolatorLoop);
//			interpolator->SetRate(2.0F);
//			break;
//		}
//
//	case kMotionAbility1:
//		{
//			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'abi1', 'ANIM')) );
//			interpolator->SetMode(kInterpolatorForward);
//			break;
//		}
//
//	case kMotionAbility2:
//		{
//			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'abi2', 'ANIM')) );
//			interpolator->SetMode(kInterpolatorForward);
//			break;
//		}
//
//	case kMotionAbility3:
//		{
//			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'abi3', 'ANIM')) );
//			interpolator->SetMode(kInterpolatorForward);
//			break;
//		}
//
//	case kMotionDeath:
//		{
//			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'abi1', 'ANIM', 'dead')) );
//			interpolator->SetMode(kInterpolatorForward);
//			break;
//		}
//	}
//}

