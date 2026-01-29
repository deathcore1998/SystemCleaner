#pragma once

#include "common/types.hpp"

namespace utils::path
{
	[[nodiscard]] common::OptionalString validate( const fs::path& path );
}