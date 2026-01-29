#pragma once

#include <filesystem>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/constants.hpp"
#include "common/id_generator.hpp"

namespace common
{
	enum class CleanerState
	{
		IDLE,
		ANALYZING,
		ANALYSIS_DONE,
		CLEANING,
		CLEANING_DONE
	};

	enum class SummaryType
	{
		NONE,
		ANALYSIS,
		CLEANING,
	};

	struct CleanOption
	{
		bool enabled = true;
		std::string displayName;
		uint64_t id = IDGenerator::next();
	};

	struct PathAdditionResult
	{
		std::string errorMessage;
		common::CleanOption option;

		bool isSuccess() const
		{
			return errorMessage.empty();
		}

		static PathAdditionResult success( common::CleanOption opt )
		{
			return { "", std::move( opt ) };
		}

		static PathAdditionResult error( std::string msg )
		{
			return { std::move( msg ), {} };
		}
	};

	enum class ItemType
	{
		NONE,
		BROWSER,
		TEMP,
		SYSTEM,
		CUSTOM_PATH
	};

	struct CleaningItem
	{
		CleaningItem( std::string name, ItemType itemType = ItemType::NONE ) : 
			name( std::move( name ) ), itemType( itemType )
		{
		}

		bool isNeedClean() const noexcept
		{
			for ( const CleanOption& cleanOption : cleanOptions )
			{
				if ( cleanOption.enabled )
				{
					return true;
				}
			}
			return false;
		}

		std::string name;
		uint64_t textureID = 0;
		ItemType itemType;
		std::vector< CleanOption > cleanOptions;
	};

	struct CleanResult
	{
		std::string propertyName;
		std::string categoryName;
		uint64_t cleanedFiles = 0;
		uint64_t cleanedSize = 0;
		uint64_t textureID = 0;
	};

	struct Summary
	{
		SummaryType type = SummaryType::NONE;

		float totalTime = 0.f;
		uint64_t totalFiles = 0;
		uint64_t totalSize = 0;

		std::vector< CleanResult > results;

		void reset()
		{
			type = SummaryType::NONE;
			totalTime = 0.f;
			totalFiles = 0;
			totalSize = 0;
			results.clear();
		}
	};
}