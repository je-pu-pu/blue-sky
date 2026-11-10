#pragma once

#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>
#include <blue_sky/graphics/RenderSettings.h>
#include <blue_sky/type.h>

#include <core/animation/SkinningAnimationSet.h>

#include <core/graphics/GraphicsManager.h>
#include <core/graphics/PrimitiveTopology.h>
#include <core/graphics/TextStyle.h>
#include <core/graphics/Mesh.h>
#include <core/graphics/LightDirection.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/CameraComponent.h>

#include <vector>
#include <functional>


namespace core
{
	class TimeManager;
	class AnimationPlayer;

	namespace ecs { class EntityManager; }

	namespace graphics
	{
		class InputLayout;
		class EffectTechnique;
		class ShadowMap;

		class Shader;
		class Texture;
		class BackBufferTexture;

		namespace direct_3d_11 { class Direct3D11; }
	}

}

namespace blue_sky::ecs
{
	struct ObjectRenderComponent;
	class ObjectRenderDataSystem;
}

namespace blue_sky::graphics
{
	class Fader;
	class ObjFileLoader;
	class FbxFileLoader;

	namespace shader
	{
		class BaseShadowMapShader;
	}

/**
 * グラフィック管理クラス
 *
 * このクラスが実体化される事はない。
 * 実際にはこのクラスを継承した blue_sky::graphics::direct_3d_11::GraphicsManager が実体化される。
 *
 * 使う側ごとの小さな窓口（RenderSettings など）も実装する。使う側は全体ではなく窓口だけを受け取る（doc/描画の窓口の分割.md）
 */
class GraphicsManager : public core::graphics::GraphicsManager, public RenderSettings
{
public:
	using Model							= Model;
	using Mesh							= Mesh;
	using Line							= Line;
	
	/**
	 * モデル・メッシュは core の型で管理し、中身は blue_sky の Model・Mesh（それ以外は入れない）
	 *
	 * ハンドル（core::graphics::ModelHandle）を core 層の ModelComponent などがそのまま持てるようにするため。
	 * 名前・ハンドルで引く関数（get_model・resolve_model など）は blue_sky の型に戻して返す
	 */
	using ModelManager					= core::ResourceManager< core::graphics::Model >;
	using MeshManager					= core::ResourceManager< core::graphics::Mesh >;

	using ShadowMap						= core::graphics::ShadowMap;
	using Sprite						= core::graphics::Sprite;

	using TransformComponent			= core::ecs::TransformComponent;
	using CameraComponent				= core::ecs::CameraComponent;
	
	using BaseShadowMapShader			= graphics::shader::BaseShadowMapShader;
	
private:
	std::unique_ptr< Fader >	fader_;
	bool_t						is_fading_in_ = true;		///< true : 現在フェードイン中 or false : 現在フェードアウト中
	float_t						fade_speed_ = 0.f;

	std::unique_ptr< Model >	post_effect_rectangle_;		///< ポストエフェクトレンダリング用矩形

	mutable std::unique_ptr< ShadowMap >					shadow_map_;
	BaseShadowMapShader*									shadow_map_shader_ = nullptr;
	BaseShadowMapShader*									shadow_map_skin_shader_ = nullptr;

	/// 紙テクスチャ（ハンドルで持つ。アンロードされたら描画で使わない。以前は生ポインタで、アンロード後に無効な参照になり得た）
	std::vector< TextureManager::Handle >	paper_texture_list_;
	TextureManager::Handle					paper_texture_;

	ModelManager				model_manager_;
	MeshManager					mesh_manager_;
	TextureManager				texture_manager_;
	SkinningAnimationSetManager skinning_animation_set_manager_;

	Texture*					null_texture_ = nullptr;

	/// GameMain が渡す依存（以前は GameMain のシングルトンから引いていた。設計上の問題 B28）
	core::ecs::EntityManager*	entity_manager_ = nullptr;		///< ゲームの物体の描画の部品を持つ（以前は ActiveObjectManager を持ち、その一覧を回していた）
	const core::TimeManager*	time_manager_ = nullptr;		///< シェーダーに渡す経過時間（以前は core::Service から取っていた）

	mutable const ConstantBuffer* current_object_constant_buffer_ = nullptr;
	mutable const ConstantBuffer* current_skinning_constant_buffer_ = nullptr;

	// Camera
	const TransformComponent*	main_camera_transform_component_ = nullptr;
	const CameraComponent*		main_camera_component_ = nullptr;

	/// 平行光源の向き（以前は setup_rendering に直接書かれていた。シーンから set_light_direction で変えられる）
	core::graphics::LightDirection	light_direction_;

	bool						is_debug_axis_enabled_ = true;

	mutable uint_t				pass_count_ = 0;
	mutable uint_t				draw_count_ = 0;

protected:
	/// ゲームの物体の描画のデータを持つ System（物体がまだ無い・EntityManager が無いなら nullptr）
	ecs::ObjectRenderDataSystem* get_object_render_data_system() const;

	// 物体 1 つを描く（以前の ActiveObject::render_mesh・render_line・bind_render_data と同じ中身）
	void bind_object_render_data( const ecs::ObjectRenderComponent& ) const;
	void render_object_mesh( const ecs::ObjectRenderComponent&, const Model* ) const;
	void render_object_mesh( const ecs::ObjectRenderComponent&, const Model*, const Shader* ) const;
	void render_object_line( const ecs::ObjectRenderComponent&, const Model* ) const;

	bool_t load_mesh( Model*, const char_t* name );
	bool_t load_line( Model*, const char_t* nmae );

	[[nodiscard]] virtual ShadowMap* create_shadow_map( uint_t, uint_t ) const = 0;
	[[nodiscard]] virtual Line* create_line() = 0;	///< Line はテクスチャを読み込むので const ではない（自分を Line に渡す）
	[[nodiscard]] virtual Texture* load_texture_file( const char_t* ) const = 0;

	[[nodiscard]] virtual Texture* load_texture_file_premultiplied( const char_t* ) const = 0;

	/**
	 * 画像ファイルを別スレッドで展開し、メインスレッドでテクスチャを作って on_created を呼ぶ（失敗したら nullptr）
	 *
	 * 実装（D3D11）は AsyncLoader を使う。on_created は dispatch_async_loads の中で呼ばれる。
	 * 今これを呼ぶのは load_named_texture_premultiplied_async だけで、それを使う側はまだ無い。
	 */
	virtual void start_texture_load_async( const string_t& file_path, std::function< void( Texture* ) > on_created ) = 0;

	// setup_default_shaders の中身（この順に呼ぶ）
	void create_default_shaders();
	void select_shadow_map_shaders();
	void setup_screen_rectangles();

	void update_constant_buffers() const;

	void render_shadow_map( const BaseShadowMapShader* , bool ) const;

	void render_debug_axis_for_bones( const core::Transform&, const core::AnimationPlayer&, const Model* ) const;
	virtual void render_debug_axis_model() const = 0;

	/// メッシュの頂点バッファを作る（create_named_mesh が使う）
	virtual Mesh::Buffer* create_mesh_buffer( Mesh::Buffer::Type ) const = 0;

	/// 名前のテクスチャを外す（空を外すときに使う）
	void unload_named_texture( const char_t* );

	// 描画の回数（実装がフレームの初め・パスごと・描くごとに数える。読むのは get_pass_count・get_draw_count）
	void clear_pass_count() const { pass_count_ = 0; }
	void count_pass() const { pass_count_++; }
	void clear_draw_count() const { draw_count_ = 0; }

public:
	/// Direct3D 11 の実装に直接触る所（頂点バッファ・Draw など）向け。抽象層（core::graphics::GraphicsManager）には置かない（B19）
	virtual core::graphics::direct_3d_11::Direct3D11* get_direct_3d() const = 0;

	GraphicsManager();
	virtual ~GraphicsManager() override;

	struct DisplayMode
	{
		int width;
		int height;
	};

	virtual int get_screen_width() const = 0;		///< 画面の幅を返す
	virtual int get_screen_height() const = 0;		///< 画面の高さを返す

	/// システムが対応する表示解像度のリストを返す
	virtual std::vector< DisplayMode > get_available_display_modes() const = 0;

	void setup_shadow_map( uint_t, uint_t );
	void unset_shadow_map();

	void update();

	virtual void set_default_viewport() = 0;
	virtual void set_viewport( float_t x, float_t y, float_t w, float_t h, float_t min_d = 0.f, float_t max_d = 1.f ) = 0;

	// Model
	template< typename Type=Model > Type* create_named_model( const char_t* name ) { return model_manager_.create_named< Type >( name ); }
	Model* load_model( const char_t* name );
	Model* clone_model( const Model* );

	/**
	 * モデルとシェーダーを複製する（メッシュ・線・アニメーションは共有する）
	 *
	 * load_model が返すモデルとそのシェーダーは、同じ種類の ActiveObject すべてで共有されている。
	 * 1 つの ActiveObject だけシェーダーのテクスチャやパラメータを変えたいときは、これで複製したモデルを持たせる
	 */
	Model* clone_model_with_shaders( const Model* );
	
	template< typename Type=Model > Type* get_model( const char_t* name ) { return model_manager_.get< Type >( name ); }

	/**
	 * モデルのハンドル
	 *
	 * 長く持つ側（段階 1 からのコンポーネント）は生ポインタではなくハンドルを持ち、使うたびに resolve_model で引く。
	 * 解放（unload_named_model）の後は nullptr を返す
	 */
	ModelHandle get_model_handle( const core::graphics::Model* m ) const { return model_manager_.handle_of( m ); }	///< 管理しているモデル（clone_model の結果など）
	Model* resolve_model( ModelHandle h ) const override { return static_cast< Model* >( model_manager_.resolve( h ) ); }

	/// 名前で作ったモデル・メッシュを外す（どの名前からも指されなくなれば解放する。ハンドルは nullptr を返すようになる）。作った側が、使い終わったときに呼ぶ
	void unload_named_model( const char_t* name ) { model_manager_.remove_named( name ); }
	void unload_named_mesh( const char_t* name ) { mesh_manager_.remove_named( name ); }

	/// create_shader で作ったシェーダーを解放する（作った側が、使い終わったときに呼ぶ）
	void destroy_shader( Shader* shader ) { get_shader_manager().release( shader ); }
	
	// Mesh
	template< typename Type=Mesh > Type* create_named_mesh( const char_t* name, Mesh::Buffer::Type type )
	{
		Mesh::Buffer* b = create_mesh_buffer( type );
		Type* m = mesh_manager_.create_named< Type >( name, b );
		b->set_mesh( m );
		
		return m;
	}

	Mesh* get_mesh( const char_t* name ) { return mesh_manager_.get< Mesh >( name ); }

	// SkinningAnimationSet
	SkinningAnimationSet* create_skinning_animation_set() { return skinning_animation_set_manager_.create(); }

	// Texture
	Texture* load_texture( const char_t* file_name ) override { return load_named_texture( file_name, file_name ); }
	// 読み込みに失敗すると例外（common::exception< std::string >。std::exception ではない）を投げる。nullptr は返らない
	Texture* load_named_texture( const char_t* name, const char_t* file_name );
	Texture* get_texture( const char_t* name );

	/// 名前のテクスチャのハンドル（無ければ空）・ハンドルから引く（解放済みなら nullptr）
	TextureHandle get_texture_handle( const char_t* name ) const { return texture_manager_.get_handle( name ); }
	Texture* resolve_texture( TextureHandle h ) const { return texture_manager_.resolve( h ); }

	/// UI 用の画像を名前付きで読み込む（乗算済みアルファ・ミップなし。詳細は Direct3D11::load_texture_premultiplied）。失敗すると例外
	Texture* load_named_texture_premultiplied( const char_t* name, const char_t* file_name );

	/**
	 * load_named_texture_premultiplied の別スレッド版（設計上の問題 B23）
	 *
	 * 画像の展開は作業スレッドで行い、テクスチャの作成・登録と on_loaded の呼び出しはメインスレッド
	 * （毎フレームの dispatch_async_loads の中）で行う。読み込み済みならすぐ on_loaded を呼ぶ。失敗したら nullptr を渡す。
	 *
	 * まだ使う側が無い（呼んでいる所は無い）。シーンの読み込みを非同期にする後続の作業で使う。
	 */
	void load_named_texture_premultiplied_async( const char_t* name, const char_t* file_name, std::function< void( Texture* ) > on_loaded );

	/// ゲームの物体の描画の部品を持つ EntityManager を渡す（GameMain が組み立てのときに呼ぶ）
	void set_entity_manager( core::ecs::EntityManager* m ) { entity_manager_ = m; }

	/// 時間の管理を渡す（GameMain が組み立てのときに呼ぶ）
	void set_time_manager( const core::TimeManager* m ) { time_manager_ = m; }

	/**
	 * シェーダーを作る（シェーダーのコンストラクタの先頭に自分を渡す）
	 *
	 * core::graphics::GraphicsManager の同名の関数を隠し、blue_sky の GraphicsManager として渡す
	 * （シェーダーは blue_sky の機能を使う。以前は core::Service から取っていた。設計上の問題 B28）
	 */
	template< typename Type, typename ... Args > Type* create_shader( Args ... args ) { return get_shader_manager().create< Type >( this, args ... ); }
	template< typename Type, typename ... Args > Type* create_named_shader( const char_t* name, Args ... args ) { return get_shader_manager().create_named< Type >( name, this, args ... ); }

	/// 今の描画先の大きさ（ビューポート。UI を描くときに、描画先に合わせて写すのに使う）
	virtual float_t get_render_target_width() const = 0;
	virtual float_t get_render_target_height() const = 0;

	/// 別スレッドの読み込みが終わったものを処理する（GameMain が毎フレーム呼ぶ）
	virtual void dispatch_async_loads() = 0;

	/// 別スレッドの読み込みで、まだ終わっていないものの数
	virtual size_t get_async_load_count() const = 0;
	Texture* get_null_texture() { return null_texture_; }

	/// 既定のシェーダー・シャドウマップ用シェーダー・画面全体の矩形を準備する
	void setup_default_shaders();
	virtual void refresh_all_shaders();

	void load_paper_textures();

	// virtual void unload_mesh( const char_t* ) = 0;
	// virtual void unload_mesh_all() = 0;

	// virtual void unload_texture_all() = 0;

	void set_paper_texture_type( int_t ) override;
	void bind_paper_texture() const;

	// RenderSettings
	void set_sky_box( const char_t* ) override = 0;
	void unset_sky_box() override = 0;
	virtual bool is_sky_box_set() const = 0;

	void set_ground( const char_t* ) override = 0;
	virtual void unset_ground() = 0;
	// virtual bool is_ground_set() const = 0;

	void set_ambient_color( const Color& ) override = 0;
	void set_shadow_color( const Color& ) override = 0;
	void set_shadow_paper_color( const Color& ) override = 0;
	void set_drawing_accent( float_t ) override = 0;
	void set_drawing_line_type( int_t ) override = 0;

	virtual void set_eye_position( const Vector& ) = 0;

	/**
	 * 平行光源の向き（光が進む向き）を設定する。次の setup_rendering から使う
	 *
	 * 長さは問わない（正規化して使う）。長さ 0 の向きは無視する。既定は ( -1, -2, 0.5 )
	 *
	 * @return 設定したら true
	 */
	bool set_light_direction( const Vector& direction ) { return light_direction_.set( direction ); }

	bool_t is_shadow_enabled() const { return static_cast< bool_t >( shadow_map_ ); }
	ShadowMap* get_shadow_map() { return shadow_map_.get(); }
	const ShadowMap* get_shadow_map() const { return shadow_map_.get(); }

	virtual void resolve_depth_texture() const = 0;
	virtual Texture* get_depth_texture() const = 0;
	virtual Sprite* get_sprite() const = 0;

	virtual void clear_default_view( const Color& = Color::Black ) = 0;
	virtual void unset_render_target() = 0;


	virtual GameConstantBuffer* get_game_render_data() const = 0;
	virtual FrameConstantBuffer* get_frame_render_data() const = 0;
	virtual FrameDrawingConstantBuffer* get_frame_drawing_render_data() const = 0;
	virtual ObjectConstantBuffer* get_shared_object_render_data() const = 0;

	virtual void set_current_object_constant_buffer( const ConstantBuffer* r ) const override { current_object_constant_buffer_ = r; }
	virtual void set_current_skinning_constant_buffer( const ConstantBuffer* r ) const override { current_skinning_constant_buffer_ = r; }
	
	const ConstantBuffer* get_current_object_constant_buffer() const { return current_object_constant_buffer_; }
	const ConstantBuffer* get_current_skinning_constant_buffer() const { return current_skinning_constant_buffer_; }

	virtual void set_depth_stencil() const = 0;
	virtual void unset_depth_stencil() const = 0;

	virtual void setup_rendering() override;
	
	virtual void render_shadow_map() const;
	/// ゲームの物体を描く（メッシュ → 線。描く対象は ecs::ObjectRenderDataSystem::get_render_targets の順）
	virtual void render_objects() const;
	virtual void render_objects_velocity() const;	///< モーションベクトルパス ( 案B / 段階0b )

	virtual void set_post_effect_shader( Shader* ) override;
	virtual Shader* get_post_effect_shader() const override;
	virtual void render_post_effect( Texture* ) override;
	virtual void render_post_effect( Texture*, RenderTarget* ) override;

	Fader* get_fader() { return fader_.get(); }
	const Fader* get_fader() const { return fader_.get(); }

	void set_fade_color( const Color& );

	void start_fade_in( float_t speed );
	void start_fade_out( float_t speed );

	void fade_in( float_t speed = 1.f );
	void fade_out( float_t speed = 1.f );

	void render_fader() const override;

	/**
	 * 矩形内にテキストを描画する
	 *
	 * @param left   描画矩形の左端（ピクセル）
	 * @param top    描画矩形の上端（ピクセル）
	 * @param right  描画矩形の右端（ピクセル）
	 * @param bottom 描画矩形の下端（ピクセル）
	 * @param text   描画するテキスト
	 * @param color  テキスト色（TextStyle 版では h_align / v_align で配置を指定可能）
	 *
	 * Color 版はデフォルト配置（LEFT+TOP）で描画する。
	 * 配置を変えたい場合は TextStyle 版を使用すること。
	 */
	virtual void draw_text( float_t left, float_t top, float_t right, float_t bottom, const char_t* text, const Color& color ) const = 0;
	virtual void draw_text( float_t left, float_t top, float_t right, float_t bottom, const char_t* text, const core::graphics::TextStyle& style ) const = 0;
	virtual void draw_text( float_t left, float_t top, float_t right, float_t bottom, const wchar_t* text, const core::graphics::TextStyle& style ) const = 0;

	/// 文字を積むだけで、まだ描かない（flush_text で描く）。続けて描く文字列を 1 回の描画にまとめる
	virtual void queue_text( float_t left, float_t top, float_t right, float_t bottom, const char_t* text, const core::graphics::TextStyle& style ) const = 0;

	/// 積んだ文字を描く
	virtual void flush_text() const = 0;

	/// 開発用 : 文字のアトラス（MSDF）を PNG に保存する
	virtual bool save_glyph_atlas( const wchar_t* /* file_name */ ) const { return false; }

	/// 画面中央にテキストを描画する（ローディング表示等）
	virtual void draw_text_at_center( const char_t*, const Color& ) const = 0;

	virtual float_t get_text_height( const char_t* text, float_t width, float_t height ) const = 0;
	virtual float_t get_text_height( const wchar_t* text, float_t width, float_t height ) const = 0;

	/**
	 * テキストの描画幅を測る（ピクセル）
	 *
	 * @param text 文字列（UTF-8）。最初の改行までを測る
	 * @param font_size フォントサイズ（ピクセル）。0 以下ならデフォルトサイズ
	 * @return 描画幅。フォント未準備なら 0
	 */
	virtual float_t measure_text_width( const char_t* text, float_t font_size ) const = 0;

	/// テキストの 1 行の高さ（ピクセル）。font_size が 0 以下ならデフォルトサイズ
	virtual float_t measure_text_line_height( float_t font_size ) const = 0;

	/// TextStyle::font_size が 0 のときに使われるフォントサイズ（ピクセル）
	virtual float_t get_default_font_size() const = 0;

	void set_debug_axis_enabled( bool e ) { is_debug_axis_enabled_ = e; }

	void render_debug_axis() const;

	/**
	 * メインカメラの情報を設定する
	 * 
	 * @param transform 位置・回転情報
	 * @param camera カメラ情報 ( FOV など )
	 */
	void set_main_camera_info( const TransformComponent* transform, const CameraComponent* camera )
	{
		main_camera_transform_component_ = transform;
		main_camera_component_ = camera;
	}

	/**
	 * カメラの位置を取得する
	 */
	Vector get_camera_position() const override
	{
		if ( main_camera_transform_component_ && main_camera_component_ )
		{
			return main_camera_transform_component_->transform.get_position() + main_camera_component_->eye_offset;
		}
		return Vector( 0.f, 0.f, 0.f );
	}

	/**
	 * カメラの前方ベクトルを取得する
	 */
	Vector get_camera_forward() const override
	{
		if ( main_camera_transform_component_ )
		{
			return main_camera_transform_component_->transform.forward();
		}
		return Vector( 0.f, 0.f, 1.f );
	}

	uint_t get_pass_count() const { return pass_count_; }

	void count_draw() const override { draw_count_++; }
	uint_t get_draw_count() const { return draw_count_; }

	virtual void reload_font( const char* font_path ) = 0;

	void debug_print_resources() const;

	// void update_frame_render_data( const Camera*, const Vector3& ) const;

}; // class GraphicsManager

} // namespace blue_sky::graphics
