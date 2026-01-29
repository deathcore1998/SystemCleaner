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
		catch ( const std::exception& ) {}

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

common::OptionalString utils::path::validate( const fs::path& path )
{
	if ( !fs::exists( path ) )
	{
		return "Path not found"; 
	}

	if ( path == path.root_path() )
	{
		return "Cannot select drive root";
	}

	if ( isWindowsSystemFolder( path ) )
	{
		return "Windows system folder";
	}

	if ( isWindowsProgramFiles( path ) )
	{
		return "Program Files folder";
	}

	if ( isProtectedByAttributes( path ) )
	{
		return "File/Folder has protected system attributes";
	}

	return std::nullopt;
}