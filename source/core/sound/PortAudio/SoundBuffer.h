#pragma once

#include <core/sound/SoundBuffer.h>
#include <core/sound/SampleRateStepper.h>
#include <game/SoundFormat.h>
#include <vector>
#include <atomic>

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

	// --- 3D（方向＋距離）。position は game スレッドが書き audio コールバックが読むので atomic ---
	std::atomic< bool >  is_3d_{ false };
	std::atomic< float > pos_x_{ 0.f };
	std::atomic< float > pos_y_{ 0.f };
	std::atomic< float > pos_z_{ 0.f };
	// audio コールバック内スクラッチ: 先頭で L/R ゲインを算出→フレームループで使う（同一スレッド・ロック不要）
	float mix_gain_l_ = 1.f;
	float mix_gain_r_ = 1.f;

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

	// --- 3D 用アクセサ ---
	bool is_3d() const { return is_3d_.load( std::memory_order_relaxed ); }
	Vector3 get_3d_position() const
	{
		return Vector3(
			pos_x_.load( std::memory_order_relaxed ),
			pos_y_.load( std::memory_order_relaxed ),
			pos_z_.load( std::memory_order_relaxed ) );
	}
	void set_mix_gains( float l, float r ) { mix_gain_l_ = l; mix_gain_r_ = r; }
	float mix_gain_l() const { return mix_gain_l_; }
	float mix_gain_r() const { return mix_gain_r_; }

}; // class SoundBuffer

} // namespace core::sound
