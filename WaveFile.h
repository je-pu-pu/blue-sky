#ifndef WAVE_FILE_H
#define WAVE_FILE_H

#include <windows.h>

/**
 * WAVE File
 *
 */
class WaveFile
{
private:
	HMMIO			hmmio_;

	WAVEFORMATEX	format_;
	
	MMCKINFO		riff_chunk_;
	MMCKINFO		format_chunk_;
	MMCKINFO		data_chunk_;

public:
	WaveFile( const char* );
	~WaveFile();
	
	WAVEFORMATEX& format() { return format_; }

//	char* data() { return data_; }
	DWORD size() const { return data_chunk_.cksize; }
	DWORD read( void*, DWORD );

}; // class DirectSound

#endif // WAVE_FILE_H
