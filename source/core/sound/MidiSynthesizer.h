#include <game/SoundFormat.h>
#include <common/exception.h>

#include <tsf.h>

#include <iostream>
#include <string>

namespace core::sound
{

/** 
 * MIDI シンセサイザー
 * 
 */
class MidiSynthesizer
{
public:
	using SoundFormat = game::SoundFormat;
	
private:
	tsf* sound_font_ = nullptr;

public:
	MidiSynthesizer( const char* sound_font_file_path, const SoundFormat& format )
	{
		sound_font_ = tsf_load_filename( sound_font_file_path );

		if ( ! sound_font_ )
		{
			// どのファイルが読めなかったかを残す（SoundFont は git に無く、tools/soundfont/fetch_soundfont.py で入手する）
			COMMON_THROW_EXCEPTION_MESSAGE( std::string( "音色（SoundFont）を読めません : " ) + sound_font_file_path + "（tools/soundfont/fetch_soundfont.py で入手する）" );
		}

		// 読めたことを残す（GameMain が生きている間、std::cout はログに流れる）
		std::cout << "[Sound] 音色（SoundFont）を読みました : " << sound_font_file_path << "（プリセット " << tsf_get_presetcount( sound_font_ ) << " 個）" << std::endl;

		if ( format.channels != 2 )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( "MidiSynthesizer::MidiSynthesizer() failed. invalid channels." );
		}

		tsf_set_output( sound_font_, TSF_STEREO_INTERLEAVED, static_cast< int >( format.sampling_rate ) );
	}

	~MidiSynthesizer()
	{
		tsf_close( sound_font_ );
	}

	void note_on( int channel, int key, float vel )
	{
		tsf_channel_note_on( sound_font_, channel, key, vel );
	}

	void note_off( int channel, int key )
	{
		tsf_channel_note_off( sound_font_, channel, key );
	}

	void program_change( int channel, int preset_number, bool is_drum )
	{
		tsf_channel_set_presetnumber( sound_font_, channel, preset_number, is_drum );
	}
	

	void control_change( int channel, int control_number, int value )
	{
		tsf_channel_midi_control( sound_font_, channel, control_number, value );
	}

	void render( float* out, size_t frame_count )
	{
		tsf_render_float( sound_font_, out, static_cast< int >( frame_count ) );
	}
};

} // namespace core::sound
