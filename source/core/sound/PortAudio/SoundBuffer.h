#pragma once

#include <core/sound/SoundBuffer.h>
#include <core/sound/SampleRateStepper.h>
#include <game/SoundFormat.h>
#include <vector>

namespace core::sound::port_audio
{
	class SoundEngine;

/**
 * PortAudio サウンドバッファ
 *
 */
class SoundBuffer : public core::sound::SoundBuffer
{
public:
	using SoundFormat = game::SoundFormat;

private:
	SoundEngine& sound_engine_;
	SoundFormat format_;

	std::vector< u8_t > data_;
	float volume_ = 1.f;
	bool is_playing_ = false;
	bool is_looping_ = false;
	size_t current_position_ = 0;		///< 次に読むバイトの位置（フレームの境目）
	SampleRateStepper rate_stepper_;	///< この音の周波数と出力の周波数の違いを吸収する

	float sample_at( size_t ) const;

public:
	SoundBuffer( SoundEngine&, const SoundFormat&, size_t );
	~SoundBuffer();

	void init_3d_sound() override;

	void* lock( size_t, size_t ) override;
	void unlock() override;

	void set_3d_position( const Vector3& ) override;
	void set_3d_velocity( const Vector3& ) override;

	float get_volume() const override;
	void set_volume( float ) override;

	float get_speed() const override;
	void set_speed( float ) override;

	void play( bool ) override;
	bool is_playing() const override;
	void stop() override;

	size_t get_size() const override;
	size_t get_current_position() const override;

	/// 出力の 1 フレーム分（左右）を読み、出力の周波数に合わせて読む位置を進める（モノラルは左右に同じ値）
	void read_frame( float& left, float& right );
	const SoundFormat& get_format() const { return format_; }

}; // class SoundBuffer

} // namespace core::sound
