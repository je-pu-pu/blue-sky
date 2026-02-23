/**
 * @file MsdfTextRenderer.h
 * @brief MSDF (Multi-channel Signed Distance Field) 方式のテキスト描画エンジン
 *
 * MSDF フォントレンダリングのメインクラス。
 * FreeType でフォントを読み込み、msdfgen で距離フィールドを生成し、
 * D3D11 でスクリーン上にテキストを描画する。
 *
 * @par 描画パイプライン
 * 1. draw_text() で文字列を走査し、各グリフのクワッド（矩形）を頂点バッファに蓄積
 * 2. flush() で蓄積した全クワッドを一括描画（バッチレンダリング）
 * 3. MSDF シェーダーが距離フィールドからアンチエイリアス付きの輪郭を再構成
 *
 * @par 座標系
 * - draw_text() の x, y はスクリーン左上原点のピクセル座標
 * - 内部で NDC (Normalized Device Coordinates) に変換して描画
 * - フォントメトリクスは EM 正規化座標（1 EM = 1.0）で管理
 * - font_size (ピクセル) × EM 正規化値 = 実際のスクリーンピクセル
 */
#pragma once

#include "MsdfFont.h"
#include "GlyphCache.h"
#include "GlyphAtlas.h"
#include <core/DirectX.h>
#include <core/graphics/TextStyle.h>
#include <d3d11.h>
#include <memory>
#include <vector>
#include <string>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
	class InputLayout;
	class EffectTechnique;
	template< typename, int > class ConstantBufferTyped;
}

namespace core::graphics {

/**
 * @brief MSDF テキスト描画エンジン
 *
 * フォント読み込み、グリフキャッシュ、アトラス管理、バッチ描画を統括する。
 * draw_text() → flush() のパターンで使用する。
 */
class MsdfTextRenderer {
public:
	/**
	 * @brief テキスト描画用の頂点データ
	 *
	 * "sprite" InputLayout に対応。position は NDC 座標、
	 * tex_coord はアトラス上の UV 座標。
	 */
	struct Vertex {
		Vector3 position;	///< NDC 座標 (x, y, z=0)
		Vector2 tex_coord;	///< アトラス UV 座標
		Color color;		///< 頂点カラー（現在は White 固定、シェーダー側で text_color を使用）
	};

	/**
	 * @brief MSDF シェーダー用の定数バッファ（ピクセルシェーダー slot 3）
	 *
	 * シェーダーが距離フィールドからテキスト/アウトラインを描画するために必要なパラメータ。
	 */
	struct MsdfConstantBufferData {
		Color text_color;				///< テキスト本体の色
		Color outline_color;			///< アウトラインの色
		float outline_width = 0.f;		///< アウトラインの太さ（0 = アウトラインなし）
		float px_range = 4.f;			///< MSDF の距離フィールド範囲（ピクセル単位、GlyphCache の msdf_range_ * 2 と一致）
		float atlas_texel_size = 0.f;	///< アトラステクスチャの 1 テクセルサイズ (1.0 / ATLAS_SIZE)
		float padding_1 = 0.f;			///< 16 バイトアラインメント用パディング
	};

	/**
	 * @brief 射影変換用の定数バッファ（頂点シェーダー slot 13）
	 *
	 * 正射影行列を格納し、NDC 座標でのクワッド描画に使用する。
	 */
	struct TransformConstantBufferData {
		Matrix transform;	///< 正射影行列
	};

	using MsdfConstantBuffer = direct_3d_11::ConstantBufferTyped< MsdfConstantBufferData, 3 >;		///< PS slot 3
	using TransformConstantBuffer = direct_3d_11::ConstantBufferTyped< TransformConstantBufferData, 13 >;	///< VS slot 13

private:
	direct_3d_11::Direct3D11* direct_3d_;	///< D3D11 デバイスラッパー（外部所有）

	std::unique_ptr< MsdfFont > font_;			///< FreeType フォントラッパー
	std::unique_ptr< GlyphAtlas > atlas_;		///< MSDF ビットマップを格納するテクスチャアトラス
	std::unique_ptr< GlyphCache > cache_;		///< コードポイント → CachedGlyph の LRU キャッシュ

	std::unique_ptr< MsdfConstantBuffer > msdf_constant_buffer_;			///< MSDF シェーダーパラメータ
	std::unique_ptr< TransformConstantBuffer > transform_constant_buffer_;	///< 射影変換行列

	ID3D11Buffer* vertex_buffer_;	///< 動的頂点バッファ（D3D11_USAGE_DYNAMIC）
	ID3D11Buffer* index_buffer_;	///< 動的インデックスバッファ（D3D11_USAGE_DYNAMIC）

	const direct_3d_11::InputLayout* input_layout_;			///< "sprite" 入力レイアウト
	const direct_3d_11::EffectTechnique* effect_technique_;	///< "msdf_text" エフェクトテクニック

	std::vector< Vertex > vertices_;	///< flush() まで蓄積される頂点データ
	std::vector< uint16_t > indices_;	///< flush() まで蓄積されるインデックスデータ

	static constexpr int MAX_CHARS = 1024;		///< 1 回の flush() で描画可能な最大文字数
	static constexpr int ATLAS_SIZE = 2048;		///< アトラステクスチャのピクセルサイズ（2048x2048）
	static constexpr int CELL_SIZE = 48;		///< アトラスの 1 セルのピクセルサイズ（48x48）
	static constexpr float MSDF_RANGE = 4.f;	///< MSDF の距離フィールド範囲（ピクセル単位）

	/** @brief 動的頂点バッファを作成する */
	void create_vertex_buffer();

	/** @brief 動的インデックスバッファを作成する */
	void create_index_buffer();

	/** @brief スクリーンサイズから正射影行列を計算して定数バッファを更新する */
	void update_transform();

	/**
	 * @brief 1 文字分のクワッド（2 三角形 = 4 頂点 + 6 インデックス）を蓄積する
	 *
	 * ピクセル座標を NDC に変換して頂点バッファに追加する。
	 * MAX_CHARS を超える場合は無視される。
	 *
	 * @param x クワッド左上の X ピクセル座標
	 * @param y クワッド左上の Y ピクセル座標
	 * @param w クワッドの幅（ピクセル）
	 * @param h クワッドの高さ（ピクセル）
	 * @param uv アトラス上のテクスチャ UV 座標
	 * @param color 頂点カラー
	 */
	void append_quad( float x, float y, float w, float h, const UVRect& uv, const Color& color );

public:
	/**
	 * @brief コンストラクタ: フォント読み込み、アトラス・キャッシュ・バッファの初期化
	 *
	 * @param direct_3d D3D11 デバイスラッパー
	 * @param font_path フォントファイルパス (.ttf / .otf)
	 */
	MsdfTextRenderer( direct_3d_11::Direct3D11* direct_3d, const char* font_path );
	~MsdfTextRenderer();

	/** @brief フォント読み込みとシェーダーの準備が完了しているか */
	bool is_ready() const { return font_ && font_->is_loaded() && effect_technique_; }

	/**
	 * @brief テキストを描画キューに追加する（wchar_t 版）
	 *
	 * 文字列を 1 文字ずつ走査し、グリフキャッシュから取得してクワッドを蓄積する。
	 * 実際の描画は flush() で行われる。改行 (\n) に対応。
	 *
	 * @param x 描画開始位置の X ピクセル座標（スクリーン左端が 0）
	 * @param y 描画開始位置の Y ピクセル座標（スクリーン上端が 0）
	 * @param text 描画するテキスト（null 終端）
	 * @param style テキストスタイル（フォントサイズ、色、アウトライン等）
	 */
	void draw_text( float x, float y, const wchar_t* text, const TextStyle& style );

	/** @brief テキストを描画キューに追加する（UTF-8 版、内部で wchar_t に変換） */
	void draw_text( float x, float y, const char* text, const TextStyle& style );

	/**
	 * @brief テキストの描画幅を計算する（改行前の最初の行のみ）
	 *
	 * @param text 計測するテキスト
	 * @param font_size フォントサイズ（ピクセル）
	 * @return テキスト幅（ピクセル）
	 */
	float measure_text_width( const wchar_t* text, float font_size );

	/** @brief テキストの描画幅を計算する（UTF-8 版） */
	float measure_text_width( const char* text, float font_size );

	/**
	 * @brief テキストの描画高さを計算する（改行を含む全行）
	 *
	 * @param text 計測するテキスト
	 * @param font_size フォントサイズ（ピクセル）
	 * @return テキスト高さ（ピクセル）= lineHeight * 行数
	 */
	float measure_text_height( const wchar_t* text, float font_size );

	/** @brief テキストの描画高さを計算する（UTF-8 版） */
	float measure_text_height( const char* text, float font_size );

	/**
	 * @brief 蓄積した全クワッドを一括描画する
	 *
	 * 頂点/インデックスバッファを GPU に転送し、MSDF シェーダーで描画する。
	 * 描画後、内部バッファはクリアされる。
	 *
	 * @note pass->apply() の後にアトラステクスチャをバインドすること。
	 *       apply() がシェーダーリソースのステートをリセットするため。
	 */
	void flush();
};

} // namespace core::graphics
