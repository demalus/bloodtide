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

#ifndef BTMath_h
#define BTMath_h

#include "C4Constants.h"

namespace C4
{


	/** 
	 * Clamps an angle between 0 and two pi.
	 * @author Frank Williams - (demalus@gmail.com)
	 */
	static void NormalizeAngle( float& fAngle )
	{
		if( fAngle > K::two_pi ) {
			fAngle -= K::two_pi;
		}
		else if( fAngle < 0 ) {
			fAngle += K::two_pi;
		}
	}

	/** 
	 * Get the 2D distance between two points.
	 * @author Frank Williams - (demalus@gmail.com)
	 */
	static float Calculate2DDistance( float x1, float y1, float x2, float y2 )
	{
		return Sqrt( Pow((x2 - x1), 2) + Pow((y2 - y1), 2) );
	}

	/** 
	 * Get the 2D distance between two Point3Ds.
	 * @author Frank Williams - (demalus@gmail.com)
	 */
	static float Calculate2DDistance( const Point3D& p1, const Point3D& p2 )
	{
		return Calculate2DDistance( p1.x, p1.y, p2.x, p2.y );
	}

	/** 
	 * Get the 3D distance between two points.
	 * @author Frank Williams - (demalus@gmail.com)
	 */
	static float Calculate3DDistance( float x1, float y1, float z1, float x2, float y2, float z2 )
	{
		return Sqrt( Pow((x2 - x1), 2) + Pow((y2 - y1), 2) + Pow((z2 - z1), 2) );
	}

	/** 
	 * Get the 3D distance between two Point3Ds.
	 * @author Frank Williams - (demalus@gmail.com)
	 */
	static float Calculate3DDistance( const Point3D& p1, const Point3D& p2 )
	{
		return Calculate3DDistance( p1.x, p1.y, p1.z, p2.x, p2.y, p2.z );
	}

	// Calculate magnitude of a vector for only x and y.
	inline float Magnitude2D(const Vector3D& v)
	{
		return (Sqrt(v.x * v.x + v.y * v.y));
	}

	// Calculate magnitude of a 2D vector.
	inline float Magnitude2D(const Vector2D& v)
	{
		return (Sqrt(v.x * v.x + v.y * v.y));
	}

}

#endif