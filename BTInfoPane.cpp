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

#include "BTInfoPane.h"
#include "BTHUD.h"
#include "BTCommander.h"
#include "BTUnit.h"

using namespace C4;

InfoPaneMgr *C4::TheInfoPaneMgr = nullptr;

enum
{
	kElementAbilityIcon = 'abil'
};

/* -------------- Ability Bar -------------- */

// TODO: Take in Default background?
AbilityBar::Icon::Icon( float fDefaultSize, float fMaxSize ) : 
		ResizingImage( "", fDefaultSize, fMaxSize, 1.0F ),
		m_lAbilityNumber( -1 ),
		m_bUnlocked( false ),
		m_bClickable( false )
{
	m_objTimer.SetElementSize( fDefaultSize, fDefaultSize );
	m_objTimer.SetQuadColor( ColorRGBA( 0.0F, 0.0F, 0.0F, 0.6F ) );

	AddSubnode( &m_objTimer );

	Unlock(false);
}

AbilityBar::Icon::~Icon( )
{

}


void AbilityBar::Icon::SetAbility( Ability *pAbility, TableID stringTableID, Type ulEntityID, Type ulAbilityID, long lAbilityNumber, bool bClickable )
{
	const StringTable *table = TheStringTableMgr->GetStringTable(  stringTableID );
	if( table ) {

		const char *icon = table->GetString( StringID( ulEntityID, ulAbilityID, 'ICON' ) );
		m_objImage.SetTexture( icon );

		/* 
		TextElement *mmm = new TextElement();
		mmm->SetText( icon );
		mmm->SetFont( AutoReleaseFont("font/Gui") );
		mmm->SetTextColor( K::red );
		mmm->SetElementSize( 30.0F, 10.0F );



		SetTipElement( mmm );
		*/
	}

	m_pAbility = pAbility;
	m_lAbilityNumber = lAbilityNumber;
	m_bClickable = bClickable;
}

void AbilityBar::Icon::Unlock( bool bUnlock )
{
	m_bUnlocked = bUnlock;

	if( m_bUnlocked ) {

		m_objTimer.Hide();

	} else {

		m_objTimer.SetElementSize( m_objImage.GetElementWidth(), m_objImage.GetElementHeight() );
		m_objTimer.Show();
	}
}

PartCode AbilityBar::Icon::TestPickPoint(const Point3D &p) const
{
	return (kElementPartButton);
}

void AbilityBar::Icon::HandleMouseEvent(EventType eventType, PartCode code, const Point3D& p)
{
	if( m_objTimer.Visible() == true ) {

		return;
	}

	ResizingImage::HandleMouseEvent( eventType, code, p );

	if( eventType == kEventMouseDown ) {

		if( m_bClickable && m_pAbility )
		{
			BTEntityController *entity = m_pAbility->GetOwner();
			if( entity ) 
			{
				TheMessageMgr->SendMessage( kPlayerServer, EntityAbilityMessage( entity->GetControllerIndex(), m_lAbilityNumber ));
			}
		}
	}
}

void AbilityBar::Icon::InterfaceTask( void )
{
	if( m_bUnlocked == false ) {

		return;
	}

	ResizingImage::InterfaceTask();

	if( m_pAbility )
	{
		AbilityState state = m_pAbility->GetState();
		if( state == kAbilityStateCooldown || state == kAbilityStateGlobal )
		{
			float deltaTime = (float)( TheTimeMgr->GetAbsoluteTime() - m_pAbility->GetLastUsed() );
			float cooldown = state == kAbilityStateCooldown ? ((float) m_pAbility->GetCooldown()) : GLOBAL_COOLDOWN;
		
			if( deltaTime > cooldown )
			{
				m_objTimer.Hide();
				return;
			}

			float perc = deltaTime / cooldown;


			m_objTimer.SetElementSize( m_objImage.GetElementWidth(), m_objImage.GetElementHeight() * (1 - perc) );

			m_objTimer.Show();
		}
		else
		{
			m_objTimer.Hide();
		}
	}
}

AbilityBar::AbilityBar( long numAbilities, float fDefaultSize, float fMaxSize ) :
		m_arrAbilityIcons( numAbilities )
{
	float offset = 0.0F;

	for( long x = 0; x < numAbilities; x++ ) {

		Icon *icon = new Icon( fDefaultSize, fMaxSize );

		icon->SetElementPosition( Point3D( offset, 0.0F, 0.0F )	);
		offset += fDefaultSize + 5.0F;

		m_arrAbilityIcons.AddElement( icon );
		AddSubnode( icon );
	}

	SetElementSize( offset, fMaxSize );
}

AbilityBar::~AbilityBar()
{

}

void AbilityBar::SetAbility( long lSlotNumber, Ability *pAbility, TableID stringTableID, Type ulEntityID, Type ulAbilityID, long lAbilityNumber, bool bClickable )
{
	if( lSlotNumber < m_arrAbilityIcons.GetElementCount() && lSlotNumber >= 0 )
		m_arrAbilityIcons[lSlotNumber]->SetAbility( pAbility, stringTableID, ulEntityID, ulAbilityID, lAbilityNumber, bClickable );
}

void AbilityBar::UnlockAbility( long lSlotNumber, bool bUnlock )
{
	if( lSlotNumber < m_arrAbilityIcons.GetElementCount() && lSlotNumber >= 0 )
		m_arrAbilityIcons[lSlotNumber]->Unlock( bUnlock );
}

/* -------------- Rank Bar -------------- */

RankBar::Icon::Icon( const char *strDefaultImage, const char *strUnlockedImage, float fSize ) :
		m_objDefaultImage( strDefaultImage, fSize, fSize + 40.0F, 500.0F ),
		m_objUnlockedImage( strUnlockedImage, fSize, fSize + 40.0F, 500.0F ),
		m_bUnlocked( false ),
		m_bUnlocking( false ),
		m_ulStartTime( -1 )
{
	AddSubnode( &m_objDefaultImage );
}

void RankBar::Icon::Unlock( bool bUnlock )
{
	if( bUnlock ) {

		if( m_bUnlocked == false ) {

			// for the effect
			m_bUnlocking = true;
		}
	} else {

		m_bUnlocked = false;

		AddSubnode( &m_objDefaultImage );
		RemoveSubnode( &m_objUnlockedImage );
	}
}

void RankBar::Icon::InterfaceTask( void )
{
	if( m_bUnlocking == false ) {

		return;
	}

	// the effect is unlocked and the commander info pane is visible, start displaying the effect
	if( m_bUnlocked == false ) {

		m_bUnlocked = true;
		m_ulStartTime = TheTimeMgr->GetAbsoluteTime();

		m_objDefaultImage.Hilite();
		m_objUnlockedImage.Hilite();

		// TODO: doesn't exactly work, the alpha channel at mid-way is 50/50 so the other stars will seap through :/
		AddSubnode( &m_objDefaultImage ); // re-attach this image to the end of the list so it is rendered above the other stars
		AddSubnode( &m_objUnlockedImage );
	}

	unsigned long curTime = TheTimeMgr->GetAbsoluteTime();

	float percentage = (curTime - m_ulStartTime) / 1000.0F;

	if( percentage > 1.0F ) {

		m_bUnlocking = false;

		percentage = 1.0F;

		m_objDefaultImage.Unhilite();
		m_objUnlockedImage.Unhilite();

		RemoveSubnode( &m_objDefaultImage );
	}

	m_objDefaultImage.GetImage().SetQuadAlpha( 1 - percentage );
	m_objUnlockedImage.GetImage().SetQuadAlpha( percentage );
}

RankBar::RankBar( long numRanks ) :
		m_lCommanderKills( 0 )
{
	Point3D posOffset;

	for( long x = 0; x < numRanks; x++ ) {

		Icon *icon = new Icon( "texture/ui/field_mode/rank_default", "texture/ui/field_mode/rank_unlocked", 25.0F );
		icon->SetElementPosition( posOffset );

		m_arrRankIcons.AddElement( icon );
		AddSubnode( icon );

		posOffset.x += 25.0F + 5.0F;
	}

	SetElementSize( posOffset.x - 30.0F, 25.0F );
}

RankBar::~RankBar()
{

}

void RankBar::AddToKillCount( long lDeltaKills )
{
	m_lCommanderKills += lDeltaKills;
}

void RankBar::UnlockRank( long lSlotNumber, bool bUnlock )
{
	if( lSlotNumber < m_arrRankIcons.GetElementCount() && lSlotNumber >= 0 )
		m_arrRankIcons[lSlotNumber]->Unlock( bUnlock );
}


/* -------------- Unit Stats Box -------------- */

StatsBox::StatsBox() : Interface(),
		m_lControllerIndex(-1),
		m_pStatsController(nullptr)
{
	Point3D pos( 0.0F, 0.0F, 0.0F );
	AutoReleaseFont font("font/Gui");

	m_textStrength.SetTextColor(K::white);
	m_textStrength.SetTextScale(1.2F);
	m_textStrength.SetFont(font);
	m_textStrength.SetElementSize(100.0F, 30.0F);

	AddSubnode(&m_textStrength);
	pos.y += 20.0F;

	m_textArmor.SetTextColor(K::white);
	m_textArmor.SetTextScale(1.2F);
	m_textArmor.SetFont(font);
	m_textArmor.SetElementSize(100.0F, 30.0F);
	m_textArmor.SetElementPosition( pos );

	AddSubnode(&m_textArmor);
	pos.y += 20.0F;

	m_textMoveSpeed.SetTextColor(K::white);
	m_textMoveSpeed.SetTextScale(1.2F);
	m_textMoveSpeed.SetFont(font);
	m_textMoveSpeed.SetElementSize(100.0F, 30.0F);
	m_textMoveSpeed.SetElementPosition( pos );

	AddSubnode(&m_textMoveSpeed);
}

void StatsBox::SetCurrentUnit(long lControllerIndex)
{
	m_lControllerIndex = lControllerIndex;
	m_pStatsController = nullptr;

	m_textStrength.SetText("");
	m_textArmor.SetText("");
	m_textMoveSpeed.SetText("");

	Controller *controller = TheWorldMgr->GetWorld()->GetController(lControllerIndex);

	if( controller && controller->GetBaseControllerType() == kControllerEntity ) {

		m_pStatsController = static_cast<BTEntityController *>(controller)->GetStats();
	}
}

void StatsBox::FormatStat(char *strIdentifier, float fBaseStat, float fEffectOnStat, String<> &strText)
{
	strText += strIdentifier;
	strText += ": ";
	strText += Text::IntegerToString( (long)fBaseStat );

	if( fEffectOnStat > 0 ) {
		
		strText += " (+ ";
		strText += Text::IntegerToString( (long)fEffectOnStat );
		strText += ")";

	} else if( fEffectOnStat ) {

		strText += " (- ";
		strText += Text::IntegerToString( (long)fEffectOnStat );
		strText += ")";
	}

	strText += "\n";

	//if (history) textElement.SetText((String<>(history) += "\n[INIT]") += text);
	//else textElement.SetText(text);

	//textElement.SplitLines();
}

void StatsBox::InterfaceTask(void)
{
	if( m_pStatsController == nullptr ) {

		m_textStrength.SetText("");
		m_textArmor.SetText("");
		m_textMoveSpeed.SetText("");
		return;
	}

	String<> stat;
	FormatStat( "Strength", m_pStatsController->GetBaseStrength(), m_pStatsController->GetEffectOnStrength(), stat );
	m_textStrength.SetText( stat );
	stat = "";

	FormatStat( "Armor", m_pStatsController->GetBaseArmor(), m_pStatsController->GetEffectOnArmor(), stat );
	m_textArmor.SetText( stat );
	stat = "";

	FormatStat( "Move Speed", m_pStatsController->GetMovementSpeed(), m_pStatsController->GetEffectOnMovementSpeed(), stat );
	m_textMoveSpeed.SetText( stat );
}

/* -------------- Info Pane -------------- */

InfoPane::InfoPane() : m_lControllerIndex( -2 )
{

}

InfoPane::~InfoPane()
{

}

void InfoPane::SetEntity( long lControllerIndex )
{
	m_lControllerIndex = lControllerIndex;
}

void InfoPane::HideIfSelected( long lControllerIndex )
{
	if( m_lControllerIndex == lControllerIndex )
	{
		if( TheInfoPaneMgr )
			TheInfoPaneMgr->ShowInfoPane( kInfoPaneNone );
	}
}

/* -------------- Commander Info Pane -------------- */

CommanderInfoPane::CommanderInfoPane( long numAbilities, long numRanks )
{
	m_pRankBar = new RankBar( numRanks );
	AddSubnode( m_pRankBar );

	m_objCommanderIcon.SetElementSize( 64.0F, 64.0F );
	AddSubnode( &m_objCommanderIcon );

	AddSubnode( &m_objStats );

	m_pHealthBar = new HealthBar("texture/ui/field_mode/health_bar", "texture/ui/field_mode/health");
	AddSubnode( m_pHealthBar );

	m_pAbilityBar = new AbilityBar( numAbilities, 40.0F, 50.0F );
	AddSubnode( m_pAbilityBar );
}


CommanderInfoPane::~CommanderInfoPane()
{
	if( m_pHealthBar )
		delete m_pHealthBar;

	if( m_pRankBar )
		delete m_pRankBar;

	if( m_pAbilityBar )
		delete m_pAbilityBar;
}

void CommanderInfoPane::SetCommander( CommanderController *commander )
{
	if( commander ) {

		SetEntity( commander->GetControllerIndex() );

		const StringTable *table = TheStringTableMgr->GetStringTable( kStringTableCommanders );
		if( table ) {

			Type stringID = commander->GetStringID();

			const char *icon = table->GetString( StringID( stringID, 'ICON' ) );

			m_objCommanderIcon.SetTexture( icon );
			m_objStats.SetCurrentUnit( commander->GetControllerIndex() );
		}

		commander->InitializeCommanderInfoPane( *this );

	} else {

		m_objCommanderIcon.SetTexture( "" );
		m_objStats.SetCurrentUnit( -1 );
		m_pHealthBar->SetStatsController( nullptr );
		SetEntity( -2 );
	}
}

void CommanderInfoPane::UpdateDisplayPosition( void )
{
	m_pRankBar->SetElementPosition( Point3D( 0.0F, 0.0F, 0.0F ) );

	m_objCommanderIcon.SetElementPosition( Point3D( 5.0F , 30.0F, 0.0F ) );

	m_objStats.SetElementPosition( Point3D( 15.0F + m_objCommanderIcon.GetElementWidth(), 45.0F, 0.0F ) );

	m_pHealthBar->SetElementPosition( Point3D( GetElementWidth() - m_pHealthBar->GetElementWidth() - 15.0F, GetElementHeight() - 65.0F - m_pHealthBar->GetElementHeight(), 0.0F ) );

	m_pAbilityBar->SetElementPosition( Point3D( GetElementWidth() - m_pAbilityBar->GetElementWidth() - 30.0F, GetElementHeight() - 55.0F, 0.0F ) );
}

/* -------------- Unit Info Pane -------------- */

UnitInfoPane::UnitInfoPane( long numAbilities )
{
	m_objUnitIcon.SetElementSize( 64.0F, 64.0F );
	AddSubnode( &m_objUnitIcon );

	AddSubnode( &m_objStats );

	m_pHealthBar = new HealthBar("texture/ui/field_mode/health_bar", "texture/ui/field_mode/health");
	AddSubnode( m_pHealthBar );

	m_pAbilityBar = new AbilityBar( numAbilities, 40.0F, 50.0F );
	AddSubnode( m_pAbilityBar );
}


UnitInfoPane::~UnitInfoPane()
{
	if( m_pHealthBar )
		delete m_pHealthBar;

	if( m_pAbilityBar )
		delete m_pAbilityBar;
}

void UnitInfoPane::SetUnit( UnitController *unit )
{
	if( unit ) {

		SetEntity( unit->GetControllerIndex() );

		const StringTable *table = TheStringTableMgr->GetStringTable( kStringTableUnits );
		if( table ) {

			Type stringID = unit->GetStringID();

			const char *icon = table->GetString( StringID( stringID, 'ICON' ) );

			m_objUnitIcon.SetTexture( icon );
			m_objStats.SetCurrentUnit( unit->GetControllerIndex() );
		}

		unit->InitializeUnitInfoPane( *this );

	} else {

		m_objUnitIcon.SetTexture( "" );
		m_objStats.SetCurrentUnit( -1 );
		m_pHealthBar->SetStatsController( nullptr );
		SetEntity( -2 );
	}
}

void UnitInfoPane::UpdateDisplayPosition( void )
{
	m_objUnitIcon.SetElementPosition( Point3D( 5.0F , 30.0F, 0.0F ) );

	m_objStats.SetElementPosition( Point3D( 15.0F + m_objUnitIcon.GetElementWidth(), 45.0F, 0.0F ) );

	m_pHealthBar->SetElementPosition( Point3D( GetElementWidth() - m_pHealthBar->GetElementWidth() - 15.0F, GetElementHeight() - 65.0F - m_pHealthBar->GetElementHeight(), 0.0F ) );

	m_pAbilityBar->SetElementPosition( Point3D( GetElementWidth() - m_pAbilityBar->GetElementWidth() - 70.0F, GetElementHeight() - 55.0F, 0.0F ) );
}

/* -------------- Base Info Pane -------------- */

BaseInfoPane::BaseInfoPane( )
{
	m_objBaseIcon.SetElementSize( 64.0F, 64.0F );
	AddSubnode( &m_objBaseIcon );

	m_objBaseName.SetFont( AutoReleaseFont("font/Gui") );
	m_objBaseName.SetTextColor( K::white );
	m_objBaseName.SetTextScale(1.2F);
	AddSubnode( &m_objBaseName );

	m_pHealthBar = new HealthBar("texture/ui/field_mode/health_bar", "texture/ui/field_mode/health");
	AddSubnode( m_pHealthBar );
}


BaseInfoPane::~BaseInfoPane()
{
	if( m_pHealthBar )
		delete m_pHealthBar;
}

void BaseInfoPane::SetBase( BaseController *base )
{
	if( base ) {

		SetEntity( base->GetControllerIndex() );

		const StringTable *table = TheStringTableMgr->GetStringTable( kStringTableUnits );
		if( table ) {

			Type stringID = base->GetUnitType();

			m_objBaseIcon.SetTexture( table->GetString( StringID( stringID, 'BASE' ) ) );

			String<> name = table->GetString( StringID( stringID ) );
			name += " Base";
			m_objBaseName.SetText( name );
		}

		m_pHealthBar->SetStatsController( base->GetStats() );

	} else {

		m_objBaseIcon.SetTexture( "" );
		m_objBaseIcon.SetTexture( "" );
		m_pHealthBar->SetStatsController( nullptr );
		SetEntity( -2 );
	}
}

void BaseInfoPane::UpdateDisplayPosition( void )
{
	m_objBaseIcon.SetElementPosition( Point3D( 5.0F , 20.0F, 0.0F ) );

	m_objBaseName.SetElementPosition( Point3D( 110.0F, 20.0F, 0.0F ) );

	m_pHealthBar->SetElementPosition( Point3D( (GetElementWidth() / 2.0F) - (m_pHealthBar->GetElementWidth() / 2.0F), 20.0F, 0.0F ) );
}

/* -------------- Building Info Pane -------------- */

BuildingInfoPane::BuildingInfoPane( )
{
	m_objBuildingIcon.SetElementSize( 64.0F, 64.0F );
	AddSubnode( &m_objBuildingIcon );

	m_objBuildingName.SetFont( AutoReleaseFont("font/Gui") );
	m_objBuildingName.SetTextColor( K::white );
	m_objBuildingName.SetTextScale(1.2F);
	AddSubnode( &m_objBuildingName );

	m_pHealthBar = new HealthBar("texture/ui/field_mode/health_bar", "texture/ui/field_mode/health");
	AddSubnode( m_pHealthBar );
}


BuildingInfoPane::~BuildingInfoPane()
{
	if( m_pHealthBar )
		delete m_pHealthBar;
}

void BuildingInfoPane::SetBuilding( BuildingController *building )
{
	if( building ) {

		SetEntity( building->GetControllerIndex() );

		const StringTable *table = TheStringTableMgr->GetStringTable( kStringTableBuildings );
		if( table ) {

			Type stringID = building->GetStringID();

			m_objBuildingIcon.SetTexture( table->GetString( StringID( stringID, 'ICON' ) ) );
			m_objBuildingName.SetText( table->GetString( StringID( stringID ) ) );
		}

		m_pHealthBar->SetStatsController( building->GetStats() );

	} else {

		m_objBuildingIcon.SetTexture( "" );
		m_objBuildingName.SetText( "" );
		m_pHealthBar->SetStatsController( nullptr );
		SetEntity( -2 );
	}
}

void BuildingInfoPane::UpdateDisplayPosition( void )
{
	m_objBuildingIcon.SetElementPosition( Point3D( 5.0F , 20.0F, 0.0F ) );

	m_objBuildingName.SetElementPosition( Point3D( 110.0F, 20.0F, 0.0F ) );

	m_pHealthBar->SetElementPosition( Point3D( (GetElementWidth() / 2.0F) - (m_pHealthBar->GetElementWidth() / 2.0F), 20.0F, 0.0F ) );
}

/* -------------- Info Pane Manager -------------- */


InfoPaneMgr::InfoPaneMgr() : Singleton<InfoPaneMgr>( TheInfoPaneMgr ),
		m_typeCurrentPane( kInfoPaneNone )
{
	m_pUnitInfoPane = new UnitInfoPane( 1 ); // TODO: 1
	m_pCommanderInfoPane = new CommanderInfoPane( 3,3 ); // TODO: 3,3
	m_pBaseInfoPane = new BaseInfoPane();
	m_pBuildingInfoPane = new BuildingInfoPane();
}

void InfoPaneMgr::New( void )
{
	if( TheInfoPaneMgr == nullptr )
	{
		new InfoPaneMgr();
	}
}

InfoPaneMgr::~InfoPaneMgr( void )
{
	if( m_pUnitInfoPane ) delete m_pUnitInfoPane;
	if( m_pCommanderInfoPane ) delete m_pCommanderInfoPane;
	if( m_pBaseInfoPane ) delete m_pBaseInfoPane;
	if( m_pBuildingInfoPane ) delete m_pBuildingInfoPane;
}

InfoPane& InfoPaneMgr::GetInfoPane( InfoPaneType type )
{
	switch( type )
	{
		case kInfoPaneUnit:

			return *m_pUnitInfoPane;

		case kInfoPaneCommander:
		
			return *m_pCommanderInfoPane;

		case kInfoPaneBase:

			return *m_pBaseInfoPane;

		case kInfoPaneBuilding:

			return *m_pBuildingInfoPane;

		default:

			return *m_pBuildingInfoPane;
	}
}

void InfoPaneMgr::ShowInfoPane( InfoPaneType type )
{
	m_typeCurrentPane = type;

	Pane& infoPane = TheBTHUD->GetPaneInfo();

	switch( type )
	{
		case kInfoPaneUnit:

			infoPane.PurgePane();
			infoPane.InsertElement( *m_pUnitInfoPane );
			break;

		case kInfoPaneCommander:
		
			infoPane.PurgePane();
			infoPane.InsertElement( *m_pCommanderInfoPane );
			break;

		case kInfoPaneBase:

			infoPane.PurgePane();
			infoPane.InsertElement( *m_pBaseInfoPane );
			break;


		case kInfoPaneBuilding:

			infoPane.PurgePane();
			infoPane.InsertElement( *m_pBuildingInfoPane );
			break;

		case kInfoPaneNone:

			infoPane.PurgePane();
			break;
	}
}

InfoPane& InfoPaneMgr::GetCurrentPane( void )
{
	return GetInfoPane( m_typeCurrentPane );
}

InfoPaneType InfoPaneMgr::CurrentPane( void )
{
	return m_typeCurrentPane;
}


