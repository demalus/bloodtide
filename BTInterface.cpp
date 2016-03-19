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


#include "BTInterface.h"
#include "C4Time.h"
#include "BTCursor.h"

using namespace C4;

ResizingImage::ResizingImage(  const char *strIcon, float fDefaultSize, float fMaxSize, float fTimeToMax  ) :
	m_fDefaultSize( fDefaultSize ),
	m_fMaxSize( fMaxSize ),
	m_fTimeToMax( fTimeToMax ),
	m_ulHoverEventTime( -1 ),
	m_fIncreasedSize( -1 )
{
	m_objImage.SetElementSize( m_fDefaultSize, m_fDefaultSize );
	m_objImage.SetTexture(strIcon);

	AddSubnode( &m_objImage );
}

void ResizingImage::HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p)
{
	if( eventType == kEventCursorEnter ) {

		Hilite();
	}

	else if( eventType == kEventCursorExit ) {

		Unhilite();
	}
}

void ResizingImage::Hilite( void )
{
	m_ulHoverEventTime = TheTimeMgr->GetAbsoluteTime();
	m_fIncreasedSize = m_objImage.GetElementWidth();
	m_fTimeFromSize = m_fTimeToMax / ((m_fMaxSize - m_fDefaultSize) / (m_fMaxSize - m_fIncreasedSize));

	SetElementState( GetElementState() | kElementHilited );
}

void ResizingImage::Unhilite( void )
{
	m_ulHoverEventTime = TheTimeMgr->GetAbsoluteTime();
	m_fIncreasedSize = m_objImage.GetElementWidth();
	m_fTimeFromSize = m_fTimeToMax / ((m_fMaxSize - m_fDefaultSize) / (m_fIncreasedSize - m_fDefaultSize));

	SetElementState( GetElementState() & ~kElementHilited );
}

void ResizingImage::InterfaceTask(void)
{
	const Point3D iconPos = m_objImage.GetElementPosition();

	float width = m_objImage.GetElementWidth();

	if( GetElementState() & kElementHilited ) {

		if( width < m_fMaxSize ) {

			unsigned long curTime = TheTimeMgr->GetAbsoluteTime();
			float percentage = (curTime - m_ulHoverEventTime) / m_fTimeFromSize;
			float increase = percentage * (m_fMaxSize - m_fIncreasedSize);
			float newWidth = m_fIncreasedSize + increase;

			if( newWidth > m_fMaxSize ) newWidth = m_fMaxSize;

			float posOffset = (m_fDefaultSize - newWidth) / 2.0F;

			m_objImage.SetElementSize( newWidth, newWidth );
			m_objImage.SetElementPosition( Point3D( posOffset, posOffset, 0.0F ) );
			m_objImage.Invalidate();
		}

	} else {

		if( width > m_fDefaultSize ) {

			unsigned long curTime = TheTimeMgr->GetAbsoluteTime();
			float percentage = (curTime - m_ulHoverEventTime) / m_fTimeFromSize;
			float decrease = percentage * (m_fIncreasedSize - m_fDefaultSize);
			float newWidth = m_fIncreasedSize - decrease;

			if( newWidth < m_fDefaultSize ) newWidth = m_fDefaultSize;

			float posOffset = (newWidth - m_fDefaultSize) / -2.0F;

			m_objImage.SetElementSize( newWidth, newWidth );
			m_objImage.SetElementPosition( Point3D( posOffset, posOffset, 0.0F ) );
			m_objImage.Invalidate();
		}
	}
}

/* -------------- Progress Bar -------------- */

ProgressBar::ProgressBar(const char *strProgressBar, const char *strProgress) : Interface(),
		m_lPreviousIndex(0)
{
	/* ---- Progress Bar ---- */

	m_objProgressBar.SetTexture(strProgressBar);

	Texture *texture = m_objProgressBar.GetTexture();
	m_objProgressBar.SetElementSize(texture->GetTextureWidth(), texture->GetTextureHeight());

	AddSubnode(&m_objProgressBar);


	SetElementSize(texture->GetTextureWidth(), texture->GetTextureHeight());

	/* ---- Progress --- */

	Texture *progress = Texture::Create(strProgress);

	long width = progress->GetTextureWidth();
	long height = progress->GetTextureHeight();
	long pixelCount = width * height;

	/* Decompress the image */
	const TextureHeader *header = progress->GetTextureHeader();
	Render::Decompressor *decompressor = progress->GetDecompressor( header, header->GetMipmapData() );

	void *image = progress->GetImagePointer();
	char *storage = nullptr;

	storage = new char[pixelCount * sizeof(Pixel)];
	(*decompressor)(static_cast<const unsigned char *>(image), storage, pixelCount);
	/* End decompression */ 

	// Free up data that is no longer needed 
	delete[] reinterpret_cast<char *>(image);
	progress->Release();
	//

	// store the header and the pixel image for later use
	m_pProgressHeader = header;
	m_pProgressImage = static_cast<Pixel *>( (void*)storage );
	//

	// now that we have a pixel image and its header in memory,
	// allow a subclass to partition the image into rows/columns/diagonals/circle ETC
	// the subclass will have to store this partitioning into this structure
	m_arrProgressLines = new Array<Array<long>*>(width); 

	m_objProgress.SetTexture(header, m_pProgressImage);
	m_objProgress.SetElementSize(header->imageWidth, header->imageHeight);

	AddSubnode(&m_objProgress);
}

ProgressBar::~ProgressBar()
{
	delete[] reinterpret_cast<const char *>(m_pProgressHeader);
	delete[] reinterpret_cast<char *>(m_pProgressImage);

	for(natural a = 0; a < m_arrProgressLines->GetElementCount(); a++) {

		Array<long> *pixels = (*m_arrProgressLines)[a];
		delete pixels;
	}

	delete m_arrProgressLines;
}

void ProgressBar::UpdateProgress( long newIndex )
{
	long max = m_arrProgressLines->GetElementCount() - 1;
	if( newIndex != 0 ) newIndex -= 1;
	if( newIndex > max ) newIndex = max;

	if( newIndex == m_lPreviousIndex )
		return;

	if( newIndex > m_lPreviousIndex ) {

		for(natural a = m_lPreviousIndex; a <= newIndex; a++) {

			Array<long> *pixels = (*m_arrProgressLines)[a];

			for(natural x = 0; x < pixels->GetElementCount(); x++) {

				Pixel p = m_pProgressImage[ (*pixels)[x] ];
				p = RGBAColor( GetRedComponent(p), GetGreenComponent(p), GetBlueComponent(p), 255);

				m_pProgressImage[ (*pixels)[x] ] = p;
			}
		}

	} else {

		for(natural a = m_lPreviousIndex; a >= newIndex; a--) {

			Array<long> *pixels = (*m_arrProgressLines)[a];

			for(natural x = 0; x < pixels->GetElementCount(); x++) {

				Pixel p = m_pProgressImage[ (*pixels)[x] ];
				p = RGBAColor( GetRedComponent(p), GetGreenComponent(p), GetBlueComponent(p), 0);

				m_pProgressImage[ (*pixels)[x] ] = p;
			}
		}
	}

	m_objProgress.SetTexture(nullptr, nullptr);
	m_objProgress.SetTexture(m_pProgressHeader, m_pProgressImage);

	m_lPreviousIndex = newIndex;
}

void ProgressBar::ResetProgress( void )
{
	//long max = m_arrProgressLines->GetElementCount() - 1;
	//m_lPreviousIndex = max;

	//UpdateProgress( m_lPreviousIndex )

	//int x = 5;
}


/* -------------- Health Bar -------------- */

HealthBar::HealthBar(const char *strHealthBar, const char *strHealth) : 
		ProgressBar( strHealthBar, strHealth ),
		m_pStatsController( nullptr ),
		m_bShowValue( true )
{
	long width = m_pProgressHeader->imageWidth;
	long height = m_pProgressHeader->imageHeight;

	Array<long> *newRow;

	for(natural x = 0; x < width; x++) {

		newRow = nullptr;

		for(natural y = 0; y < height; y++) {

			long index = x + (y * width);
			Pixel p = m_pProgressImage[index];

			if( GetAlphaComponent(p) != 0 ) {

				if( newRow == nullptr ) newRow = new Array<long>;

				newRow->AddElement(index);
			}
		}

		if( newRow ) m_arrProgressLines->AddElement(newRow);
	}

	UpdateProgress( m_arrProgressLines->GetElementCount() );
}

HealthBar::~HealthBar()
{

}

void HealthBar::SetStatsController(StatsProtocol *pStatsController)
{
	m_pStatsController = pStatsController;

	m_objHealthValue.SetTextColor(K::black);
	m_objHealthValue.SetFont( AutoReleaseFont("font/Gui") );
	m_objHealthValue.SetTextScale( 1.2F );
	m_objHealthValue.SetTextAlignment( kTextAlignCenter ); // TODO: this won't work, need to find the '/' and make sure that it never moves
	//m_objHealthValue.SetTextFlags(kTextWrapped);
	m_objHealthValue.SetElementSize( GetElementWidth(), 0.0F );

	AddSubnode( &m_objHealthValue );

	HandleDisplayUpdate();
	InterfaceTask();
}

void HealthBar::HandleDisplayUpdate( void )
{
	Point3D position( 0.0F, 0.0F, 0.0F );

	m_objHealthValue.SetElementPosition( Point3D( 0.0F, GetElementHeight() / 2.0F - 5.0F, 0.0F ) ); // TODO: - 5.0F ....
}

void HealthBar::InterfaceTask(void)
{
	if( m_pStatsController == nullptr ) {

		return;
	}

	float curHealth = m_pStatsController->GetHealth();
	float maxHealth = m_pStatsController->GetMaxHealth();

	// Update the health text box
	if( m_bShowValue ) 
	{
		String<> text;
		char buffer[32];
		long size = 0;

		size = (long)ceil(log10(curHealth)) + 1;
		if( size < 1 ) size = 1;

		size = Text::IntegerToString( (long)ceil(curHealth), buffer, size );
		text.Append( buffer, size );

		text.Append( " / ", 3 );

		size = (long)ceil(log10(maxHealth)) + 1;
		if( size < 1 ) size = 1;

		size = Text::IntegerToString( (long)ceil(maxHealth), buffer, size );
		text.Append( buffer, size );

		m_objHealthValue.SetText( text );
	}
	
	// Update the progress bar
	float perHealth = curHealth / maxHealth;
	if( perHealth < 0 ) perHealth = 0;

	long newIndex = (long) ceil(m_arrProgressLines->GetElementCount() * perHealth);

	UpdateProgress( newIndex );	
}

/* -------------- Mini Health Bar -------------- */

MiniHealthBar::MiniHealthBar( Point3D& p3dOffset, float width, float height ) :
		//m_objBackground( width, height ),
		m_objHealth( Vector2D(width, height) ),
		m_p3dOffset( p3dOffset )
{
	PlateGeometryObject *object = m_objHealth.GetObject();
	object->SetCollisionExclusionMask(kColliderExcludeAll);
	object->SetGeometryFlags(object->GetGeometryFlags() | (kGeometryShadowInhibit | kGeometryMarkingInhibit | kGeometryRenderEffectPass) );

	MaterialObject *material = new MaterialObject;
	material->AddAttribute( new DiffuseAttribute( K::green ) );
	m_objHealth.SetMaterialObject(0, material);
	material->Release();

	m_objHealth.Update();
	object->Build( &m_objHealth );

	AddSubnode( &m_objHealth );
	/*
	m_objBackground.SetQuadColor( K::black );
	AddSubnode( &m_objBackground );

	m_objHealth.SetQuadColor( K::red );
	AddSubnode( &m_objHealth );
	*/
}

void MiniHealthBar::Update( Point3D curPosition, float perc )
{
	/*
	long width = TheDisplayMgr->GetDisplayWidth();
	long height = TheDisplayMgr->GetDisplayHeight();

	Point3D topLeft = TheBTCursor->GetCameraRayBisectionWithHeight( Point(0, 0), 0.0F );
	Point3D botRight = TheBTCursor->GetCameraRayBisectionWithHeight( Point(width, height), 0.0F );

	Point3D offset( 0 - topLeft.x, 0 - botRight.y, 0.0F );

	if( curPosition.x > topLeft.x && curPosition.x < botRight.x && curPosition.y > botRight.y && curPosition.y < topLeft.y ) {

		int x = 5; 5;
	}

	topLeft += offset;
	botRight += offset;
	curPosition += offset;

	float percX = curPosition.x / botRight.x;
	float percY = curPosition.y / topLeft.y;

	if( percX > 1.0F || percX < 0.0F || percY > 1.0F || percY < 0.0F ) {

		//m_objHealth.Hide();
		//m_objBackground.Hide();
		return;
	}

	m_objHealth.Show();
	m_objBackground.Show();


	Point3D pos( percX * width, percY * height, 0.0F );

	SetElementPosition( pos );

	if( perc > 1.0F )
		perc = 1.0F;

	if( perc < 0.0F )
		perc = 0.0F;

	m_objHealth.SetElementSize( m_objBackground.GetElementWidth() * perc, m_objBackground.GetElementHeight() );
	*/
}

SelectionRing::SelectionRing( Point3D& p3dOffset, float fRadius ) : 
		m_objRing( Vector2D( fRadius - 0.2F, fRadius - 0.2F ), Vector2D( fRadius, fRadius ) ),
		m_p3dOffset( p3dOffset )
{
	AnnulusGeometryObject *object = m_objRing.GetObject();
	object->SetCollisionExclusionMask(kColliderExcludeAll);
	object->SetGeometryFlags(object->GetGeometryFlags() | (kGeometryShadowInhibit | kGeometryMarkingInhibit | kGeometryRenderEffectPass) );

	MaterialObject *material = new MaterialObject;
	material->AddAttribute( new DiffuseAttribute( K::green ) );
	m_objRing.SetMaterialObject(0, material);
	material->Release();

	m_objRing.Update();
	object->Build( &m_objRing );

	AddSubnode( &m_objRing );
}

void SelectionRing::UpdateHealth( float perHealth )
{
	DiffuseAttribute *diff = static_cast<DiffuseAttribute *>( m_objRing.GetMaterialObject(0)->FindAttribute( kAttributeDiffuse ) );
	if( diff )
	{
		if( perHealth > 1.0F )
			perHealth = 1.0F;
		if( perHealth < 0.0F )
			perHealth = 0.0F;

		float RED = 0.0F;
		float GREEN = 0.0F;

		if( perHealth >= 0.5F ) {

			GREEN = 1.0F;
			RED = (1 - perHealth) * 2.0F;

		} else {

			RED = 1.0F;
			GREEN = perHealth * 2.0F;
		}

		diff->SetDiffuseColor( ColorRGBA( RED, GREEN, 0.0F ) );
		m_objRing.InvalidateAmbientShaderData();
	}
}