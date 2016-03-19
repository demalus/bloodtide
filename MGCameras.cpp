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


#include "C4World.h"
#include "BTCommander.h"
#include "BTMath.h"
#include "MGGame.h"

#define CURSOR_BOUNDARY 5.0F // how close the cursor needs to be to the edge of the screen to move the camera

// Camera Blender constants //
#define END_INITIAL_BLEND 0.4F // from the start camera's orientation to an orientation facing the end camera
#define START_SECONDARY_BLEND 0.6F // from the orientation facing the end camera to the end camera's orientation


//

using namespace C4;

RTSCamera::RTSCamera() :
		OrientedCamera( TheGame->GetCameraFocalLength(), 1.0F ),
		m_lNavigationFlags(0),
		m_fCeiling(100.0F),
		m_bEnabled(true)
{

}

void RTSCamera::Initialize(Point3D &objNodePosition)
{
	SetNodePosition( objNodePosition );
	m_fCeiling = objNodePosition.z;

	OrientedCamera::SetCameraAltitude( -K::pi_over_4 - K::pi_over_6 );
	OrientedCamera::SetCameraAzimuth( K::pi_over_2 );
}

void RTSCamera::Move()
{
	if( m_bEnabled == false )
		return;

	if( (TheEngine->GetEngineFlags() & kEngineForeground) == 0 )
		return;

	GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
	if( !world ) {
		return;
	}

	// Calculate new position for camera
	Point3D newPos = GetNodePosition();

	float deltaTime = TheTimeMgr->GetFloatDeltaTime();
	float multiplier = deltaTime / 100;
	multiplier *= 5.0F;

	const Point curPos = TheInterfaceMgr->GetMousePosition();
	float displayWidth = TheDisplayMgr->GetDisplayWidth();
	float displayHeight = TheDisplayMgr->GetDisplayHeight();

	if( curPos.x <= CURSOR_BOUNDARY )
		newPos.x -= multiplier;

	else if( curPos.x >= displayWidth - CURSOR_BOUNDARY )
		newPos.x += multiplier;

	if( curPos.y <= CURSOR_BOUNDARY )
		newPos.y += multiplier;

	else if( curPos.y >= displayHeight - CURSOR_BOUNDARY )
		newPos.y -= multiplier;

	// TODO: Only one or the other?

	if( m_lNavigationFlags & kNavUp ) {
		newPos.y += multiplier;
	}

	if( m_lNavigationFlags & kNavLeft ) {
		newPos.x -= multiplier;
	}

	if( m_lNavigationFlags & kNavDown ) {
		newPos.y -= multiplier;
	}

	if( m_lNavigationFlags & kNavRight ) {
		newPos.x += multiplier;
	}

	// TODO: Change mouse cursor

	// Clip camera to world bounds
	if( world->PointWithinMapBounds(newPos.GetPoint2D()) ) {
		// Now update the node's position
		SetNodePosition( newPos );
	}

	OrientedCamera::Move();
}

CameraBlender::CameraBlender() :
		OrientedCamera( TheGame->GetCameraFocalLength(), 1.0F ),
		m_pEndCamera( nullptr ),
		m_fTransitionTime( -1 ),
		m_ulStartTime( -1 ),
		m_fStartAzimuth( 0 ),
		m_fStartAltitude( 0 ),
		m_bStartFinalTransition( false ),
		m_pCallback( nullptr )
{

}

void CameraBlender::Initialize( FrustumCamera& startCamera, FrustumCamera& endCamera, float fTime, BlenderCallback *pCallback )
{
	World *world = TheWorldMgr->GetWorld();
	if( world == nullptr ) return;

	if( fTime <= 0 ) {

		world->SetCamera( &endCamera );

	} else {

		m_pStartCamera = &startCamera;
		m_pEndCamera = &endCamera;
		m_fTransitionTime = fTime;
		m_ulStartTime = TheTimeMgr->GetAbsoluteTime();

		const Transform4D& startTransform = startCamera.GetNodeTransform();
		SetNodeTransform( startTransform );

		Vector3D direction = startTransform[2];
		ConvertVectorToEuelerAngles( direction, m_fStartAzimuth, m_fStartAltitude );

		m_bStartFinalTransition = true;
		m_pCallback = pCallback;

		world->SetCamera( this );
	}
}

void CameraBlender::ConvertVectorToEuelerAngles( Vector3D& v3dVector, float& fAzimuth, float &fAltitude )
{
	// Calculate the azimuth to the end camera
	fAzimuth = atan2f( v3dVector.y, v3dVector.x );
	NormalizeAngle( fAzimuth );

	// Calculate the altitude to the end camera
	if( (fAzimuth > K::two_pi_over_15) && (fAzimuth < (K::pi_over_2 + K::two_pi_over_15)) )
	{
		fAltitude = atan2f( v3dVector.z, v3dVector.y ); // if the azimuth is pointing up the Y axis, use the Y value to calculate the altitude
	}

	else if( (fAzimuth > (K::three_pi_over_2 - K::two_pi_over_15)) && (fAzimuth < (K::three_pi_over_2 + K::two_pi_over_15)) )
	{
		fAltitude = atan2f( v3dVector.z, v3dVector.y ); // if the azimuth is pointing down the Y axis, use the Y value to calculate the altitude
	}

	else
	{
		fAltitude = atan2f( v3dVector.z, v3dVector.x ); // the azimuth is not on the Y axis, so just use the X value to calculate the altitude
	}

	NormalizeAngle( fAltitude );

	// Sanitize the altitude (must be in quadrant 1 and 4)
	if( fAltitude > K::pi_over_2 && fAltitude <= K::pi ) {

		fAltitude = K::pi_over_2 - (K::pi - fAltitude); // reflect it over the Y axis from quadrant 2 to 1
	}

	if( fAltitude > K::pi && fAltitude <= K::three_pi_over_2 ) {

		fAltitude = K::three_pi_over_2 + (K::three_pi_over_2 - fAltitude); // reflect it over the Y axis from quadrant 3 to 4
	}

	// If in quadrant 4, the altitude must be a negative value from 0 to -pi/2
	if( fAltitude > K::three_pi_over_2 ) {

		fAltitude -= K::two_pi;
	}

}

void CameraBlender::Move( void )
{
	unsigned long curTime = TheTimeMgr->GetAbsoluteTime();

	float perc = (curTime - m_ulStartTime) / m_fTransitionTime;

	if( perc > 1 ) {

		m_pEndCamera->SetWorldTransform( m_pEndCamera->GetNodeTransform() );
		TheWorldMgr->GetWorld()->SetCamera( m_pEndCamera );

		if( m_pCallback ) {

			m_pCallback();
		}

	} else {

		m_pEndCamera->Move();

		// Calculate the new azimuth and altitude for the blending camera

		float azimuth = GetCameraAzimuth();
		float altitude = GetCameraAltitude();

		// The first phase of blending the camera's orientation is to face the camera towards the end camera
		if( perc < START_SECONDARY_BLEND ) {

			Vector3D vectorToEndCamera = m_pEndCamera->GetNodePosition() - m_pStartCamera->GetNodePosition();

			ConvertVectorToEuelerAngles( vectorToEndCamera, azimuth, altitude );

			float blendPerc = (perc) / (END_INITIAL_BLEND);
			if( blendPerc > 1.0F ) blendPerc = 1.0F;

			azimuth = ( m_fStartAzimuth * (1 - blendPerc) ) + ( azimuth * blendPerc );
			altitude = ( m_fStartAltitude * (1 - blendPerc) ) + ( altitude * blendPerc );

			NormalizeAngle( azimuth );
		}

		// The second phase consists of moving the camera to the end camera's position
		if( perc >= END_INITIAL_BLEND ) {

			Point3D startPos = m_pStartCamera->GetNodePosition();
			Point3D endPos = m_pEndCamera->GetNodePosition();

			float blendPerc = (perc - END_INITIAL_BLEND) / (START_SECONDARY_BLEND - END_INITIAL_BLEND);
			if( blendPerc > 1.0F ) blendPerc = 1.0F;

			SetNodePosition( ( startPos * (1 - blendPerc ) ) + ( endPos * blendPerc ) );
			Invalidate();
		}

		// The last phase of blending the camera's orientation so that it matches the end camera's orientation
		if( perc >= START_SECONDARY_BLEND ) {

			if( m_bStartFinalTransition ) {

				m_bStartFinalTransition = false;

				m_fStartAzimuth = GetCameraAzimuth();
				m_fStartAltitude = GetCameraAltitude();
			}

			Vector3D endCameraDirection = m_pEndCamera->GetNodeTransform()[2];

			ConvertVectorToEuelerAngles( endCameraDirection, azimuth, altitude );

			float blendPerc = (perc - START_SECONDARY_BLEND) / (1.0F - START_SECONDARY_BLEND);

			azimuth = ( m_fStartAzimuth * (1 - blendPerc) ) + ( azimuth * blendPerc );
			altitude = ( m_fStartAltitude * (1 - blendPerc) ) + ( altitude * blendPerc );

			NormalizeAngle( azimuth );
		}

		SetCameraAzimuth( azimuth );
		SetCameraAltitude( altitude );
	}

	OrientedCamera::Move();
}
