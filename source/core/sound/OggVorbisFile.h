#pragma once

#include <game/SoundFile.h>
#include <game/SoundFormat.h>

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

	SoundFormat format_;

public:
	explicit OggVorbisFile( const char* );
	~OggVorbisFile();
	
	SizeType size() const override;
	SizeType size_per_sec() const override;

	SizeType read( void*, SizeType, bool = false ) override;
	void seek( SizeType );

	const SoundFormat& format() const override { return format_; }

}; // class OggVorbisFile
