#pragma once

#include <blue_sky/graphics/Model.h>

namespace blue_sky::graphics
{

/**
 * SkyBox
 *
 */
class SkyBox : public Model
{
public:
	/// スカイボックスのテクスチャを登録する名前の接頭辞（"front" などの汎用的な名前が他のテクスチャと衝突しないように）
	static constexpr const char_t* texture_name_prefix = "skybox/";

	/// スカイボックスが使うテクスチャ名（面の名前）
	static constexpr const char_t* face_name_list[] = { "front", "back", "top", "bottom", "left", "right" };

	/// 面の名前から、テクスチャを登録する名前を作る
	static string_t get_texture_resource_name_of( const char_t* face_name ) { return string_t( texture_name_prefix ) + face_name; }

private:
	string_t name_;		///< 以前は const char* で持っていて、呼ぶ側の文字列が消えると無効になり得た

protected:
	Shader* create_shader( GraphicsManager* graphics_manager ) const override;
	string_t get_texture_file_path_by_texture_name( const char* ) const override;
	string_t get_texture_resource_name( const char* texture_name ) const override { return get_texture_resource_name_of( texture_name ); }

public:
	explicit SkyBox( const char* );
	~SkyBox() override;

}; // class SkyBox

} // namespace blue_sky::graphics
