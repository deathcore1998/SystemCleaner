#include "dialogs.hpp"

#include <windows.h>
#include <shobjidl.h>
#include <shlobj.h>

common::OptionalPath utils::openSelectionDialog( std::string_view title, DialogType type )
{
    HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
    if ( FAILED( hr ) )
    {
        return std::nullopt;
    }

    IFileOpenDialog* pFileOpen = nullptr;
    hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast< void** >( &pFileOpen ) );

    common::OptionalPath result = std::nullopt;

    if ( SUCCEEDED( hr ) )
    {
        FILEOPENDIALOGOPTIONS options;
        pFileOpen->GetOptions( &options );

        options |= FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM;
        options |= type == DialogType::FILE ? FOS_FILEMUSTEXIST : FOS_PICKFOLDERS;
        pFileOpen->SetOptions( options );
        pFileOpen->SetTitle( std::wstring( title.begin(), title.end() ).c_str() );

        hr = pFileOpen->Show( NULL );
        if ( SUCCEEDED( hr ) )
        {
            IShellItem* pItem = nullptr;
            hr = pFileOpen->GetResult( &pItem );

            if ( SUCCEEDED( hr ) )
            {
                PWSTR pszFilePath = nullptr;
                hr = pItem->GetDisplayName( SIGDN_FILESYSPATH, &pszFilePath );

                if ( SUCCEEDED( hr ) )
                {
                    result = fs::path( pszFilePath );
                    CoTaskMemFree( pszFilePath );
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }

    CoUninitialize();
    return result;
}

common::OptionalPath utils::openFileDialog()
{
    return openSelectionDialog( "Select file to clean", DialogType::FILE );
}

common::OptionalPath utils::openFolderDialog()
{
    return openSelectionDialog( "Select folder to clean", DialogType::FOLDER );
}

utils::Result utils::openMessageBox( std::string_view title, std::string_view message, ButtonFlag buttons, BoxType type )
{
    UINT winButtons = 0;

    switch ( static_cast< uint32_t >( buttons ) )
    {
        case ( BUTTON_YES | BUTTON_NO | BUTTON_CANCEL ):
            winButtons = MB_YESNOCANCEL;
            break;
        case ( BUTTON_YES | BUTTON_NO ):
            winButtons = MB_YESNO;
            break;
        case ( BUTTON_OK | BUTTON_CANCEL ):
            winButtons = MB_OKCANCEL;
            break;
        case BUTTON_OK:
            winButtons = MB_OK;
            break;
        case BUTTON_YES:
            winButtons = MB_YESNO;
            break;
        default:
            winButtons = MB_OK;
            break;
    }

    UINT winIcon = 0;
    switch ( type )
    {
        case TYPE_WARNING:  
            winIcon = MB_ICONWARNING;
            break;
        case TYPE_ERROR:    
            winIcon = MB_ICONERROR; 
            break;
        case TYPE_QUESTION: 
            winIcon = MB_ICONQUESTION; 
            break;
        case TYPE_INFO:
        default:           
            winIcon = MB_ICONINFORMATION; 
            break;
    }

    int result = MessageBoxA( nullptr, message.data(), title.data(), winButtons | winIcon | MB_TOPMOST );
    switch ( result )
    {
        case IDOK:     
            return RESULT_OK;
        case IDCANCEL: 
            return RESULT_CANCEL;
        case IDYES:    
            return RESULT_YES;
        case IDNO:     
            return RESULT_NO;
        default:       
            return RESULT_NONE;
    }
}
