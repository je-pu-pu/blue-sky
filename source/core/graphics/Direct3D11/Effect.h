#pragma once

#include "EffectPass.h"

#include <core/graphics/EffectParser.h>

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
	class EffectTechnique;

/**
 * effect ファイル（.fx）を読んで technique を作る
 *
 * 以前は Microsoft の Effect ライブラリ（FX11。開発が止まっている）を使っていた。今は自前で読む :
 * 1. ファイルをプリプロセスし（#include を展開）、EffectParser で technique / pass / 状態のブロックを読む
 * 2. 各パスのシェーダーを、同じソースから D3DCompile でコンパイルする（関数名＋引数＋プロファイルが同じものは共有する）
 * 3. 状態のブロックから D3D11 の状態オブジェクトを作る
 * 4. サンプラーは hlsl に register を書いていないので、シェーダーのリフレクションで割り当てられたスロットを調べ、apply() で結び付ける
 */
class Effect
{
public:
	using Direct3D			= Direct3D11;

	using Technique			= EffectTechnique;
	using Pass				= EffectPass;

	/// technique の名前 → technique（キーは文字列。以前はポインタの値をキーにしていて、同じ名前でも別のエントリになっていた）
	using TechniqueList		= std::unordered_map< std::string, std::unique_ptr< Technique > >;

	/// technique の名前 → パスごとの設定（読み直しのたびに作り直す）
	using TechniqueSetupList = std::unordered_map< std::string, std::vector< EffectPassSetup > >;

private:
	Direct3D*			direct_3d_;
	bool				loaded_ = false;

	TechniqueSetupList	technique_setup_list_;
	TechniqueList		technique_list_;

public:
	explicit Effect( Direct3D* );
	~Effect();

	/**
	 * エフェクトファイルを読む（読み直しにも使う）
	 *
	 * コンパイルに成功してから差し替える。失敗したら例外を投げ、それまでのエフェクトをそのまま使い続ける。
	 * 取得済みの technique のオブジェクトは残し、中身だけを新しいエフェクトのものに入れ替えるので、
	 * 描画側が持っているポインタは読み直しの後も有効。
	 */
	void load( const char* );
	void clear();

	/// technique を名前で引く。無ければ nullptr（"|main" のようなグループ付きの名前は | の後ろを使う）
	Technique* get_technique( const char* name );

	/// effect ファイルをプリプロセスする（#include を展開し、コメントを除く）。失敗したら例外
	static std::string preprocess( const char* file_path );

	/// シェーダーを 1 つコンパイルする（デバイスは使わない）。失敗したら nullptr を返し、message に理由を入れる。成功しても警告があれば message に入る
	static Microsoft::WRL::ComPtr< ID3DBlob > compile_shader( const std::string& source, const char* source_name, const effect::ShaderRef&, std::string& message );

}; // class Effect

} // namespace namespace core::graphics::direct_3d_11
