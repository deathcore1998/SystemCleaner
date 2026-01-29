#include <imgui.h>

#include <string>

namespace ImGui
{
	struct StyleGuard
	{
		StyleGuard( ImGuiCol idx, ImU32 col )
		{
			ImGui::PushStyleColor( idx, col );
		}

		~StyleGuard()
		{
			ImGui::PopStyleColor();
		}
	};

	struct IDGuard
	{
		IDGuard( std::string id )
		{
			ImGui::PushID( id.c_str() );
		}

		IDGuard( uint64_t id )
		{
			ImGui::PushID( id );
		}

		~IDGuard()
		{
			ImGui::PopID();
		}
	};

	struct DisabledGuard
	{
		DisabledGuard( bool disable )
		{
			ImGui::BeginDisabled( disable );
		}

		~DisabledGuard()
		{
			ImGui::EndDisabled();
		}
	};

	struct IndentGuard
	{
		IndentGuard( float indent ) : m_indent( indent )
		{
			ImGui::Indent( m_indent );
		}

		~IndentGuard()
		{
			ImGui::Unindent( m_indent );
		}
	private:
		float m_indent;
	};

	class Table
	{
	public:
		Table( std::string_view strId, int columns, ImGuiTableFlags flags = 0, const ImVec2& size = ImVec2( 0.0f, 0.0f ) )
		{
			m_open = ImGui::BeginTable( strId.data(), columns, flags, size );
		}

		~Table()
		{
			if ( m_open )
			{
				ImGui::EndTable();
			}	
		}

		explicit operator bool() const
		{
			return m_open;
		}

	private:
		bool m_open = true;
	};

	class Child
	{
	public:
		Child( std::string_view strId, const ImVec2& size = ImVec2( 0.0f, 0.0f ), ImGuiChildFlags childFlags = 0 )
		{
			m_open = ImGui::BeginChild( strId.data(), size, childFlags );
		}

		~Child()
		{
			if ( m_open )
			{
				ImGui::EndChild();
			}
		}

		explicit operator bool()const
		{
			return m_open;
		}
	private:
		bool m_open = true;
	};
}