#pragma once

#include <type/type.h>
#include <windows.h>

namespace win
{

class Clipboard
{
public:

private:
	

public:
	static string_t get_text()
	{
		string_t text = "";

		if ( OpenClipboard( NULL ) )
		{
			HGLOBAL data = GetClipboardData( CF_TEXT );

			if ( data )
			{
				LPTSTR str = static_cast< LPTSTR >( GlobalLock( data ) );

				if ( str )
				{
					text = str;

					GlobalUnlock( data );
				}
			}

			CloseClipboard();
        }

		return text;
    }
};

} // namespace win