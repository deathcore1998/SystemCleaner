#pragma once

#include "common/types.hpp"

namespace fs = std::filesystem;

namespace utils
{
    enum class DialogType
    {
        FILE,
        FOLDER,
    };

	// blocking main thread
    common::OptionalPath openSelectionDialog( std::string_view title, DialogType type );

    common::OptionalPath openFileDialog();
    common::OptionalPath openFolderDialog();

    enum ButtonFlag : uint32_t
    {
        BUTTON_OK = 0x0001,
        BUTTON_CANCEL = 0x0002,
        BUTTON_YES = 0x0004,
        BUTTON_NO = 0x0008,
    };

    constexpr ButtonFlag operator|( ButtonFlag a, ButtonFlag b )
    {
        return static_cast< ButtonFlag >( static_cast< uint32_t >( a ) | static_cast< uint32_t >( b ) );
    }

    enum BoxType : uint32_t
    {
        TYPE_INFO = 0,
        TYPE_WARNING = 1,
        TYPE_ERROR = 2,
        TYPE_QUESTION = 3
    };

    enum Result : int32_t
    {
        RESULT_NONE = 0,
        RESULT_OK = 1,
        RESULT_CANCEL = 2,
        RESULT_YES = 3,
        RESULT_NO = 4,
    };

    Result openMessageBox( std::string_view title, std::string_view message, ButtonFlag buttons = BUTTON_OK, BoxType type = TYPE_INFO );
}