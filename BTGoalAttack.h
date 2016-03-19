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


#ifndef BTGoalAttack_h
#define BTGoalAttack_h

#include "BTGoal.h"
#include "C4Link.h"
#include "BTCharacter.h"

namespace C4
{
	/** 
	 * Goal for attacking some entity.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTGoalAttack : public BTGoal
	{
		private:

			Link<BTEntityController> m_pTarget;
			Link<BTEntityController> m_pOwner;

			bool AttackValid( void );

		public:

			BTGoalAttack( BTEntityController *pTarget, BTEntityController *pOwner );
			~BTGoalAttack();

			virtual void Enter( void );
			virtual void Exit( void );

			virtual long Process( void );

			BTEntityController *GetTarget( void ) const
			{
				return ( m_pTarget );
			}

			void SetTarget( BTEntityController *pTarget )
			{
				m_pTarget = pTarget;
			}
	};
}

#endif