#include "CrashReport.h"

#include <windows.h>
#include <dbghelp.h>

#include <cstdio>

#pragma comment( lib, "dbghelp.lib" )

namespace blue_sky
{

namespace
{

/// 呼び出し履歴を 1 行ずつ書く（関数名・ファイル名・行番号。引けなければアドレスとモジュール）
void write_stack( FILE* f, CONTEXT context )
{
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	SymSetOptions( SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS );
	SymInitialize( process, nullptr, TRUE );

	STACKFRAME64 frame = { };
	frame.AddrPC.Offset = context.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;

	for ( int i = 0; i < 48; i++ )
	{
		if ( ! StackWalk64( IMAGE_FILE_MACHINE_AMD64, process, thread, & frame, & context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr ) )
		{
			break;
		}

		const DWORD64 address = frame.AddrPC.Offset;

		if ( address == 0 )
		{
			break;
		}

		char symbol_buffer[ sizeof( SYMBOL_INFO ) + 256 ] = { };
		SYMBOL_INFO* symbol = reinterpret_cast< SYMBOL_INFO* >( symbol_buffer );
		symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
		symbol->MaxNameLen = 255;

		DWORD64 symbol_offset = 0;
		const bool has_symbol = SymFromAddr( process, address, & symbol_offset, symbol ) != FALSE;

		IMAGEHLP_LINE64 line = { };
		line.SizeOfStruct = sizeof( line );
		DWORD line_offset = 0;
		const bool has_line = SymGetLineFromAddr64( process, address, & line_offset, & line ) != FALSE;

		char module_name[ MAX_PATH ] = "?";
		if ( HMODULE module = reinterpret_cast< HMODULE >( SymGetModuleBase64( process, address ) ) )
		{
			GetModuleFileNameA( module, module_name, MAX_PATH );
		}

		std::fprintf( f, "  #%02d %016llx %s", i, static_cast< unsigned long long >( address ), has_symbol ? symbol->Name : "(関数名なし)" );

		if ( has_line )
		{
			std::fprintf( f, "  %s(%lu)", line.FileName, line.LineNumber );
		}
		else
		{
			std::fprintf( f, "  [%s]", module_name );
		}

		std::fprintf( f, "\n" );
	}

	SymCleanup( process );
}

LONG WINAPI on_unhandled_exception( EXCEPTION_POINTERS* info )
{
	CreateDirectoryA( "log", nullptr );

	FILE* f = nullptr;

	if ( fopen_s( & f, "log/crash.log", "w" ) == 0 && f )
	{
		SYSTEMTIME t;
		GetLocalTime( & t );

		const EXCEPTION_RECORD* r = info->ExceptionRecord;

		std::fprintf( f, "落ちました: %04d-%02d-%02d %02d:%02d:%02d\n", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond );
		std::fprintf( f, "例外コード: 0x%08lX  アドレス: %p\n", static_cast< unsigned long >( r->ExceptionCode ), r->ExceptionAddress );

		if ( r->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && r->NumberParameters >= 2 )
		{
			std::fprintf( f, "アクセス違反: %s 0x%016llx\n", r->ExceptionInformation[ 0 ] == 0 ? "読み込み" : r->ExceptionInformation[ 0 ] == 1 ? "書き込み" : "実行", static_cast< unsigned long long >( r->ExceptionInformation[ 1 ] ) );
		}

		std::fprintf( f, "呼び出し履歴:\n" );
		write_stack( f, * info->ContextRecord );
		std::fclose( f );
	}

	return EXCEPTION_CONTINUE_SEARCH;		// 既定の処理（Windows のエラー報告）も続ける
}

} // namespace

void install_crash_report()
{
	SetUnhandledExceptionFilter( & on_unhandled_exception );
}

} // namespace blue_sky
