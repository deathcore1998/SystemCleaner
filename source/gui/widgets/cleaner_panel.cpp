#include "cleaner_panel.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <string>

#include <imgui.h>

#include "common/constants.hpp"
#include "common/scoped_guards.hpp"

#include "utils/custom_widgets.hpp"
#include "utils/dialogs.hpp"

namespace
{
	constexpr float BUTTON_HEIGHT = 30.f;
	constexpr float VERTICAL_OFFSET = 20.f;
	constexpr float KILOBYTE = 1024.0f;
	constexpr float MEGABYTE = KILOBYTE * KILOBYTE;
	constexpr ImVec2 SMALL_ICON_SIZE = ImVec2( 16.f, 16.f );
	constexpr ImVec2 BIG_ICON_SIZE = ImVec2( 24.f, 24.f );

	constexpr ImU32 GREEN_COLOR = IM_COL32( 0, 200, 0, 255 );

	inline std::string separateString( std::string_view str )
	{
		std::string result( str );
		for ( int i = result.size() - 3; i > 0; i -= 3 )
		{
			result.insert( i, " " );
		}
		return result;
	}

	inline void rightAlignedText( const std::string& text )
	{
		const float regionAvail = ImGui::GetContentRegionAvail().x;
		const float textSize = ImGui::CalcTextSize( text.c_str() ).x;
		ImGui::SetCursorPosX( ImGui::GetCursorPosX() + regionAvail - textSize );
		ImGui::Text( text.c_str() );
	}
}

gui::CleanerPanel::CleanerPanel()
{
	m_cleaningItems = m_systemCleaner.collectCleaningItems();
	for ( common::CleaningItem& cleanTarget : m_cleaningItems )
	{
		cleanTarget.textureID = m_textureManager.getTexture( cleanTarget.name );
	}
	m_customIndex = m_cleaningItems.size() - 1;
}

void gui::CleanerPanel::draw()
{
	ImGui::StyleGuard styleGuard( ImGuiCol_ChildBg, IM_COL32( 100, 100, 100, 255 ) );
	
	ImGui::Child cleanerPanel( "Cleaner panel" );

	drawTabBar();

	constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_SizingFixedFit;
	const ImVec2 tableSize = ImGui::GetContentRegionAvail();

	if ( auto table = ImGui::Table( "CleanerTable", 2, tableFlags, tableSize ) )
	{
		constexpr ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn( "Settings", columnFlags, tableSize.x * 0.3f );
		ImGui::TableSetupColumn( "Main", columnFlags, tableSize.x * 0.7f );

		ImGui::TableNextColumn();
		{
			ImGui::Child options( "OptionsColumn" );

			drawCleaningItemsHeader();

			drawCleaningItems();
		}

		ImGui::TableNextColumn();
		drawMain();
	}
}

void gui::CleanerPanel::drawTabBar()
{
	if ( ImGui::BeginTabBar( "CleanerTabs" ) )
	{
		if ( ImGui::BeginTabItem( "Temp/System" ) )
		{
			m_activeContext = ActiveContext::TEMP_AND_SYSTEM;
			ImGui::EndTabItem();
		}

		if ( ImGui::BeginTabItem( "Browsers" ) )
		{
			m_activeContext = ActiveContext::BROWSER;
			ImGui::EndTabItem();
		}

		if ( ImGui::BeginTabItem( "Custom paths" ) )
		{
			m_activeContext = ActiveContext::CUSTOM;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void gui::CleanerPanel::drawMain()
{
	const float cursorPosX = ImGui::GetCursorPosX();
	const ImVec2 contentAvail = ImGui::GetContentRegionAvail();
	const ImVec2 buttonSize = ImVec2( 100.f, BUTTON_HEIGHT );
	const float buttonPosY = contentAvail.y - VERTICAL_OFFSET;

	const common::CleanerState currentSystemState = m_systemCleaner.getCurrentState();
	const bool isSystemNotIdle = currentSystemState != common::CleanerState::IDLE;
	if ( currentSystemState == common::CleanerState::ANALYSIS_DONE ||
		 currentSystemState == common::CleanerState::CLEANING_DONE )
	{
		prepareResultsForDisplay();
	}

	if ( m_cleanSummary.type != common::SummaryType::NONE || isSystemNotIdle )
	{
		drawProgress();
		drawResultCleaningOrAnalysis();
	}
	
	ImGui::DisabledGuard disabledGuard( isSystemNotIdle );
	ImGui::SetCursorPosY( buttonPosY );
	if ( ImGui::Button( "Analysis", buttonSize ) )
	{
		m_cleanSummary.reset();
		m_systemCleaner.analysis( m_cleaningItems );
	}

	ImGui::SameLine( contentAvail.x - buttonSize.x );
	if ( ImGui::Button( "Clear", buttonSize ) )
	{
		m_cleanSummary.reset();
		m_systemCleaner.clear( m_cleaningItems );
	}	
}

void gui::CleanerPanel::drawBulkCheckboxButtons()
{
	auto toggleAllOptions = [ this ]( bool enable )
	{
		for ( common::CleaningItem& cleanItem : m_cleaningItems )
		{
			if ( !isItemVisible( cleanItem ) )
			{
				continue;
			}

			for ( common::CleanOption& cleanOption : cleanItem.cleanOptions )
			{
				cleanOption.enabled = enable;
			}
		}
	};

	if ( ImGui::ImageButton( "Enable all visible items", m_textureManager.getTexture( "Enable All" ), BIG_ICON_SIZE ) )
	{
		toggleAllOptions( true );
	}
	utils::Tooltip( "Enable all visible items" );

	ImGui::SameLine();
	if ( ImGui::ImageButton( "Disable all visible items", m_textureManager.getTexture( "Disable All" ), BIG_ICON_SIZE ) )
	{
		toggleAllOptions( false );
	}
	utils::Tooltip( "Disable all visible items" );
}

void gui::CleanerPanel::drawCleaningItems()
{
	for ( common::CleaningItem& cleanItem : m_cleaningItems )
	{
		if ( !isItemVisible( cleanItem ) )
		{
			continue;
		}

		const bool isCustomItem = cleanItem.itemType == common::ItemType::CUSTOM_PATH;
		if ( isCustomItem )
		{
			drawCustomOptions( cleanItem );
		}
		else
		{
			drawOptions( cleanItem );
		}
	}
}

void gui::CleanerPanel::drawCustomPathsMenu()
{
	auto processingPath = [ & ]( common::OptionalPath& path )
	{
		if ( path.has_value() && !path.value().empty() )
		{
			common::PathAdditionResult result = m_systemCleaner.addCustomPath( path.value() );
			if ( result.isSuccess() )
			{
				m_cleaningItems[ m_customIndex ].cleanOptions.push_back( std::move( result.option ) );
			}
			else
			{
				utils::openMessageBox( "Warning", result.errorMessage, utils::ButtonFlag::BUTTON_OK, utils::BoxType::TYPE_WARNING );
			}
		}
	};

	if ( ImGui::ImageButton( "Custom file", m_textureManager.getTexture( "Add File" ), BIG_ICON_SIZE ) )
	{
		common::OptionalPath path = utils::openFileDialog();
		processingPath( path );
	}
	utils::Tooltip( "Add file path" );

	ImGui::SameLine();
	if ( ImGui::ImageButton( "Custom folder", m_textureManager.getTexture( "Add Folder" ), BIG_ICON_SIZE ) )
	{
		common::OptionalPath path = utils::openFolderDialog();
		processingPath( path );
	}
	utils::Tooltip( "Add folder path" );

	ImGui::SameLine();
	if ( ImGui::ImageButton( "Remove custom paths", m_textureManager.getTexture( "Remove" ), BIG_ICON_SIZE ) )
	{
		if ( !m_cleaningItems[ m_customIndex ].cleanOptions.empty() )
		{
			const utils::ButtonFlag flags = utils::ButtonFlag::BUTTON_YES | utils::ButtonFlag::BUTTON_NO;
			const utils::Result messageBoxResult = utils::openMessageBox( "Warning", "Do you really want to delete all user paths?", flags, utils::BoxType::TYPE_WARNING );
			if ( messageBoxResult == utils::Result::RESULT_YES )
			{
				auto& options = m_cleaningItems[ m_customIndex ].cleanOptions;
				options.erase( std::remove_if( options.begin(), options.end(),
				[&] ( const common::CleanOption& opt )
				{
					if ( opt.enabled )
					{
						m_systemCleaner.removeCustomPath( opt.id );
						return true;
					}
					return false;
				} ), options.end() );
			}
		}
	}
	utils::Tooltip( "Remove enabled custom paths" );
}

void gui::CleanerPanel::drawCleaningItemsHeader()
{
	drawBulkCheckboxButtons();

	if ( m_activeContext == ActiveContext::CUSTOM )
	{
		ImGui::SameLine();
		drawCustomPathsMenu();
	}
}

void gui::CleanerPanel::drawOptions( common::CleaningItem& cleaningItem )
{
	ImGui::IDGuard guard( cleaningItem.name );
	const unsigned int textureId = m_textureManager.getTexture( cleaningItem.name );
	if ( textureId != ImTextureID_Invalid )
	{
		ImGui::Image( textureId, SMALL_ICON_SIZE );
		ImGui::SameLine();
	}

	const float checkboxOffset = ImGui::GetCursorPosX();
	ImGui::AlignTextToFramePadding();
	ImGui::Text( cleaningItem.name.c_str() );
	{
		ImGui::IndentGuard indent( checkboxOffset );
		for ( common::CleanOption& cleanOption : cleaningItem.cleanOptions )
		{
			ImGui::Checkbox( cleanOption.displayName.c_str(), &cleanOption.enabled );
		}
	}
}

void gui::CleanerPanel::drawCustomOptions( common::CleaningItem& cleaningItem )
{
	std::vector< common::CleanOption >& cleanOptions = cleaningItem.cleanOptions;
	for ( auto& [ enabled, displayName, id ] : cleanOptions )
	{
		ImGui::IDGuard guard( id );

		ImGui::Checkbox( displayName.c_str(), &enabled);
		const common::OptionalString fullPath = m_systemCleaner.getFullPath( id );
		if ( fullPath.has_value() && !fullPath.value().empty() )
		{
			utils::Tooltip( fullPath.value().c_str() );
		}
	}

	if ( ImGui::IsKeyPressed( ImGuiKey_Delete ) )
	{
		cleaningItem.cleanOptions.erase( std::remove_if( cleanOptions.begin(), cleanOptions.end(), 
		[ & ] ( const common::CleanOption& opt )
		{
			return opt.enabled;
		} ), cleanOptions.end() );
	}
}

void gui::CleanerPanel::drawProgress()
{
	ImGui::StyleGuard progressStyle( ImGuiCol_PlotHistogram, GREEN_COLOR );
	const float length = ImGui::GetContentRegionAvail().x;
	ImGui::ProgressBar( m_systemCleaner.getCurrentProgress(), ImVec2( length, 20.f ) );
}

void gui::CleanerPanel::drawResultCleaningOrAnalysis()
{
	{
		const bool isSummaryAnalysis = m_cleanSummary.type == common::SummaryType::ANALYSIS;

		ImGui::IndentGuard indent( 10.f );
		ImGui::Text( isSummaryAnalysis ? "Analysis completed" : "Cleaning is complete" );
		ImGui::SameLine();
		ImGui::Text( "(%.3fs)", m_cleanSummary.totalTime );

		ImGui::Text( isSummaryAnalysis ? "Will be cleared approximately:" : "Cleared:" );
		ImGui::SameLine();
		ImGui::Text( "%.2f MB", static_cast< float >( m_cleanSummary.totalSize ) / MEGABYTE );
	}

	ImGui::Spacing();
	ImGui::Separator();

	const ImVec2 contentAvail = ImGui::GetContentRegionAvail();
	const float childHieght = contentAvail.y - BUTTON_HEIGHT - VERTICAL_OFFSET * 2;
	ImGui::Child result( "ResultCleaningOrAnalysis", ImVec2( 0, childHieght ) );
	
	constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
	if ( auto table = ImGui::Table( "CleanSummaryTable", 3, flags ) )
	{
		constexpr ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn( "##name", columnFlags, contentAvail.x * 0.5 );
		ImGui::TableSetupColumn( "##cleanedSize", columnFlags, contentAvail.x * 0.30 );
		ImGui::TableSetupColumn( "##cleanedFiles", columnFlags, contentAvail.x * 0.20 );

		for ( const auto& result : m_cleanSummary.results )
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			
			if ( result.textureID != ImTextureID_Invalid )
			{
				ImGui::Image( result.textureID, SMALL_ICON_SIZE );
				ImGui::SameLine();
			}
			ImGui::Text( "%s - %s", result.propertyName.c_str(), result.categoryName.c_str() );

			ImGui::TableNextColumn();
			const std::string cleanedSize = std::to_string( static_cast< int >( std::ceil( result.cleanedSize / KILOBYTE ) ) );
			rightAlignedText( separateString( cleanedSize ) + " KB" );

			ImGui::TableNextColumn();
			const std::string cleanedFiles = separateString( std::to_string( result.cleanedFiles ) );
			rightAlignedText( cleanedFiles );
		}
	}
}

void gui::CleanerPanel::prepareResultsForDisplay()
{
	m_cleanSummary = m_systemCleaner.getSummary();

	// sort results
	std::vector< common::CleanResult >& results = m_cleanSummary.results;
	std::sort( results.begin(), results.end(),
	[] ( const common::CleanResult& r1, const common::CleanResult& r2 )
	{
		return r1.propertyName < r2.propertyName;
	} );

	// assign icons
	for ( auto& result : m_cleanSummary.results )
	{
		result.textureID = m_textureManager.getTexture( result.propertyName );
	}
}

bool gui::CleanerPanel::isItemVisible( const common::CleaningItem& item ) const
{
	switch ( m_activeContext )
	{
		case ActiveContext::BROWSER:
			return item.itemType == common::ItemType::BROWSER;
		case ActiveContext::TEMP_AND_SYSTEM:
			return item.itemType == common::ItemType::TEMP || item.itemType == common::ItemType::SYSTEM;
		case ActiveContext::CUSTOM:
			return item.itemType == common::ItemType::CUSTOM_PATH;
	}
	return false;
}