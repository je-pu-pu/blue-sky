#include "Model.h"
#include <blue_sky/graphics/GraphicsManager.h>
#include <core/graphics/GraphicsManager.h>
#include <blue_sky/graphics/shader/FlatShader.h>
#include <boost/filesystem.hpp>

namespace blue_sky::graphics
{

/**
 * このモデル用のシェーダーを作成する
 * 
 * 影の有無とスキニングの有無でシェーダーを選ぶ（モデルごとに別のシェーダーを使いたいときは、読み込んだ後で set_shader_at で差し替える）
 */
Model::Shader* Model::create_shader( GraphicsManager* graphics_manager ) const
{
	if ( graphics_manager->is_shadow_enabled() )
	{
		if ( is_skin_mesh() )
		{
			return graphics_manager->create_shader< shader::FlatShadowShader >( "skin", "skin_with_shadow" );
		}
		else
		{
			return graphics_manager->create_shader< shader::FlatShadowShader >( "main", "main_with_shadow" );
		}
	}
	else
	{
		if ( is_skin_mesh() )
		{
			return graphics_manager->create_shader< shader::FlatShader >( "skin", "flat_skin" );
		}
		else
		{
			return graphics_manager->create_shader< shader::FlatShader >( "main", "flat" );
		}
	}
}

bool Model::get_cull_sphere( Vector3& center, float_t& radius ) const
{
	// アニメーションする物は頂点が読んだ形の外へ出るので、カリングしない（プレイヤー・敵・少女など。数は少ない）
	if ( is_skin_mesh() )
	{
		return false;
	}

	LocalBounds bounds;

	if ( mesh_ )
	{
		if ( mesh_->get_bounds().empty )
		{
			return false;	// 頂点から箱を求めていない（読み込み以外で作ったメッシュ）
		}

		bounds.add( mesh_->get_bounds() );
	}

	float_t line_margin = 0.f;

	if ( line_ )
	{
		bounds.add( line_->get_bounds() );
		line_margin = line_->get_max_segment_length() * 2.f;
	}

	if ( bounds.empty )
	{
		return false;
	}

	center = bounds.center();
	radius = bounds.radius() * 1.1f + 0.05f + line_margin;

	return true;
}

/**
 * 指定したテクスチャ名に対応するテクスチャファイルパスを取得する
 *
 * @param texture_name テクスチャ名
 * @return テクスチャファイルパス
 */
string_t Model::get_texture_file_path_by_texture_name( const char_t* name ) const
{
	string_t file_path = name;
	boost::filesystem::path path( file_path );
	
	if ( ! path.has_extension() )
	{
		file_path += ".png";
	}

	if ( ! path.has_parent_path() )
	{
		file_path = string_t( "media/model/" ) + file_path;
	}

	return file_path;
}

/**
 * モデル自身に設定されているシェーダーでモデルをレンダリングする
 *
 */
void Model::render() const
{
	get_mesh()->bind();

	for ( uint_t n = 0; n < get_shader_count(); n++ )
	{
		get_shader_at( n )->update();
		get_shader_at( n )->bind();

		get_shader_at( n )->render( get_mesh(), n );
	}
}

} // namespace blue_sky::graphics
