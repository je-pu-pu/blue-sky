#pragma once

#include <core/sound/SoundEngine.h>
#include "SoundBuffer.h"
#include <game/SoundFormat.h>
#include <portaudio/portaudio.h>
#include <mutex>

namespace core::sound
{
	class SoundFilter;
}

namespace core::sound::port_audio
{

/**
 * PortAudio を使った SoundEngine
 *
 */
class SoundEngine : public core::sound::SoundEngine
{
public:
	using SoundBuffer = port_audio::SoundBuffer;

private:
	PaStream* stream_ = nullptr;
	std::vector< SoundBuffer* > sound_buffer_list_;
	SoundFormat format_;

	std::mutex callback_mutex_;

	std::vector< SoundFilter* > sound_filter_list_;

	std::unique_ptr< MidiSynthesizer > midi_synthesizer_;

	float volume_ = 1.0f;

	// --- 3D リスナー（自前の方向＋距離）。staged=game スレッドが書き、commit() で active へ publish。
	// active は callback_mutex_ 下でのみ触る（コールバックが読む）。---
	Vector3 listener_pos_staged_     { 0.f, 0.f, 0.f };
	Vector3 listener_forward_staged_ { 0.f, 0.f, 1.f };
	Vector3 listener_up_staged_      { 0.f, 1.f, 0.f };
	Vector3 listener_pos_     { 0.f, 0.f, 0.f };
	Vector3 listener_forward_ { 0.f, 0.f, 1.f };
	Vector3 listener_up_      { 0.f, 1.f, 0.f };
	float ref_distance_ = 2.f;   ///< これ以内は減衰なし (m)
	float max_distance_ = 45.f;  ///< これ以遠は無音 (m)

	static int callback( const void* input, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* );

public:
	SoundEngine();
	~SoundEngine();

	void set_volume( float ) override;

	SoundBuffer* create_sound_buffer( bool is_3d, bool is_streaming, size_t size, const SoundFormat& format ) override;
	void unregister_sound_buffer( SoundBuffer* );

	// 3D
	void set_listener_position( const Vector3& ) override;
	void set_listener_velocity( const Vector3& ) override;
	void set_listener_orientation( const Vector3& , const Vector3& ) override;
	void commit() override;

	/**
	 * 音源の位置 → 左右のゲイン（距離の減衰＋等パワーのパン）。コールバックが 3D の音ごとに呼ぶ（テストで中身を確かめる）
	 *
	 * @param ref_distance これ以内は減衰しない (m)
	 * @param max_distance これより遠いと鳴らない (m)。その間は線形
	 */
	static void compute_3d_gains( const Vector3& source, const Vector3& listener_position, const Vector3& listener_forward, const Vector3& listener_up, float ref_distance, float max_distance, float& out_l, float& out_r );

	const SoundFormat& get_format() const override { return format_; }

	void add_sound_filter( SoundFilter* ) override;
	void clear_sound_filter_list() override;

	MidiSynthesizer* get_midi_synthesizer() override { return midi_synthesizer_.get(); }

}; // class SoundEngine

} // namespace core::sound::port_audio
