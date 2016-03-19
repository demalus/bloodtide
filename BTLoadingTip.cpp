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

#include "BTLoadingTip.h"

using namespace C4;

LoadingTip *C4::TheLoadingTip = nullptr;

/* -------------- Loading Tip -------------- */

LoadingTip::LoadingTip( const StringHeader *pTips ) : 
	Interface(),
	Singleton<LoadingTip>(TheLoadingTip)
{
	if( pTips != nullptr ) {
		const char* tex = "<missing>";

		int numTips = Text::StringToInteger( pTips->GetString() );
		int randi = Math::Random( numTips );

		pTips = pTips->GetFirstSubstring();
		for( int x = 0; x < numTips; x++ ) {
			if( x == randi ) {
				tex = pTips->GetString();
				break;
			}

			pTips = pTips->GetNextString();
		}

		if( !Text::CompareTextCaseless( tex, "<missing>" ) ) {
			m_objTip.SetTexture( tex );
			m_objTip.SetElementSize( TheDisplayMgr->GetDisplayWidth(), TheDisplayMgr->GetDisplayHeight() );
			AddSubnode( &m_objTip );
		}
	}
}

LoadingTip::~LoadingTip()
{

}

void LoadingTip::New( const StringHeader *pTips )
{
	if( TheLoadingTip == nullptr ) {
		Interface *interface = new LoadingTip( pTips );
		TheInterfaceMgr->AddElement(interface);
	}
}

