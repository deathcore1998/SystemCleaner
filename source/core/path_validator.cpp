#include "path_validator.hpp"

#include <windows.h>

#include <array>

#include "utils/filesystem.hpp"

namespace
{
	constexpr std::array< std::string_view, 7 > SYSTEM_FOLDERS = 
	{
		"System32",
		"SysWOW64",
		"WinSxS",
		"assembly",
		"Microsoft.NET",
		"boot",
		"SystemResources"
	};

	constexpr std::array< std::string_view, 3 > PROGRAM_FOLDERS =
	{
		"Program Files",
		"Program Files (x86)",
		"ProgramData"
	};
	
	constexpr DWORD PROTECTED_ATTR = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;
	const size_t MAX_LENGTH = 260;

	bool isWindowsSystemFolder( const fs::path& path )
	{
		const fs::path windowsDir = utils::FileSystem::instance().getWindowsDir();
		if ( fs::equivalent( path, windowsDir ) )
		{
			return true;
		}

		try
		{
			const fs::path relativePath = fs::relative( path, windowsDir );
			if ( !relativePath.empty() && relativePath.string().rfind( "..", 0 ) != 0 )
			{
				return true;
			}
		}
		catch ( const std::exception& )
		{
		}

		for ( std::string_view systemFolder : SYSTEM_FOLDERS )
		{
			const fs::path sysPath = windowsDir / systemFolder;
			if ( fs::exists( sysPath ) && fs::equivalent( sysPath, path ) )
			{
				return true;
			}
		}

		return false;
	}

	bool isWindowsProgramFiles( const fs::path& path )
	{
		const fs::path diskPath = utils::FileSystem::instance().getWindowsDir().parent_path();
		for ( std::string_view programFolder : PROGRAM_FOLDERS )
		{
			const fs::path progPath = diskPath / programFolder;
			if ( fs::exists( progPath ) && fs::equivalent( diskPath, progPath ) )
			{
				return true;
			}

			try
			{
				const fs::path relative = fs::relative( path, progPath );
				if ( !relative.empty() && relative.string().rfind( "..", 0 ) != 0 )
				{
					return true;
				}

			}
			catch ( const std::exception& )
			{
				continue;
			}
		}

		return false;
	}

	bool isProtectedByAttributes( const fs::path& path )
	{
		const DWORD attrs = GetFileAttributesW( path.wstring().c_str() );
		if ( attrs == INVALID_FILE_ATTRIBUTES )
		{
			return true;
		}

		return ( attrs & PROTECTED_ATTR ) != 0;
	}
}

bool utils::path::validate( const fs::path& path )
{
	if ( !fs::exists( path ) || path.string().length() > MAX_LENGTH )
	{
		return false;
	}

	if ( path == path.root_path() )
	{
		return false;
	}

	if ( isWindowsSystemFolder( path ) || isWindowsProgramFiles( path ) )
	{
		return false;
	}

	if ( isProtectedByAttributes( path ) )
	{
		return false;
	}

	return true;
}