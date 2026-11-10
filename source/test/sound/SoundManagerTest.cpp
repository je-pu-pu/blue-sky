#include "pch.h"

#include <core/sound/SoundManager.h>
#include <core/sound/SoundEngine.h>
#include <core/sound/SoundBuffer.h>
#include <core/sound/Sound.h>
#include <game/SoundFormat.h>

#include <filesystem>
#include <memory>
#include <vector>

// ---- 音のハンドル : 名前でもハンドルでも引け、読み直し（差し替え）でも番号が変わらず、外すと nullptr になること ----
// 音の出力の機器を使わないよう、偽物のエンジン（バッファをメモリに取るだけ）を渡す。音のファイルは本物（media/sound）を読む

namespace
{

/// メモリに取るだけのバッファ（生きている数・作った数を数える。音が解放されるとバッファも解放される）
class FakeSoundBuffer : public core::sound::SoundBuffer
{
public:
	static inline int alive = 0;
	static inline int created = 0;

private:
	std::vector< unsigned char > data_;

public:
	explicit FakeSoundBuffer( size_t size ) : data_( size ) { alive++; created++; }
	~FakeSoundBuffer() override { alive--; }

	void init_3d_sound() override { }

	void* lock( size_t offset = 0, size_t = 0 ) override { return data_.data() + offset; }
	void unlock() override { }

	void set_3d_position( const core::Vector3& ) override { }
	void set_3d_velocity( const core::Vector3& ) override { }

	float get_volume() const override { return 1.f; }
	void set_volume( float ) override { }

	float get_speed() const override { return 1.f; }
	void set_speed( float ) override { }

	void play( bool ) override { }
	bool is_playing() const override { return false; }
	void stop() override { }

	size_t get_size() const override { return data_.size(); }
	size_t get_current_position() const override { return 0; }
};

/// バッファを作るだけのエンジン
class FakeSoundEngine : public core::sound::SoundEngine
{
	SoundFormat format_ { 2, 44100, 16 };

public:
	void set_volume( float ) override { }

	SoundBuffer* create_sound_buffer( bool, bool, size_t size, const SoundFormat& ) override { return new FakeSoundBuffer( size ); }

	void set_listener_position( const core::Vector3& ) override { }
	void set_listener_velocity( const core::Vector3& ) override { }
	void set_listener_orientation( const core::Vector3&, const core::Vector3& ) override { }
	void commit() override { }

	const SoundFormat& get_format() const override { return format_; }

	void add_sound_filter( SoundFilter* ) override { }
	void clear_sound_filter_list() override { }

	MidiSynthesizer* get_midi_synthesizer() override { return nullptr; }
};

/// SoundManager は作業ディレクトリ（source/）からの相対の場所で音を読むので、テストの間だけ移る
struct CurrentPathScope
{
	std::filesystem::path previous = std::filesystem::current_path();
	CurrentPathScope() { std::filesystem::current_path( std::filesystem::path( __FILE__ ).parent_path() / "../.." ); }
	~CurrentPathScope() { std::filesystem::current_path( previous ); }
};

} // namespace

TEST( SoundManagerTest, 同じ名前で別のファイルを読むとハンドルは同じまま新しい音を引け古い音は解放される )
{
	CurrentPathScope current_path_scope;
	FakeSoundBuffer::alive = 0;
	FakeSoundEngine engine;
	core::sound::SoundManager m( & engine );

	core::Sound* a = m.load( "se", "silence" );
	const core::SoundHandle h = m.get_sound_handle( "se" );
	ASSERT_FALSE( h.is_null() );
	EXPECT_EQ( a, m.resolve( h ) );
	EXPECT_EQ( a, m.get_sound( "se" ) );
	EXPECT_EQ( a, m.load( "se", "silence" ) );		// 同じファイルなら使い回す
	EXPECT_EQ( 1, FakeSoundBuffer::alive );

	core::Sound* b = m.load( "se", "fall" );		// 同じ名前で別のファイル（ステージの load_sound と同じ）
	EXPECT_EQ( h, m.get_sound_handle( "se" ) );		// 番号は変わらない
	EXPECT_EQ( b, m.resolve( h ) );
	EXPECT_EQ( b, m.get_sound( "se" ) );
	EXPECT_EQ( std::string( "media/sound/fall.ogg" ), b->get_file_name() );
	EXPECT_EQ( 1, FakeSoundBuffer::alive );			// 古い音は解放された

	m.unload( "se" );
	EXPECT_EQ( nullptr, m.resolve( h ) );			// 外した後は nullptr
	EXPECT_EQ( nullptr, m.get_sound( "se" ) );
	EXPECT_EQ( 0, FakeSoundBuffer::alive );
}

TEST( SoundManagerTest, 読み直すと同じファイルでも作り直しハンドルは同じまま )
{
	CurrentPathScope current_path_scope;
	FakeSoundBuffer::alive = 0;
	FakeSoundEngine engine;
	core::sound::SoundManager m( & engine );

	core::Sound* first = m.reload( "fin", "silence" );	// 読み込んでいなければ普通に読む
	const core::SoundHandle h = m.get_sound_handle( "fin" );
	EXPECT_EQ( first, m.resolve( h ) );

	const int created = FakeSoundBuffer::created;
	core::Sound* second = m.reload( "fin", "silence" );	// load_sound fin silence 1（replace）と同じ

	EXPECT_EQ( created + 1, FakeSoundBuffer::created );	// 作り直した
	EXPECT_EQ( 1, FakeSoundBuffer::alive );				// 古い音は解放された
	EXPECT_EQ( h, m.get_sound_handle( "fin" ) );
	EXPECT_EQ( second, m.resolve( h ) );

	m.unload_all();
	EXPECT_EQ( nullptr, m.resolve( h ) );
	EXPECT_EQ( 0, FakeSoundBuffer::alive );
}

TEST( SoundManagerTest, グループを外すとそのグループで読んだ音のハンドルは無効になる )
{
	CurrentPathScope current_path_scope;
	FakeSoundBuffer::alive = 0;
	FakeSoundEngine engine;
	core::sound::SoundManager m( & engine );

	m.load( "keep", "silence" );
	const core::SoundHandle keep = m.get_sound_handle( "keep" );

	m.push_group( "stage" );
	m.load( "stage-se", "silence" );
	const core::SoundHandle stage = m.get_sound_handle( "stage-se" );
	m.pop_group();

	EXPECT_EQ( nullptr, m.resolve( stage ) );
	EXPECT_NE( nullptr, m.resolve( keep ) );
	EXPECT_EQ( 1, FakeSoundBuffer::alive );
	EXPECT_TRUE( m.get_sound_handle( "none" ).is_null() );
	EXPECT_EQ( nullptr, m.resolve( core::SoundHandle() ) );
}
