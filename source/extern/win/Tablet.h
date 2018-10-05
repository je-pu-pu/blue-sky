#ifndef WIN_TABLET_H
#define WIN_TABLET_H

#include <Windows.h>

#include <common/exception.h>

#include "MSGPACK.H"
#include "WINTAB.H"
#define PACKETDATA PK_CURSOR | PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_ORIENTATION
#define PACKETMODE PK_BUTTONS
#include "PKTDEF.H"

#include <boost/utility.hpp>

#include <sstream>

namespace win
{

/**
 * ペンタブレット
 *
 */
class Tablet : private boost::noncopyable
{
public:
	constexpr static bool is_enabled() { return false; }

private:
	HMODULE module_ = 0;
	HCTX context_handle_ = 0;

	AXIS pressure_axis_;
	AXIS orientation_axis_[ 3 ];

	int cursor_index_ = 0;
	float x_ = 0.f;
	float y_ = 0.f;
	float pressure_ = 0.f;
	float azimuth_ = 0.f;
	float altitude_ = 0.f;

	static UINT ( API * DllWTInfo )( UINT, UINT, LPVOID );
	static HCTX ( API * DllWTOpen )( HWND, LPLOGCONTEXTA, BOOL );
	static BOOL ( API * DllWTClose )( HCTX );

	static BOOL ( API * DllWTEnable )( HCTX, BOOL );
	static BOOL ( API * DllWTPacket )( HCTX, UINT, LPVOID );
	static BOOL ( API * DllWTOverlap )( HCTX, BOOL );

protected:
	explicit Tablet( HWND );

	template< typename T >
	void get_proc_address( T& f, const char* name )
	{
		T x = reinterpret_cast< T >( GetProcAddress( module_, name ) );

		if ( ! x )
		{
			std::stringstream ss;
			ss << "GetProcAddress( " << name << " ) failed.";

			COMMON_THROW_EXCEPTION_MESSAGE( ss.str() );
		}

		f = x;
	}

public:
	~Tablet();
	
	void on_activate( WPARAM, LPARAM );
	void on_packet( WPARAM, LPARAM );

	static Tablet* get_instance( HWND hwnd )
	{
		static Tablet t( hwnd );

		return & t;
	}

	int get_cursor_index() const { return cursor_index_; }
	float get_x() const { return x_; }
	float get_y() const { return y_; }
	float get_pressure() const { return pressure_; }
	float get_azimuth() const { return azimuth_; }
	float get_altitude() const { return altitude_; }

}; // class Tablet

} // namespace win

#endif // WIN_TABLET_H

