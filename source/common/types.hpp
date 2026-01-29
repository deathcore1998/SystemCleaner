#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "common/cleaner_info.hpp"

namespace fs = std::filesystem;

namespace common
{
	using OptionalPath = std::optional< fs::path >;
	using OptionalString = std::optional< std::string >;

	using CleaningItems = std::vector< common::CleaningItem >;
}