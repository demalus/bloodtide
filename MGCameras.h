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


#ifndef MGCameras_h
#define MGCameras_h


#include "C4Cameras.h"


namespace C4
{

	// Navigation flags for the RTSCamera
	enum NavigationFlag {

		kNavUp		= 1 << 0,
		kNavLeft	= 1 << 1,
		kNavDown	= 1 << 2,
		kNavRight	= 1 << 3
	};

	/*
	* RTSCamera functions as an oriented camera, but allows others to move the camera by setting its movement flags.
	* @author Chris Williams
	*/
	class RTSCamera : public OrientedCamera
	{
		private:

			unsigned long			m_lNavigationFlags; // see navigation flags below
			float					m_fCeiling; // max Z-value
			bool					m_bEnabled;


		public:

			RTSCamera();

			unsigned long GetNavigationFlags() const {return m_lNavigationFlags;}
			void SetNavigationFlags(unsigned long lNavigationFlags) {m_lNavigationFlags = lNavigationFlags;}

			// The camera's world position is set to the given world position.
			// The camera's ceiling is set to the Z-value in the given world position.
			// Also initializes the camera's altitude and azimuth.
			void Initialize(Point3D &objWorldPosition);

			void SetEnabled(bool enabled) { m_bEnabled = enabled; }


			virtual void Move(void);
	};

	/*
	* Blends from one camera to another camera over a given amount of time.
	* @author Chris Williams
	* @author Frank Williams - (demalus@gmail.com)
	*/
	class CameraBlender : public OrientedCamera
	{
		public:

			typedef void BlenderCallback( void );

		private:

			FrustumCamera				*m_pStartCamera;
			FrustumCamera				*m_pEndCamera;

			float						m_fTransitionTime;
			unsigned long				m_ulStartTime;

			float						m_fStartAzimuth;
			float						m_fStartAltitude;

			bool						m_bStartFinalTransition; // when true, and in the last phase, initial book-keeping will be done (getting the starting azimuth/altitude to do the blending)

			BlenderCallback				*m_pCallback;

			void ConvertVectorToEuelerAngles( Vector3D& v3dVector, float& fAzimuth, float &fAltitude );

		public:

			CameraBlender();

			// transition time is in milliseconds
			void Initialize( FrustumCamera& startCamera, FrustumCamera& endCamera, float fTransitionTime, BlenderCallback *pCallback = nullptr );

			void Move( void );
	};
}


#endif
