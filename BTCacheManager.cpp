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

#include "BTCacheManager.h"

using namespace C4;

CacheManager *C4::TheGameCacheMgr = nullptr;


CacheManager::CacheManager() :
	Singleton<CacheManager>(TheGameCacheMgr)
{
}

CacheManager::~CacheManager()
{
}

// Get a game cache based on the type.
Array<long> *CacheManager::GetGameCache( CacheType nType, long nTeam )
{
	switch( nType )
	{
		case kCacheUnits:

			if( nTeam == 1 ) {
				return (&m_arrUnitsT1);
			}
			else if( nTeam == 2 ) {
				return (&m_arrUnitsT2);
			}
			
			return nullptr;

		case kCacheCommanders:

			if( nTeam == 1 ) {
				return (&m_arrCommandersT1);
			}
			else if( nTeam == 2 ) {
				return (&m_arrCommandersT2);
			}
			
			return nullptr;

		case kCacheBases:

			if( nTeam == 1 ) {
				return (&m_arrBasesT1);
			}
			else if( nTeam == 2 ) {
				return (&m_arrBasesT2);
			}
			
			return nullptr;

		case kCachePlots:

			if( nTeam == 1 ) {
				return (&m_arrPlotsT1);
			}
			else if( nTeam == 2 ) {
				return (&m_arrPlotsT2);
			}
			
			return nullptr;

		case kCacheBuildings:

			if( nTeam == 1 ) {
				return (&m_arrBuildingsT1);
			}
			else if( nTeam == 2 ) {
				return (&m_arrBuildingsT2);
			}
			
			return nullptr;

		default:
			return nullptr;
	}

	return nullptr;
}

void CacheManager::Purge( void )
{
	m_arrUnitsT1.Purge();
	m_arrUnitsT2.Purge();
	m_arrCommandersT1.Purge();
	m_arrCommandersT2.Purge();
	m_arrBasesT1.Purge();
	m_arrBasesT2.Purge();
	m_arrPlotsT1.Purge();
	m_arrPlotsT2.Purge();
	m_arrBuildingsT1.Purge();
	m_arrBuildingsT2.Purge();
}