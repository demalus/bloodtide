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

#ifndef BTLoadingTip_h_
#define BTLoadingTip_h_


#include "C4Interface.h"


namespace C4 {

	class LoadingTip : public Interface, Singleton<LoadingTip>
	{
	private:

		ImageElement		m_objTip;


		LoadingTip( const StringHeader *pTips );

	public:

		~LoadingTip();

		static void New( const StringHeader *pTips );

		ImageElement *GetImage( void ) 
		{
			return (&m_objTip);
		}
	};


	extern LoadingTip *TheLoadingTip;
}

#endif

