#pragma once

#include <core/DirectX.h>
#include <core/type.h>
#include <core/graphics/Pixel2DSpace.h>

#include <d3d11.h>

#include <unordered_map>
#include <vector>
#include <memory>

namespace core::graphics
{
	class Sprite;
	class GraphicsManager;
}

namespace core::graphics::direct_3d_11
{
	class Texture;
	class RenderTargetTexture;
	class BackBufferTexture;

	class InputLayout;

	class Effect;
	class EffectTechnique;
	class EffectPass;

	class Sprite;

/**
 * Direct3D 11 のラッパークラス
 *
 */
class Direct3D11
{
public:
	using Texture			= core::graphics::direct_3d_11::Texture;

	using InputLayout		= core::graphics::direct_3d_11::InputLayout;
	using InputLayoutList	= std::unordered_map< string_t, std::unique_ptr< InputLayout > >;

	using Effect			= core::graphics::direct_3d_11::Effect;
	using EffectTechnique	= core::graphics::direct_3d_11::EffectTechnique;
	using EffectPass		= core::graphics::direct_3d_11::EffectPass;

	using Sprite			= core::graphics::Sprite;

	using Vector			= core::math::direct_x_math::Vector;
	using Matrix			= core::math::direct_x_math::Matrix;
	using Color				= core::math::direct_x_math::Color;

private:
	static const Color DEFAULT_CLEAR_COLOR;

	com_ptr< IDXGIAdapter1 >	dxgi_adapter_;

	/// @todo スマートポインタ ( com_ptr ) 化する？
	ID3D11Device*				device_;				///< Direct3D 11 Device
	ID3D11DeviceContext*		immediate_context_;		///< Direct3D 11 Device Context
	IDXGISwapChain*				swap_chain_;			///< Direct3D 11 Swap Chain

	DXGI_SWAP_CHAIN_DESC		swap_chain_desc_;		///< 画面の大きさ・形式と、画面への描画の MSAA（スワップチェーン自体は MSAA なし）
	UINT						swap_chain_buffer_count_ = 1;	///< スワップチェーンのバッファの枚数

	std::unique_ptr< BackBufferTexture >	back_buffer_texture_;		///< バックバッファへの描画用テクスチャ

	ID3D11Texture2D*			depth_stencil_texture_;
	ID3D11DepthStencilView*		depth_stencil_view_;
	std::unique_ptr< Texture >	depth_texture_;

	InputLayoutList				input_layout_list_;

	D3D11_VIEWPORT				viewport_;

	float_t						offset_2d_x_ = 0.f;		///< 2D 描画の立体視のずらし（Sprite と文字で共有する）
	bool						vsync_ = false;			///< 垂直同期を待って表示する

	std::unique_ptr< Sprite >	sprite_;
	std::unique_ptr< Effect >	effect_;

	void create_device();
	
	void create_swap_chain( IDXGIFactory1*, HWND, uint_t, uint_t, bool, int, int );
	
	void create_back_buffer_texture();

	void create_depth_stencil_view();
	void create_depth_texture();

	void setup_default_viewport();

	void log_all_adapter_desc( IDXGIFactory1* );
	void log_adapter_desc( int, const DXGI_ADAPTER_DESC1& );
	void log_feature_level();

public:
	Direct3D11( HWND, int, int, bool, int = 0, int = 0 );
	~Direct3D11();

	// 以前あった get_instance()（隠れた大域アクセス）は削除した。使う側は作り手から Direct3D11* を受け取る

	/// OculusRift で使用
	ID3D11RenderTargetView* create_render_target_view( ID3D11Texture2D* );
	ID3D11DepthStencilView* create_depth_stencil_view( ID3D11Texture2D* );

	/// スプライトを作る。graphics_manager は描画回数を数える先（作り手の GraphicsManager が自分を渡す）
	void setup_sprite( const core::graphics::GraphicsManager* graphics_manager );

	void create_default_input_layout();
	void create_input_layout( const char_t*, const char_t*, D3D11_INPUT_ELEMENT_DESC[], UINT );

	struct DisplayMode
	{
		int width;
		int height;
	};

	int get_width() const { return swap_chain_desc_.BufferDesc.Width; }
	int get_height() const { return swap_chain_desc_.BufferDesc.Height; }

	/**
	 * 2D 描画（Sprite・文字）の座標系
	 *
	 * Sprite と文字の描画は必ずここから座標系を得る（以前はそれぞれが同じ変換を書いていた）。
	 * 大きさは今の描画先（ビューポート）の大きさ（VR の目ごとのバッファなどにも正しく描ける。設計上の問題 B16）。
	 * ビューポートが設定されていなければスワップチェーンの大きさ。
	 */
	Pixel2DSpace get_2d_space() const;

	/// 2D 描画の立体視のずらし（正射影の後の座標で）
	void set_2d_offset_x( float_t x ) { offset_2d_x_ = x; }

	/// システムが対応する表示解像度のリストを返す (重複除去・ソート済み)
	std::vector< DisplayMode > get_available_display_modes() const;

	void set_size( int, int );

	/// 垂直同期を待って表示するか（以前は選べず、常に待たなかった）
	void set_vsync( bool v ) { vsync_ = v; }
	bool is_vsync() const { return vsync_; }
	void set_full_screen( bool );

	bool is_full_screen() const;
	void switch_full_screen();

	void on_resize( int, int );

	void clear_default_view( const Color& = DEFAULT_CLEAR_COLOR );

	void clear_back_buffer_view( const Color& );
	void clear_depth_stencil_view();

	void clear_render_target_view( ID3D11RenderTargetView*, const Color& = DEFAULT_CLEAR_COLOR );
	void clear_depth_stencil_view( ID3D11DepthStencilView* );

	void set_default_render_target( bool = true );
	void set_default_viewport();

	/**
	 * 描画の状態を既定に戻す（設計上の問題 B18）
	 *
	 * ブレンド・深度・ラスタライザを既定に、ジオメトリ／ハル／ドメインシェーダーを外し、
	 * シェーダーに結び付けたテクスチャを外し、描画先とビューポートを画面に戻す。
	 * 以前はフレームの頭でも初期化せず、前の描画（前のフレームを含む）の状態に依存していた。
	 * フレームの頭と、オーバーレイ（UI）を描く前に呼ぶ。
	 */
	void reset_render_state();

	/// バックバッファ（今のフレームの描画結果）を PNG に保存する（開発用。画面がロック中でも描画結果を確かめられる）
	bool save_back_buffer_to_png( const wchar_t* file_name );

	void set_render_target( RenderTargetTexture* );
	void set_render_target( BackBufferTexture* );

	void set_render_target_for_vr( ID3D11RenderTargetView*, ID3D11DepthStencilView* );

	void unset_render_target();

	void set_viewport( float_t, float_t, float_t, float_t, float_t = 0.f, float_t = 1.f );

	const InputLayout* get_input_layout( const char* ) const;
	void set_input_layout( const char* );
	void set_input_layout( const InputLayout* );

	[[nodiscard]] Texture* load_texture( const char* );

	/**
	 * UI 用の画像を読み込む（失敗したら例外）
	 *
	 * - 乗算済みアルファ（RGB×A）に変換する。線形補間で透明部分の色が縁ににじまない
	 * - ミップマップは作らない（縮小時に 1 段粗い画像と混ざってぼけたり、アトラスの隣の部品が混ざったりしない）
	 * - 色空間のメタデータ（PNG の sRGB チャンクなど）は無視して常に UNORM で作る（書き出したツールで明るさが変わらない）
	 *
	 * 描画は乗算済みアルファ用の technique（sprite_premultiplied）で行うこと。
	 */
	[[nodiscard]] Texture* load_texture_premultiplied( const char* );

	/// 展開した画像（乗算済みアルファの RGBA8）
	struct DecodedImage
	{
		unsigned int width = 0;
		unsigned int height = 0;
		std::vector< uint8_t > pixels;
	};

	/**
	 * 画像ファイルを読み込み、乗算済みアルファの RGBA8 に展開する（失敗したら例外）
	 *
	 * GPU に触れないので、どのスレッドからでも呼べる（COM を初期化したスレッドであること）。
	 * 別スレッドでの読み込み（AsyncLoader）では、これを作業スレッドで、create_texture_from_image をメインスレッドで呼ぶ。
	 */
	static DecodedImage decode_image_premultiplied( const char* file_path );

	/// 展開した画像からテクスチャを作る（ミップなし・UNORM）
	[[nodiscard]] Texture* create_texture_from_image( const DecodedImage& image, const char* name_for_error );

	void bind_texture_to_ds( uint_t, const Texture* );
	void bind_texture_to_ps( uint_t, const Texture* );


	void present();

	inline Sprite* get_sprite() { return sprite_.get(); }
	inline Effect* get_effect() { return effect_.get(); }

	inline const Sprite* get_sprite() const { return sprite_.get(); }
	inline const Effect* get_effect() const { return effect_.get(); }

	BackBufferTexture* get_back_buffer_texture() { return back_buffer_texture_.get(); }

	void resolve_depth_texture();
	Texture* get_depth_texture() { return depth_texture_.get(); }

	/** BAD functions */
	inline ID3D11Device* getDevice() const { return device_; }
	inline ID3D11DeviceContext* getImmediateContext() const { return immediate_context_; }

}; // class Direct3D11

} // namespace core::graphics::direct_3d_11
