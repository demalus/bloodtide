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

#ifndef StatEffect_h
#define StatEffect_h

#include "C4Link.h"

namespace C4
{
	// Forward declarations
	class BTEntityController;

	/**
	 * This base class is for effects on a stats controller.
	 * Examples of effects are heals over time, stat buffs, etc.
	 * A stat can do whatever it wants, but it *must* have an enter
	 * and exit routine when it is applied or removed from a stats 
	 * controller.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class StatEffect
	{
		protected:

			// The owner of a stat effect is the entity to which the stat effect is applied to
			BTEntityController				*m_pOwner;

			// The controller index of the creator of the stat effect
			long							m_lSource;

		public:

			StatEffect( BTEntityController *pOwner, long lSource ) : m_pOwner( pOwner ), m_lSource( lSource ) { }

			void SetOwner( BTEntityController *pOwner ) { m_pOwner = pOwner; }
			
			virtual void OnApplyEffect( void ) = 0;		// does something as soon as it is applied
			virtual void OnRemoveEffect( void ) = 0;	// does something when the stat is removed
			virtual void UpdateEffect( unsigned long ulTime ) = 0;		// does something every iteration/tick

			virtual StatEffect *Clone( void ) = 0;
	};
}

#endif