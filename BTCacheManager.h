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


#ifndef BTCacheManager_h
#define BTCacheManager_h

#include "C4Array.h"

namespace C4
{

	enum CacheType
	{
		kCacheUnits			= 1 << 0,
		kCacheCommanders	= 1 << 1,
		kCacheBases			= 1 << 2,
		kCachePlots			= 1 << 3,
		kCacheBuildings		= 1 << 4,

		kCacheAll			= kCacheUnits | kCacheCommanders | kCacheBases | kCachePlots | kCacheBuildings
	};

	/**
	 * This class will store various entities that 
	 * we need quick access to.
	 * @author - Frank Williams (demalus@gmail.com)
	 */
	class CacheManager : public Singleton<CacheManager>
	{
		private:

			// Each cache will store the controller ID of the entity.

			Array<long>	m_arrUnitsT1;
			Array<long>	m_arrUnitsT2;
			Array<long>	m_arrCommandersT1;
			Array<long>	m_arrCommandersT2;
			Array<long>	m_arrBasesT1;
			Array<long>	m_arrBasesT2;
			Array<long>	m_arrPlotsT1;
			Array<long>	m_arrPlotsT2;
			Array<long>	m_arrBuildingsT1;
			Array<long>	m_arrBuildingsT2;

		public:

			CacheManager();
			~CacheManager();	

			Array<long> *GetGameCache( CacheType nType, long nTeam );
			void Purge( void );
	};

	extern CacheManager *TheGameCacheMgr;
}


#endif
