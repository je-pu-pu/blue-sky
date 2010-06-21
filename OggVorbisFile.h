#ifndef OGG_VORBIS_FILE_H
#define OGG_VORBIS_FILE_H

#include <game/SoundFile.h>
#include <vorbis/vorbisfile.h>
#include <windows.h>

/**
 * Ogg Vorbis File
 *
 */
class OggVorbisFile : public game::SoundFile
{
private:
	OggVorbis_File	file_;
	vorbis_info*	vorbis_info_;

	WAVEFORMATEX	format_;

public:
	OggVorbisFile( const char* );
	~OggVorbisFile();
	
	WAVEFORMATEX& format() { return format_; }

	SizeType size() const;
	SizeType read( void*, SizeType  );

}; // class OggVorbisFile

#endif // OGG_VORBIS_FILE_H
