#pragma once

#include <core/ResourceHandle.h>
#include <type/type.h>

namespace core::graphics
{

class Mesh;
class Shader;
class Model;

/**
 * モデルを指すハンドル
 *
 * 型は core の Model にしている。管理する側（blue_sky の GraphicsManager）は core の Model の型で管理し、中身は blue_sky のモデル。
 * こうすると core 層（ModelComponent・RenderSystem）も、blue_sky 層を知らずにハンドルを持てる。
 * 引くときは GraphicsManager::resolve_model を使う（解放済みなら nullptr）
 */
using ModelHandle = core::ResourceHandle< Model >;

/**
 * 抽象モデルクラス
 *
 * 以下の情報を保持する
 * * Mesh ( どのような形状を持つか？ )
 * * Shader ( どう描画するか？ )
 * 
 */
class Model
{
public:

public:
	Model() { }
	virtual ~Model() { }

	virtual Mesh* get_mesh() = 0;
	virtual const Mesh* get_mesh() const = 0;
	virtual void set_mesh( Mesh* ) = 0;

	virtual Shader* get_shader_at( size_t ) = 0;
	virtual const Shader* get_shader_at( size_t ) const = 0;
	virtual void set_shader_at( size_t, Shader* ) = 0;

	virtual size_t get_shader_count() const = 0;

	virtual void render() const = 0;
};

} // namespace core::graphics
