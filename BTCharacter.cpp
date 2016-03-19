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


#include "BTCharacter.h"
#include "BTCommander.h"
#include "MGGame.h"

#define CLOSE_TO_Z		0.05F
#define DEATH_ANIM_TIME	3000

using namespace C4;


namespace
{
	const float kGroundProbeDepth			= 0.05F;
	const float kInitialGravity				= -0.02F;
}


BTCharacterController::BTCharacterController(ControllerType contType) : 
	BTEntityController(contType),
	m_pFrameAnimator( nullptr ),
	m_lCharFlags( 0 ),
	m_v3dVelocity( Vector3D(0,0,0) ),
	m_fCharTime( 0.0F ),
	m_v3dImpulse( Vector3D(0,0,0) ),
	m_fFloatHeight( 0.0F ),
	m_ulKillTime( 0 )
{
	Initialize();
}

BTCharacterController::BTCharacterController(ControllerType contType, EntityType lEntityType, unsigned long ulStringID, TableID lTableID, unsigned long flags, StatsProtocol *pStats, float fFloatHeight, long lTeam, float fOriMult) : 
	BTEntityController( contType, lEntityType, pStats, lTeam, ulStringID, lTableID, fOriMult ),
	m_pFrameAnimator( nullptr ),
	m_lCharFlags( flags ),
	m_v3dVelocity( Vector3D(0,0,0) ),
	m_fCharTime( 0.0F ),
	m_v3dImpulse( Vector3D(0,0,0) ),
	m_fFloatHeight( fFloatHeight ),
	m_ulKillTime( 0 )
{
	Initialize();
}

BTCharacterController::~BTCharacterController()
{
}

void BTCharacterController::Initialize(void)
{
	// todo: exclusion masks ?
}

void BTCharacterController::Pack(Packer& data, unsigned long packFlags) const
{
	// todo: pack more stuff?
	BTEntityController::Pack(data, packFlags);

	data << m_lCharFlags;
}

void BTCharacterController::Unpack(Unpacker& data, unsigned long unpackFlags)
{
	BTEntityController::Unpack(data, unpackFlags);
	
	data >> m_lCharFlags;
}

void BTCharacterController::Preprocess(void)
{
	BTEntityController::Preprocess();

	// Gravity is -z unless the character floats.
	m_v3dInitialGrav = Vector3D(0, 0, -1);
	if( m_lCharFlags & kCharacterFloat ) {
		if( GetPosition().z < m_fFloatHeight ) {
			m_v3dInitialGrav = Vector3D(0, 0, 1);
		}
	}

	// Setup animator
	Model *model = static_cast<Model*>( GetTargetNode() );
	m_pFrameAnimator = new FrameAnimator(model);
	model->SetRootAnimator(m_pFrameAnimator);
	SetAnimation(kMotionMove);
}

void BTCharacterController::Premove()
{
	if( !m_pCollider ) {
		return;
	}

	// Set fall time.
	if( !(m_lCharFlags & kCharacterGround) ) {
		float dt = TheTimeMgr->GetFloatDeltaTime();
		m_fCharTime += dt;
	}

	if( (m_lCharFlags & kCharacterFloat) && !(m_lCharFlags & kCharacterGround) ) {
		// Apply gravity towards float height.
		float mult = -2.0F * m_v3dInitialGrav.z;
		//if( m_pCollider->GetPosition().z < m_fFloatHeight ) {
		//	mult *= -1.0F;
		//}

		m_v3dVelocity.z = (K::gravity * m_fCharTime) * mult;
	}
	else {
		// Apply normal gravity force.
		if( (m_lCharFlags & kCharacterGravity) && !(m_lCharFlags & kCharacterGround) ) {
			m_v3dVelocity.z = K::gravity * m_fCharTime + kInitialGravity;
		}
	}
}

void BTCharacterController::Postmove()
{
	if( m_lCharFlags & kCharacterFloat ) {
		// Remain at float height.
		//if( Fabs(m_pCollider->GetPosition().z - m_fFloatHeight) <= CLOSE_TO_Z ) {
		if( (m_v3dVelocity.z > 0 && m_pCollider->GetPosition().z > m_fFloatHeight) ||
			(m_v3dVelocity.z < 0 && m_pCollider->GetPosition().z < m_fFloatHeight) )
		{
			m_v3dVelocity.z = 0;
			m_lCharFlags |= kCharacterGround;
			m_pCollider->GetPosition().z = m_fFloatHeight;
			m_fCharTime = 0.0F;
		}
	}
	else {
		// If the new position is off a cliff or in the air, start falling,
		// otherwise clip to the ground.

		CollisionData data;

		Point3D p1 = m_pCollider->GetPosition();

		Point3D p2 = m_pCollider->GetPosition();
		p2.z -= kGroundProbeDepth;

		if( TestGround(p1, p2, 0, &data) ) {
			m_v3dVelocity.z = 0;
			m_lCharFlags |= kCharacterGround;

			m_pCollider->GetPosition() = data.position;
			m_pCollider->GetPosition().z += -kGroundProbeDepth;
			m_fCharTime = 0.0F;
		} 
		else {
			m_lCharFlags &= ~kCharacterGround;
		}
	}
}

void BTCharacterController::Move(void)
{
	BTEntityController::Move();

	Premove();

	Point3D newPosition = m_pCollider->GetPosition();
	newPosition += m_v3dVelocity + m_v3dImpulse;

	m_pCollider->GetPosition() = newPosition;

	// Reset impulse
	m_v3dImpulse = Vector3D(0,0,0);

	
	//if (!TheMessageMgr->Server()) Travel();
	Postmove();
}

void BTCharacterController::Travel(void)
{
	BTEntityController::Travel();

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	Model *tar = static_cast<Model*>( GetTargetNode() );
	if( !tar ) {
		return;
	}

	tar->Invalidate();

	// Clip character to world.
	Point3D pos = m_pCollider->GetPosition();
	Marker *mb = world->GetMapBounds();
	if( mb ) {
		MapBoundsController *mbc = static_cast<MapBoundsController*>( mb->GetController() );
		if( mbc ) {
			if( pos.x < mb->GetNodePosition().x ) {
				pos.x = mb->GetNodePosition().x;
			}
			if( pos.x > mb->GetNodePosition().x + mbc->GetTopRightVector().x ) {
				pos.x = mb->GetNodePosition().x + mbc->GetTopRightVector().x;
			}
			if( pos.y < mb->GetNodePosition().y ) {
				pos.y = mb->GetNodePosition().y;
			}
			if( pos.y > mb->GetNodePosition().y + mbc->GetTopRightVector().y ) {
				pos.y = mb->GetNodePosition().y + mbc->GetTopRightVector().y;
			}
		}

		m_pCollider->GetPosition() = pos;
	}

	const Point3D newPosition( tar->GetSuperNode()->GetInverseWorldTransform() * m_pCollider->GetPosition() );
	tar->SetNodePosition( newPosition );

	tar->SetNodeMatrix3D(tar->GetSuperNode()->GetInverseWorldTransform() * Matrix3D().SetRotationAboutZ(m_fAzimuth + (m_fOriMult * K::pi)));
}

bool BTCharacterController::TestGround(const Point3D& p1, const Point3D& p2, float fRadius, CollisionData *data) const
{
	Model *character = static_cast<Model *>( GetTargetNode() );

	return (character->GetWorld()->DetectCollision(p1, p2, fRadius, kColliderGeometry, data));
}

void BTCharacterController::SetAnimation( EntityMotion motion )
{
	if( m_EntityMotion == motion ) {
		return;
	}

	// once a unit is dead, it can no longer animate
	if( m_EntityMotion == kMotionDeath ) {
		return;
	}

	const StringTable *pTable = TheStringTableMgr->GetStringTable(m_lTableID);
	if( !pTable ) {

		BTEntityController::SetAnimation( motion );
		return;
	}

	FrameAnimator *animator = GetFrameAnimator();
	Interpolator *interpolator = animator->GetFrameInterpolator();

	interpolator->SetCompletionProc( &MotionComplete, this );

	switch (motion)
	{
		case kMotionNone:
		{
			interpolator->SetMode(kInterpolatorStop);
			break;
		}

		case kMotionMove:
		{
			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'MDL', 'ANIM', 'move')) );
			interpolator->SetMode(kInterpolatorForward | kInterpolatorLoop);
			interpolator->SetRate(2.0F);
			break;
		}

		case kMotionAbility1:
		{
			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'abi1', 'ANIM')) );
			interpolator->SetMode(kInterpolatorForward);
			break;
		}

		case kMotionAbility2:
		{
			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'abi2', 'ANIM')) );
			interpolator->SetMode(kInterpolatorForward);
			break;
		}

		case kMotionAbility3:
		{
			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'abi3', 'ANIM')) );
			interpolator->SetMode(kInterpolatorForward);
			break;
		}

		case kMotionDeath:
		{
			animator->SetAnimation( pTable->GetString(StringID(m_ulStringID, 'MDL', 'ANIM', 'dead')) );
			interpolator->SetMode(kInterpolatorForward);
			break;
		}
	}

	BTEntityController::SetAnimation( motion );
}

void BTCharacterController::MotionComplete(Interpolator *interpolator, void *data)
{
	static_cast<BTCharacterController *>(data)->SetAnimation( kMotionNone );
}

// Play the death animation and destroy this character.
void BTCharacterController::Kill( void )
{
	if( !(GetEntityFlags() & kEntityDead) )
	{
		SetAnimation( kMotionDeath );
		SetEntityFlags( GetEntityFlags() | kEntityDead );
		m_ulKillTime = TheTimeMgr->GetAbsoluteTime() + DEATH_ANIM_TIME;
	}
}

// Do attack bookkeeping.
void BTCharacterController::PerformAbility( long lAbilityNumber )
{
	BTEntityController::PerformAbility( lAbilityNumber ); // TODO: should just set global cooldown methinks
}

ControllerMessage *BTCharacterController::ConstructMessage(ControllerMessageType type) const
{
	return (BTEntityController::ConstructMessage(type));
}

void BTCharacterController::ReceiveMessage(const ControllerMessage *message)
{
	//switch (message->GetControllerMessageType())
	//{
	//	default:

			BTEntityController::ReceiveMessage(message);
	//		break;
	//}
}

