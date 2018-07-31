#include "Model.h"
#include <GameMain.h>
#include <blue_sky/graphics/shader/FlatShader.h>
#include <boost/filesystem.hpp>

namespace blue_sky::graphics
{

Model::Shader* Model::create_shader() const
{
	if ( GameMain::get_instance()->get_graphics_manager()->is_shadow_enabled() )
	{
		if ( is_skin_mesh() )
		{
			return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::FlatShader >( "skin", "skin_with_shadow" );
		}
		else
		{
			return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::FlatShader >( "main", "main_with_shadow" );
		}
	}
	else
	{
		if ( is_skin_mesh() )
		{
			return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::FlatShader >( "skin", "flat_skin" );
		}
		else
		{
			return GameMain::get_instance()->get_graphics_manager()->create_shader< shader::FlatShader >( "main", "flat" );
		}
	}
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
		get_shader_at( n )->render( get_mesh(), n );
	}
}

/**
 * 指定したシェーダーでモデルをレンダリングする
 *
 * @param shader シェーダー
 */
void Model::render( const Shader* shader ) const
{
	get_mesh()->bind();

	for ( uint_t n = 0; n < get_shader_count(); n++ )
	{
		shader->render( get_mesh(), n );
	}
}

} // namespace blue_sky::graphics
