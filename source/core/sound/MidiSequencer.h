#include <core/sound/MidiSynthesizer.h>

#define LIBREMIDI_HEADER_ONLY 1
#define LIBREMIDI_WINMM 1

#include <libremidi/libremidi.hpp>
#include <libremidi/reader.hpp>


#include <chrono>

namespace core::sound
{

/** 
 * MIDI シーケンサー
 * 
 */
class MidiSequencer
{
private:
	class Track
	{
	private:
		const libremidi::midi_track& track_;
		libremidi::midi_track::const_iterator current_track_event_;
		int last_tick_ = 0; // 最後に処理したイベントの再生開始時点からの tick 数 ( 経過時間 )

	public:
		Track( const libremidi::midi_track& track )
			: track_( track )
			, current_track_event_( track.begin() )
		{
		}

		/**
		 *
		 * 
		 * @param tick 再生開始からの経過時間 ( tick )
		 */
		void process( MidiSequencer& sequencer, int total_tick )
		{
			for ( ; current_track_event_ != track_.end(); current_track_event_++ )
			{
				if ( current_track_event_->m.is_meta_event() )
				{
					// std::cout << std::format( "meta : {:#x}", int( current_track_event_->m.get_meta_event_type() ) ) << std::endl;

					if ( current_track_event_->m.get_meta_event_type() == libremidi::meta_event_type::TRACK_NAME )
					{
						std::cout << "track name : ";

						for ( int n = 3; n < current_track_event_->m.size(); n++ )
						{
							std::cout << current_track_event_->m[ n ];
						}

						std::cout << std::endl;
					}
				}

				if ( current_track_event_ == track_.end() )
				{
					break;
				}

				// std::cout << tick << " " << midi_event->tick << " " << int( midi_event->m.get_message_type() ) << std::endl;

				if ( total_tick  >= current_track_event_->tick + last_tick_ )
				{
					sequencer.process_midi_message( current_track_event_->m );

					last_tick_ += current_track_event_->tick;
				}
				else
				{
					break;
				}
			}
		}
	};
private:
	libremidi::reader reader_;

	float bpm_ = 0.f;
	float ticks_per_ms_ = 0.f;

	float elapsed_ticks_ = 0.f; // 再生を開始してからの経過 Ticks

	// auto midi_event = track.begin();
    std::chrono::system_clock::time_point last_time_ = std::chrono::system_clock::now(); //

	std::vector< Track > tracks_;

	MidiSynthesizer* midi_synthesizer_ = nullptr;

	int beat_ = 0; // beat 数 ( 再生開始からの経過 Beat )
	int last_beat_ = 0; // 最後に処理した beat

	std::function< void(int) > beat_handler_; // ビートが変化したときのハンドラ

	/**
	* MIDI メッセージを処理する
	* 
	* @param m MIDI メッセージ
	*/
	void process_midi_message( const libremidi::message& m )
	{
		if ( m.get_message_type() == libremidi::message_type::NOTE_ON && m.bytes[ 2 ] > 0 )
		{
			midi_synthesizer_->note_on( m.get_channel() - 1, m.bytes[ 1 ], m.bytes[ 2 ] / 127.f );
		}
		else if ( m.get_message_type() == libremidi::message_type::NOTE_OFF || m.get_message_type() == libremidi::message_type::NOTE_ON && m.bytes[ 2 ] == 0 )
		{
			midi_synthesizer_->note_off( m.get_channel() - 1, m.bytes[ 1 ] );
		}
		else if ( m.get_message_type() == libremidi::message_type::PROGRAM_CHANGE )
		{
			midi_synthesizer_->program_change( m.get_channel() - 1, m.bytes[ 1 ], m.get_channel() == 10 );
		}
		else if ( m.get_message_type() == libremidi::message_type::CONTROL_CHANGE )
		{
			const auto control_number = m.bytes[ 1 ];
			const auto value = m.bytes[ 2 ];

			midi_synthesizer_->control_change( m.get_channel() - 1, control_number, value );

			/*
			if ( control_number == CONTROL_NUMBER_MASTER_VOLUME )
			{
				master_volume = value / 127.f;
			}
			else if ( control_number == CONTROL_NUMBER_TEMPO )
			{
				bpm_factor = value / 127.f * 2.f;
			}
			else if ( control_number == CONTROL_NUMBER_DELAY_LEVEL )
			{
				delay_level_factor = value / 127.f;
			}
			else if ( control_number == CONTROL_NUMBER_DELAY_FEEDBACK )
			{
				delay_feedback_factor = value / 127.f;
			}
			else if ( control_number == CONTROL_NUMBER_LOW_PASS )
			{
				low_pass_factor = value / 127.f;
				biquad_filter->setCutoff( std::lerp( 20.f, 4000.f, low_pass_factor ) );
			}

			std::cout << "master_volume:" << master_volume << ", bpm factor:" << bpm_factor << ", delay level factor:" << delay_level_factor << ", delay feedback factor:" << delay_feedback_factor << ", low pass factor:" << low_pass_factor << std::endl;
			*/
		}
		else
		{
			std::cout << "unknown message type : " << std::format( "meta : {:#x}", int( m.get_message_type() ) ) << std::endl;
		}
	}

public:
	MidiSequencer( const char* file_path, MidiSynthesizer* synthesizer )
		: midi_synthesizer_( synthesizer )
	{
		// Read raw from a MIDI file
		std::ifstream file{ file_path , std::ios::binary };

		std::vector<uint8_t> bytes;
		bytes.assign( std::istreambuf_iterator< char >( file ), std::istreambuf_iterator< char >() );

		libremidi::reader::parse_result result = reader_.parse( bytes );

		if ( result == libremidi::reader::invalid )
		{
			throw "load midi file failed";
		}

		auto bpm = reader_.startingTempo;

		for ( const auto& e : reader_.tracks[ 0 ] )
		{
			if ( e.m.is_meta_event() )
			{
				std::cout << std::format( "meta : {:#x}", int( e.m.get_meta_event_type() ) ) << std::endl;

				if ( e.m.get_meta_event_type() == libremidi::meta_event_type::TRACK_NAME )
				{
					std::cout << "track name : ";

					for ( int n = 3; n < e.m.size(); n++ )
					{
						std::cout << e.m[ n ];
					}

					std::cout << std::endl;
				}

				if ( e.m.get_meta_event_type() == libremidi::meta_event_type::TEMPO_CHANGE )
				{
					uint32_t tempo = e.m[ 3 ] << 16 | e.m[ 4 ] << 8 | e.m[ 5 ];

					bpm = 60.f / ( static_cast< float >( tempo ) / 1000.f / 1000.f );
				}
			}
		}

		set_bpm( bpm );

		for ( auto& t : reader_.tracks )
		{
			tracks_.emplace_back( t );
		}
	}

	/**
	 * 現在の BPM を取得する
	 * 
	 * @return 現在の BPM
	 */
	float get_bpm() const
	{
		return bpm_;
	}

	/**
	 * BPM を設定する
	 * 
	 * @param bpm 設定する BPM
	 */
	void set_bpm( float bpm )
	{
		auto changed = bpm_ != bpm;

		bpm_ = bpm;

		if ( changed )
		{
			std::cout << "bpm: " << bpm_ << std::endl;
		}

		// std::cout << "tpb : " << reader_.ticksPerBeat << std::endl;

		const auto ticks_per_sec = bpm / 60 * reader_.ticksPerBeat;
		ticks_per_ms_ = ticks_per_sec / 1000;

		// std::cout << "ticks_per_sec : " << ticks_per_sec << std::endl;
		// std::cout << "ticks_per_ms : " << ticks_per_ms_ << std::endl;
	}

	void set_beat_handler( std::function< void(int) > handler )
	{
		beat_handler_ = std::move( handler );
	}

	int get_ticks_per_beat() const
	{
		return reader_.ticksPerBeat;
	}

	int get_ticks() const
	{
		return static_cast< int >( std::fmod( elapsed_ticks_, get_ticks_per_beat() ) );
	}

	void process()
	{
		auto now = std::chrono::system_clock::now();

		elapsed_ticks_ += std::chrono::duration_cast< std::chrono::milliseconds >( now - last_time_ ).count() * ticks_per_ms_;
		last_time_ = now;

		beat_ = static_cast< int >( elapsed_ticks_ / reader_.ticksPerBeat );
		
		if ( beat_ != last_beat_ )
		{
			if ( beat_handler_ )
			{
				beat_handler_( beat_ );
			}
			
			// std::cout << "beat: " << beat_ << std::endl;
			last_beat_ = beat_;
		}

		for ( auto& t : tracks_ )
		{
			t.process( * this, static_cast< int >( elapsed_ticks_ ) );
		}
	}
};

} // namespace core::sound
