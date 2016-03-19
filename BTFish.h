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


#ifndef BTFish_h
#define BTFish_h

#include "BTControllers.h"
#include "BTGoalFishWander.h"

namespace C4
{
	/** 
	 * This class represents a simple fish that 
	 * randomly swims around a map.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class SimpleFishController : public BTEntityController, public LinkTarget<SimpleFishController>
	{
		protected:

			// Positioning/Motion
			BTSteering			m_objSteering;

			// AI
			BTGoalFishWander		m_objBaseGoal;

			// Animation
			//FrameAnimator		*m_pFrameAnimator;

			//FrameAnimator *GetFrameAnimator( void ) const
			//{
			//	return (m_pFrameAnimator);
			//}

			SimpleFishController( const SimpleFishController& pController );
			Controller *Replicate( void ) const;

		public:

			SimpleFishController();
			~SimpleFishController();

			virtual void Pack(Packer& data, unsigned long packFlags) const;
			void Unpack(Unpacker& data, unsigned long unpackFlags);

			virtual void Preprocess(void);
			virtual void Move(void);

			BTSteering *GetSteering( void )
			{
				return (&m_objSteering);
			}

			//virtual void SetAnimation( EntityMotion motion );

			// Which nodes this controller can be placed on.
			static bool ValidNode( const Node *node );

			// Settings
			long GetSettingCount( void ) const;
			Setting *GetSetting( long index ) const;
			void SetSetting( const Setting *setting );
	};
}


#endif
