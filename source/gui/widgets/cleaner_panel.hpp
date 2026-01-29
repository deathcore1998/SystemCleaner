#pragma once
#pragma execution_character_set("utf-8")

#include <memory>

#include "common/cleaner_info.hpp"
#include "common/types.hpp"

#include "core/system_cleaner.hpp"
#include "core/texture_manager.hpp"

namespace gui
{
	enum class ActiveContext : size_t
	{
		BROWSER,
		TEMP_AND_SYSTEM,
		CUSTOM
	};

	class CleanerPanel
	{
	public:
		CleanerPanel();

		void draw();

	private:
		void drawTabBar();
		void drawMain();

		void drawCleaningItems();
		void drawOptions( common::CleaningItem& cleaningItem );
		void drawCustomOptions( common::CleaningItem& cleaningItem );
		void drawCustomPathsMenu();
		void drawProgress();
		void drawResultCleaningOrAnalysis();

		void prepareResultsForDisplay();

		core::SystemCleaner m_systemCleaner;
		common::CleaningItems m_cleaningItems;
		size_t m_customIndex;
		common::Summary m_cleanSummary;

		ActiveContext m_activeContext = ActiveContext::TEMP_AND_SYSTEM;

		core::TextureManager m_textureManager;
	};
}