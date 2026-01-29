#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace utils::path
{
	[[nodiscard]] bool validate( const fs::path& path );
}