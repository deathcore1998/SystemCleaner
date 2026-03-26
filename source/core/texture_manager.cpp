#include "texture_manager.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "utils/filesystem.hpp"


core::TextureManager::TextureManager()
{
	loadAllIcons();
}

core::TextureManager::~TextureManager()
{
	clear();
}

void core::TextureManager::loadAllIcons()
{
	const utils::FileSystem& fs = utils::FileSystem::instance();

	char buffer[ MAX_PATH ];
	GetModuleFileNameA( NULL, buffer, MAX_PATH );
	const fs::path exeDir = fs::path( buffer ).parent_path();

	fs::path iconsDir;
	if ( fs::exists( exeDir / "icons" ) )
	{
		iconsDir = exeDir / "icons";
	}
	else
	{
		iconsDir = fs.getProjectSourceDir() / "source/icons";
	}


	if ( !fs::exists( iconsDir ) || !fs::is_directory( iconsDir ) )
	{
		return;
	}

	for ( const auto& icon : fs::directory_iterator( iconsDir ) )
	{
		const fs::path iconPath = icon.path();
		if ( !icon.is_regular_file() || iconPath.extension() != ".png" )
		{
			continue;
		}

		const std::string filename = iconPath.stem().string();
		if ( m_textures.contains( filename ) )
		{
			continue;
		}

		int width = 0;
		int height = 0;
		int channels = 0;
		unsigned char* data = stbi_load( iconPath.string().c_str(), &width, &height, &channels, 4 );

		if ( !data )
		{
			continue;
		}

		GLuint textureID;
		glGenTextures( 1, &textureID );
		glBindTexture( GL_TEXTURE_2D, textureID );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

		stbi_image_free( data );

		m_textures[ iconPath.stem().string() ] = textureID;
	}
}

unsigned int core::TextureManager::getTexture( const std::string& name )
{
	auto it = m_textures.find( name );
	if ( it == m_textures.end() )
	{
		return 0;
	}
	return it->second;
}

void core::TextureManager::clear()
{
	for ( auto& pair : m_textures )
	{
		glDeleteTextures( 1, &pair.second );
	}
	m_textures.clear();
}