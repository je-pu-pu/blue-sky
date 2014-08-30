#include "Tablet.h"


namespace win
{

UINT ( API * Tablet::DllWTInfo )( UINT, UINT, LPVOID );
HCTX ( API * Tablet::DllWTOpen )( HWND, LPLOGCONTEXTA, BOOL );
BOOL ( API * Tablet::DllWTClose )( HCTX );
BOOL ( API * Tablet::DllWTEnable )( HCTX, BOOL );
BOOL ( API * Tablet::DllWTPacket )( HCTX, UINT, LPVOID );
BOOL ( API * Tablet::DllWTOverlap )( HCTX, BOOL );

Tablet::Tablet( HWND hwnd )
	: module_( 0 )
	, context_handle_( 0 )
	, cursor_index_( 0 )
	, x_( 0.f )
	, y_( 0.f )
	, pressure_( 0.f )
{
	module_ = LoadLibrary( "Wintab32.dll" );

	if ( ! module_ )
	{
		std::stringstream ss;
		ss << "LoadLibrary( Wintab32.dll ) failed. ( error : " << GetLastError() << " )";

		COMMON_THROW_EXCEPTION_MESSAGE( ss.str() );
	}

	get_proc_address( DllWTInfo,  "WTInfoA" );
	get_proc_address( DllWTOpen,  "WTOpenA" );
	get_proc_address( DllWTClose, "WTClose" );
	get_proc_address( DllWTEnable, "WTEnable" );
	get_proc_address( DllWTPacket, "WTPacket" );
	get_proc_address( DllWTOverlap, "WTOverlap" );

	if ( ! DllWTInfo( 0, 0, nullptr ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "DllWTInfo() failed." );
	}
	
	LOGCONTEXT log_context_ = { 0 };
	log_context_.lcOptions |= CXO_SYSTEM;

	if ( DllWTInfo( WTI_DEFSYSCTX, 0, & log_context_ ) != sizeof( LOGCONTEXT ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "DllWTInfo() failed." );
	}

	wsprintf( log_context_.lcName, "Tablet %x", module_ );
	log_context_.lcOptions |= CXO_MESSAGES;
	log_context_.lcPktData = PACKETDATA;
	log_context_.lcPktMode = PACKETMODE;
	log_context_.lcMoveMask = PACKETDATA;
	log_context_.lcBtnUpMask = log_context_.lcBtnDnMask;

	AXIS x_axis = { 0 };
	AXIS y_axis = { 0 };

	if ( DllWTInfo( WTI_DEVICES, DVC_X, & x_axis ) != sizeof( AXIS ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "DllWTInfo( DVC_X ) failed." );
	}
	if ( DllWTInfo( WTI_DEVICES, DVC_Y, & y_axis ) != sizeof( AXIS ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "DllWTInfo( DVC_Y ) failed." );
	}
	if ( DllWTInfo( WTI_DEVICES, DVC_NPRESSURE, & pressure_axis_ ) != sizeof( AXIS ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "DllWTInfo( DVC_NPRESSURE ) failed." );
	}
	if ( DllWTInfo( WTI_DEVICES, DVC_ORIENTATION, & orientation_axis_ ) != sizeof( orientation_axis_ ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "DllWTInfo( DVC_ORIENTATION ) failed." );
	}
	
	log_context_.lcInOrgX = 0;
	log_context_.lcInOrgY = 0;
	log_context_.lcInExtX = x_axis.axMax;
	log_context_.lcInExtY = y_axis.axMax;

	log_context_.lcOutOrgX =  GetSystemMetrics( SM_XVIRTUALSCREEN );
	log_context_.lcOutOrgY =  GetSystemMetrics( SM_YVIRTUALSCREEN );
	log_context_.lcOutExtX =  GetSystemMetrics( SM_CXVIRTUALSCREEN );
	log_context_.lcOutExtY = -GetSystemMetrics( SM_CYVIRTUALSCREEN );

	context_handle_ = DllWTOpen( hwnd, & log_context_, FALSE );
}

Tablet::~Tablet()
{
	if ( context_handle_ )
	{
		DllWTClose( context_handle_ );
	}

	if ( ! FreeLibrary( module_ ) )
	{
		std::stringstream ss;
		ss << "FreeLibrary( Wintab32.dll ) failed. ( error : " << GetLastError() << " )";

		COMMON_THROW_EXCEPTION_MESSAGE( ss.str() );
	}
}

void Tablet::on_activate( WPARAM wp, LPARAM lp )
{
	if ( context_handle_ )
	{
		DllWTEnable( context_handle_, GET_WM_ACTIVATE_STATE( wp, lp ) );
		
		if ( context_handle_ && GET_WM_ACTIVATE_STATE( wp, lp ) )
		{
			DllWTOverlap( context_handle_, TRUE );
		}
	}
}

void Tablet::on_packet( WPARAM wp, LPARAM lp )
{
	PACKET p;

	if ( DllWTPacket( ( HCTX ) lp, wp, & p ) ) 
	{
		cursor_index_ = p.pkCursor;
		x_ = static_cast< float >( p.pkX );
		y_ = static_cast< float >( p.pkY );
		pressure_ = ( p.pkNormalPressure - pressure_axis_.axMin ) / static_cast< float >( pressure_axis_.axMax - pressure_axis_.axMin );
		azimuth_ = ( p.pkOrientation.orAzimuth - orientation_axis_[ 0 ].axMin ) / static_cast< float >( orientation_axis_[ 0 ].axMax - orientation_axis_[ 0 ].axMin );
		altitude_ = ( p.pkOrientation.orAltitude - orientation_axis_[ 1 ].axMin ) / static_cast< float >( orientation_axis_[ 1 ].axMax - orientation_axis_[ 1 ].axMin );
	}
}

} // namespace win


