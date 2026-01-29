#pragma once

#include <imgui.h>

namespace utils
{
	inline void Tooltip( const char* text )
	{
		if ( ImGui::IsItemHovered() )
		{
			ImGui::SetTooltip( text );
		}
	}
}