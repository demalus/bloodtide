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

#ifndef BTInterface_h_
#define BTInterface_h_

#include "StatsProtocol.h"
#include "C4Interface.h"
#include "C4Primitives.h"

namespace C4
{
	// forward declerations
	class StatsProtocol;

	/*
	 * An image element that grows when the cursor is hovering over it and shrinks when the cursor stops hovering over it.
	 * Width and height are assumed to be equal.
	 * In order for the image to handle mouse events, you must subclass and define a TestPickPoint(const Point3D &p) function
	 * @author Chris Williams
	 */
	class ResizingImage : public Interface
	{
		private:

			float					m_fDefaultSize; // default width and height
			float					m_fMaxSize;
			float					m_fTimeToMax; // how long it should take to reach max size from default size (in milliseconds)

			unsigned long			m_ulHoverEventTime; // the time when the mouse entered or exited the icon
			float					m_fIncreasedSize; // how much the icon increased while the mouse was hovering over it
			float					m_fTimeFromSize; // time it will take to reach max size depending on current size

			Point3D					m_p3dInitialPosition;

		protected:

			ImageElement			m_objImage;

		public:

			ResizingImage( const char *strIcon, float fDefaultSize, float fMaxSize, float fTimeToMax );

			ImageElement &GetImage( void )
			{
				return (m_objImage);
			}

			void Hilite( void ); // increases the size of the image to the max size
			void Unhilite( void ); // decreases the size of the image back to the default size

			virtual void HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p);

			virtual void InterfaceTask(void);
	};

	class ProgressBar : public Interface
	{
		protected:

			ImageElement					m_objProgressBar;
			ImageElement					m_objProgress;

			const TextureHeader				*m_pProgressHeader;
			Pixel							*m_pProgressImage;

			Array<Array<long>*>				*m_arrProgressLines; // holds rows, columns or diagonals of the health bar
			long							m_lPreviousIndex; // the index of the last line of pixels that was drawn on the health bar

		public:

			ProgressBar( const char *strProgressBar, const char *strProgress );
			virtual ~ProgressBar();

			void UpdateProgress( long newIndex );
			void ResetProgress( void );
	};


	class HealthBar : public ProgressBar
	{
		private:

			Link<StatsProtocol>				m_pStatsController;

			bool							m_bShowValue;
			TextElement						m_objHealthValue;

		public:

			HealthBar( const char *strHealthBar, const char *strHealth );
			~HealthBar();

			void HandleDisplayUpdate( void );

			void SetStatsController( StatsProtocol *pStatsController );

			void SetShowValue( bool show )
			{
				m_bShowValue = show;
			}

			void InterfaceTask( void );
	};

	class MiniHealthBar : public Node
	{
		private:

			PlateGeometry				m_objHealth;

			Point3D						m_p3dOffset;

		public:

			MiniHealthBar( Point3D& p3dOffset, float width, float height );

			const Point3D& GetBarOffset( void ) const
			{
				return (m_p3dOffset);
			}

			void Update( Point3D curPosition, float perc );
	};

	class SelectionRing : public Node
	{
		private:

			AnnulusGeometry				m_objRing;

			Point3D						m_p3dOffset;

		public:

			SelectionRing( Point3D& p3dOffset, float fRadius );

			const Point3D& GetRingOffset( void ) const
			{
				return (m_p3dOffset);
			}

			void UpdateHealth( float perHealth );
	};
}

#endif
