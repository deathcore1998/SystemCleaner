#pragma once

#include <atomic>
#include <mutex>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/cleaner_info.hpp"
#include "common/types.hpp"

namespace core
{
	struct DirInfo
	{
		uint64_t dirSize = 0;
		uint64_t countFile = 0;
	};

	class SystemCleaner
	{
	public:
		~SystemCleaner();

		[[nodiscard]] common::Summary getSummary();

		void clear( const common::CleaningItems& cleanTargets );
		void analysis( const common::CleaningItems& cleanTargets );

		common::CleanerState getCurrentState();
		float getCurrentProgress();

		[[nodiscard]] common::CleaningItems collectCleaningItems();

		[[nodiscard]] common::PathAdditionResult addCustomPath( const fs::path& path );
		void removeCustomPath( uint64_t id );
		[[nodiscard]] common::OptionalString getFullPath( uint64_t id );
	private:
		void initBrowserData( common::CleaningItems& cleaningItems );
		void initSystemTempData( common::CleaningItems& cleaningItems );
		void initCustomPaths( common::CleaningItems& cleaningItems );

		void fini();

		[[nodiscard]] DirInfo processPath( const fs::path& pathDir, bool deleteFiles = false );

		void analysisTargets( const common::CleaningItems& cleaningItems );
		void analysisOptions( const common::CleaningItem& cleaningItem );
		void clearTargets( const common::CleaningItems& cleaningItems );
		void cleanOptions( const common::CleaningItem& cleaningItem );

		void accumulateResult( std::string itemName, std::string category, const core::DirInfo dirInfo );

		void resetData();

		std::atomic< uint64_t > m_cleanedFiles { 0 };
		std::atomic< float > m_progress { 0.f };
		std::atomic< size_t > m_countAnalysTasks { 0 };

		std::mutex m_summaryMutex;
		common::Summary m_summary;

		std::unordered_map< uint64_t, fs::path > m_cleanPathCache;
		std::unordered_map< uint64_t, fs::path > m_customPathCache;
		std::atomic < common::CleanerState > m_currentState = common::CleanerState::IDLE;
	};
}