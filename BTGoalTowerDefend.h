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


#ifndef BTGoalTowerDefend_h
#define BTGoalTowerDefend_h

#include "BTGoal.h"
#include "C4Link.h"
#include "BTCharacter.h"

namespace C4
{
	/** 
	 * Goal for guard towers to defend.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class BTGoalTowerDefend : public BTGoal
	{
		private:

			Link<BTEntityController> m_pOwner;

			float m_fRange;		// range in meters of attacking
			float m_fSpeed;		// delay in seconds between attacks
			float m_fDamage;	// attack damage
			const char *m_pSound;	// sound that plays when attacking

			unsigned long m_ulNextAttack;			// time of next valid attack
			Link<BTEntityController> m_pTarget;		// currently targeted enemy

		public:

			BTGoalTowerDefend( BTEntityController *pOwner, float fRange, float fSpeed, float fDamage, const char *pSound );
			~BTGoalTowerDefend();

			virtual void Enter( void );
			virtual void Exit( void );

			virtual long Process( void );

			bool ValidTarget( void );
	};
}

#endif