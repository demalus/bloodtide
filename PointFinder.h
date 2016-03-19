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


#ifndef PointFinder_h
#define PointFinder_h

#include "C4Vector3D.h"
#include "C4Array.h"
#include "MGGame.h"
#include "BTMath.h"

namespace C4
{


	class PointFinder
	{
		private:

			PointFinder() {}

			/** 
			 * When creating safe points, it is important to check old safe points.
			 * For convenience, the SafePointFinder can find multiple points and checks
			 * to see if there are any obstructions.  Usually, it is used to place another
			 * entity in said safe point - so when finding multiple points, these new objects
			 * won't show up until *after* all of the safe points have been generated - which
			 * causes things to be on top of each other.  It is therefore necessary to check
			 * previous safe points with the same information that the regular PointFinder
			 * requires.
			 **/
			static bool CheckPreviousSafePoints( Point3D& p3dPoint, Array<Point3D>& arrSafePoints, float& fSafeDist, bool bIs3D = true ) 
			{
				int nLength = arrSafePoints.GetElementCount();
				for( int x = 0; x < nLength; x++ ) {
					
					// If the first object needed a safe distance, than another object needs to obey its safe distance, plus
					// the safe distance of a previously placed object.
					float safe_dist = 2 * fSafeDist;  

					if( bIs3D ) {
						if( Calculate3DDistance(p3dPoint, arrSafePoints[x]) <= safe_dist ) {
							return false;
						}
					}
					else {
						if( Calculate2DDistance(p3dPoint, arrSafePoints[x]) <= safe_dist ) {
							return false;
						}
					}
				}

				return true;
			}

		public:

			/** 
			 * Given some location, find some number of locations around that point that
			 * are safe, i.e. have no units or obstructions on them.
			 * Note: This does not factor in the Z (it is taken from the supplied point).
			 * p3dPoint - Initial point. Safe points will be found around this point.
			 * nPointsToFind - Number of safe points that should be generated.
			 * arrSafePoints - Safe points will be stored here.
			 * fInitialRadius - This function works by checking circles around the initial point. This 
			 *   is the radius of the very first circle.
			 * fStepRadius - After the initial radius, the circles will increase in radius by this amount.
			 * fRadians - How many radians each check should increment by.  For example, if this value is pi, 
			 *   then there will only be at most two safe points per circle layer.
			 * fMaxRadius - If a point isn't found by the time the circles reach this radius, then fail.
			 * fSafeDist - This is the closest distance to an obstruction to find a safe point.
			 * fPreferredAngle - The PointFinder will select points closest to this angle first.
			 * @author Frank Williams - (demalus@gmail.com)
			 */
			static bool FindSafePoint_Spiral( Point3D p3dPoint, int nPointsToFind, Array<Point3D>& arrSafePoints, float fInitialRadius, float fStepRadius, float fRadians, float fMaxRadius, float fSafeDist, float fPreferredAngle, bool bIs3D = true )
			{
				int pointsFound = 0;
				float radius = fInitialRadius;
				float curAngle = 0.0F;

				GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
				if( !world ) {
					return false;
				}

				while( pointsFound < nPointsToFind )
				{
					Vector3D offset(1,0,0);  // Unit vector
					offset.RotateAboutZ( curAngle + fPreferredAngle );
					offset *= radius;

					Point3D pointToTest = p3dPoint + offset;

					if( world->PointWithinMapBounds(pointToTest) ) {
						// If no obstruction is on the spot, it is safe.
						BTCollisionData data;
						Array<long> ignoreConts;
						TheGame->DetectEntity( pointToTest, data, kCacheUnits | kCacheCommanders, fSafeDist, ignoreConts, bIs3D );
						if( data.m_State == kCollisionStateNone ) {
							if( CheckPreviousSafePoints(pointToTest, arrSafePoints, fSafeDist, bIs3D) ) {
								arrSafePoints.AddElement( pointToTest );
								pointsFound++;
							}
						}
						else {
							// Ignore moving units.
							UnitController *unit = static_cast<UnitController*>( data.m_pController );
							if( unit ) {
								if( unit->GetAnimation() == kMotionMove ) {
									if( CheckPreviousSafePoints(pointToTest, arrSafePoints, fSafeDist, bIs3D) ) {
										arrSafePoints.AddElement( pointToTest );
										pointsFound++;
									}
								}
							}
						}
					}

					
					if( curAngle <= 0 ) {
						// If the circle has been traversed step up the radius,
						// otherwise traverse the circle some more.
						if( curAngle <= -K::pi ) {
							curAngle = 0.0F;
							radius += fStepRadius;
							if( radius > fMaxRadius ) {
								return false;
							}
						}
						else {
							curAngle *= -1.0F;
							curAngle += fRadians;
						}
					}
					else {
						// Basically, the algorithm starts at 0 and goes up by theta, then
						// down by negative theta, then up by two theta, then down by negative
						// two theta, etc...
						// This will pick a spot closest to the preferred angle first.
						curAngle *= -1.0F;
					}
				}

				return false;
			}


			/** 
			 * This method is used to find an avoid_collision point. It basically tries next to some unit
			 * blocking the path and keeps trying points to the sides. If it encounters another unit, it tries
			 * points to that unit's sides as well.
			 * Note: This does not factor in the Z (it is taken from the supplied point).
			 * pMover - The entity trying to find a way through.
			 * pBlocker - The initial entity in the way.
			 * arrSafePoints - Safe points will be stored here.
			 * fSafeDist - This is the closest distance to an obstruction to find a safe point.
			 * @author Frank Williams - (demalus@gmail.com)
			 */
			static bool FindSafePoint_Wall( BTEntityController *pMover, BTEntityController *pBlocker, Array<Point3D>& arrSafePoints, float& fSafeDist, bool bIs3D = true )
			{
				if( !pMover ) {
					return false;
				}

				if( !pBlocker ) {
					return false;
				}

				BTCylinderCollider *myCol = static_cast<BTCylinderCollider*>( pMover->GetCollider() );
				if( !myCol ) {
					return false;
				}

				int pointsFound = 0;
				bool isRight = true;
				BTEntityController *left = pBlocker;
				BTEntityController *right = pBlocker;

				while( true )
				{
					BTEntityController *target = nullptr;
					if( isRight ) {
						target = right;
					}
					else {
						target = left;
					}

					BTCylinderCollider *tCol = static_cast<BTCylinderCollider*>( target->GetCollider() );
					if( !tCol ) {
						isRight = !isRight;
						continue;
					}

					Vector3D toTar = tCol->GetPosition() - myCol->GetPosition();
					float ftar = atan2f( toTar.y, toTar.x );
					NormalizeAngle( ftar );

					Vector3D offset(1,0,0); 
					if( isRight ) {
						offset.RotateAboutZ( ftar - K::pi_over_2 );
					}
					else {
						offset.RotateAboutZ( ftar + K::pi_over_2 );
					}
					offset *= ( tCol->GetRadius() + myCol->GetRadius() + ENT_AVOID_DIST );

					Point3D pointToTest = tCol->GetPosition() + offset;

					// If no obstruction is on the spot, it is safe.
					BTCollisionData data;
					Array<long> ignoreConts;
					if( isRight	) {
						ignoreConts.AddElement( right->GetControllerIndex() );
					}
					else {
						ignoreConts.AddElement( left->GetControllerIndex() );
					}

					TheGame->DetectEntity( pointToTest, data, kCacheUnits | kCacheCommanders, fSafeDist, ignoreConts, bIs3D );

					if( data.m_State == kCollisionStateNone ) {
						arrSafePoints.AddElement( pointToTest );
						return true;
					}
					else {
						if( data.m_pController ) {
							if( isRight ) {
								right = data.m_pController;
							}
							else {
								left = data.m_pController;
							}
						}

						if( right == left ) {
							return false;
						}

						isRight = !isRight;
					}	
				}

				return false;
			}

			/** 
			 * Find some random points within the map.
			 * nPointsToFind - Number of safe points that should be generated.
			 * arrSafePoints - Safe points will be stored here.
			 * @author Frank Williams - (demalus@gmail.com)
			 */
			static bool FindSafePoint_Random( int nPointsToFind, Array<Point2D>& arrSafePoints )
			{
				int pointsFound = 0;

				GameWorld *world = static_cast<GameWorld*>( TheWorldMgr->GetWorld() );
				if( !world ) {
					return false;
				}

				Marker *bounds = world->GetMapBounds();
				if( !bounds ) {
					return false;
				}

				MapBoundsController *mbc = static_cast<MapBoundsController*>( bounds->GetController() );
				if( !mbc ) {
					return false;
				}

				float x = bounds->GetNodePosition().x;
				float y = bounds->GetNodePosition().y;
				float width = mbc->GetTopRightVector().x;
				float length = mbc->GetTopRightVector().y;

				while( pointsFound < nPointsToFind )
				{
					float randX = Math::RandomFloat( width ) + x;
					float randY = Math::RandomFloat( length ) + y;
				
					arrSafePoints.AddElement( Point2D(randX, randY) );
					pointsFound++;
				}

				return true;
			}
	};
}

#endif