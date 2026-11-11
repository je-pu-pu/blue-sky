#include "DebugConsole.h"

#include <blue_sky/ScriptManager.h>
#include <blue_sky/SceneManager.h>

#include <core/Logger.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <boost/algorithm/string.hpp>

namespace blue_sky::dev
{

DebugConsole::DebugConsole( ScriptManager& script_manager, const SceneManager& scene_manager, OpenScene open_scene )
	: script_manager_( script_manager )
	, scene_manager_( scene_manager )
	, open_scene_( std::move( open_scene ) )
	, user_command_( 1024, '\0' )
{

}

void DebugConsole::render()
{
	if ( ! is_open_ )
	{
		return;
	}

	render_console_window();
	render_scene_list_window();
}

/**
 * @brief コンソールウィンドウを描画する
 */
void DebugConsole::render_console_window()
{
	if ( ! ImGui::Begin( "Console", 0 ) )
	{
		ImGui::End();
		return;
	}

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if ( ImGui::BeginChild( "ScrollingRegion", ImVec2( 0, -footer_height_to_reserve ), 0, ImGuiWindowFlags_HorizontalScrollbar ) )
	{
		if ( ImGui::BeginPopupContextWindow() )
		{
			if ( ImGui::Selectable( "Clear" ) )
			{
				// clear log
			}

			ImGui::EndPopup();
		}

		for ( const auto& log: core::logger.get_log_list() )
		{
			auto has_color = false;

			switch ( log.get_type() )
			{
			case core::Logger::Log::Type::ERROR:
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.4f, 0.4f, 1.0f ) );
				has_color = true;
				break;
			case core::Logger::Log::Type::WARN:
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.7f, 0.1f, 1.0f ) );
				has_color = true;
				break;
			case core::Logger::Log::Type::INFO:
				break;
			case core::Logger::Log::Type::DEBUG:
				break;
			}

			ImGui::TextUnformatted( log.get_message().c_str() );

			if ( has_color )
			{
				ImGui::PopStyleColor();
			}
		}

		if ( scroll_to_bottom_ )
		{
			ImGui::SetScrollHereY( 1.f );
		}

		scroll_to_bottom_ = false;
	}

	ImGui::EndChild();

	ImGui::Separator();

	const auto input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

	ImGui::SetNextItemWidth( -FLT_MIN );

	bool reclaim_focus = false;

	const auto callback = [] ( ImGuiInputTextCallbackData* data ) -> int
	{
		auto console = static_cast< DebugConsole* >( data->UserData );
		string_t command;

		switch ( data->EventFlag )
		{
		case ImGuiInputTextFlags_CallbackCompletion:
			command = console->user_command_;

			console->script_manager_.auto_complete( command );

			if ( ! console->script_manager_.get_output().empty() )
			{
				core::logger.info( console->script_manager_.get_output() );
			}

			console->scroll_to_bottom_ = true;

			break;
		case ImGuiInputTextFlags_CallbackHistory:
			if ( data->EventKey == ImGuiKey_UpArrow )
			{
				command = console->script_manager_.get_prev_hisotry_command();
			}
			else if ( data->EventKey == ImGuiKey_DownArrow )
			{
				command = console->script_manager_.get_next_hisotry_command();
			}
		}

		if ( command != console->user_command_ )
		{
			data->DeleteChars( 0, data->BufTextLen );
			data->InsertChars( 0, command.c_str() );
		}

		return 0;
	};

	if ( ImGui::InputText( "##command", & user_command_, input_text_flags, callback, this ) )
	{
		boost::trim( user_command_ );

		if ( user_command_ != "" )
		{
			core::logger.info( user_command_ );

			try
			{
				script_manager_.exec( user_command_, true );
			}
			catch ( const ScriptError& e )
			{
				core::logger.error( e.what() );
			}

			user_command_ = "";
			scroll_to_bottom_ = true;
		}

		reclaim_focus = true;
	}

	ImGui::SetItemDefaultFocus();

	if ( reclaim_focus || ! ImGui::IsAnyItemActive() )
	{
		ImGui::SetKeyboardFocusHere( -1 );
	}

	ImGui::End();
}

/**
 * @brief シーン一覧ウィンドウを描画する
 */
void DebugConsole::render_scene_list_window()
{
	if ( ! ImGui::Begin( "SceneList" ) )
	{
		ImGui::End();
		return;
	}

	if ( ImGui::BeginListBox( "##SceneListBox", ImVec2( -FLT_MIN, 0 ) ) )
	{
		int n = 0;

		for ( const auto& scene: scene_manager_.get_scene_generator_map() )
		{
			ImGuiSelectableFlags flags = n == selected_scene_index_ ? ImGuiSelectableFlags_Highlight : 0;

			if ( ImGui::Selectable( scene.first.c_str(), n == selected_scene_index_, flags ) )
			{
				selected_scene_index_ = n;
			}

			if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( 0 ) && open_scene_ )
			{
				open_scene_( scene.first );
			}

			n++;
		}

		ImGui::EndListBox();
	}

	ImGui::End();
}

} // namespace blue_sky::dev
