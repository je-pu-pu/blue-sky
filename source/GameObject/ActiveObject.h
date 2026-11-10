#pragma once

#include "GameObject.h"
#include <blue_sky/ConstantBuffers.h>
#include <common/exception.h>
#include <memory>

namespace core
{
	class AnimationPlayer;

	namespace graphics
	{
		class Shader;
		class Texture;
		class ConstantBuffer;
	}
}

namespace blue_sky
{
	namespace graphics
	{
		class Model;
		class GraphicsManager;
	}

/**
 * 行動するオブジェクト
 *
 */
class alignas( 16 ) ActiveObject : public GameObject
{
public:
	using Model				= graphics::Model;
	using Shader			= core::graphics::Shader;
	using Texture			= core::graphics::Texture;
	using AnimationPlayer	= core::AnimationPlayer;

private:
	Vector				front_;				///< 前
	Vector				right_;				///< 右

	Vector				start_location_;	///< スタート時の位置
	Vector				start_rotation_;	///< スタート時の回転 ( Degree )
	float_t				start_direction_degree_;	///< スタート時の向き ( Y Axis )

	Model*				model_;				///< Model (GraphicsManager が所有)
	graphics::GraphicsManager*						graphics_manager_ = nullptr;	///< 定数バッファを作る先（登録する ActiveObjectManager が渡す。以前は定数バッファが Direct3D11 の get_instance() を使っていた）
	std::unique_ptr< const core::graphics::ConstantBuffer >	object_constant_buffer_;	///< 定数バッファ（setup_render_data() で作る）
	std::unique_ptr< AnimationPlayer >				animation_player_;			///< アニメーション再生

	bool				is_dead_;			///< 死亡フラグ
	float_t				flicker_scale_;		///< ゆらぎの大きさ

	float_t				direction_degree_;	///< 方向 ( Y Axis Degree )

	bool				is_mesh_visible_;	///< メッシュを描画するかどうか
	bool				is_line_visible_;	///< ラインを描画するか

	mutable Matrix		prev_world_;		///< 前フレームの world 行列 ( モーションベクトル用 )
	mutable bool		prev_world_valid_ = false;	///< prev_world_ が有効か ( 初回は cur=prev で速度0 )

protected:
	void setup_animation_player();

	virtual void limit_velocity();

	constexpr float get_max_speed() const { return 20.f; }

public:
	ActiveObject();
	ActiveObject( const ActiveObject& o );
	virtual ~ActiveObject() override;

	virtual ActiveObject* clone() const { COMMON_THROW_EXCEPTION_MESSAGE( "clone() not implemented." ); }

	virtual void restart();

	virtual void update() override { }

	virtual void set_model( Model* m ) { model_ = m; setup_animation_player(); }
	Model* get_model() { return model_; }
	const Model* get_model() const { return model_; }

	/**
	 * モデルとシェーダーを、このオブジェクトだけのものにする（メッシュ・線・アニメーションは共有のまま）
	 *
	 * create_object で作ったオブジェクトは、同じ種類のオブジェクトとモデル・シェーダーを共有している。
	 * このオブジェクトだけシェーダーのテクスチャやパラメータを変えたいときに、先に呼ぶ。
	 * まだ ActiveObjectManager に登録していない（複製する先が無い）ときやモデルが無いときは何もしない
	 *
	 * @return 自分だけのものにしたら true
	 */
	bool make_model_unique();

	virtual void action( const string_t& );

	virtual void set_flicker_scale( float_t s ) { flicker_scale_ = s; }
	virtual float_t get_flicker_scale() const { return flicker_scale_; }

	AnimationPlayer* get_animation_player() { return animation_player_.get(); }
	const AnimationPlayer* get_animation_player() const { return animation_player_.get(); }

	void set_start_location( float_t, float_t, float_t );
	void set_start_rotation( float_t, float_t, float_t );
	void set_start_direction_degree( float_t );

	float_t get_direction_degree() const { return direction_degree_; }
	void set_direction_degree( float_t );
	void chase_direction_degree( float_t, float_t );
	void chase_direction_to( const Vector&, float_t );

	const Vector& get_front() const { return front_; }
	const Vector& get_right() const { return right_; }

	Vector& get_start_location() { return start_location_; }
	const Vector& get_start_location() const { return start_location_; }

	virtual void kill();
	
	bool is_dead() const { return is_dead_; }

	/// 描画されるかどうかを返す
	virtual bool is_visible() const { return ( is_mesh_visible() || is_line_visible() ); }

	virtual bool is_mesh_visible() const { return is_mesh_visible_; }
	virtual bool is_line_visible() const { return is_line_visible_; }

	void set_mesh_visible( bool v ) { is_mesh_visible_ = v; }
	void set_line_visible( bool v ) { is_line_visible_ = v; }

	/// 描画の準備をする（定数バッファを作る。作り直しはしない）。ActiveObjectManager に登録するときに呼ばれる
	void setup_render_data( graphics::GraphicsManager* graphics_manager );

	const core::graphics::ConstantBuffer* get_object_constant_buffer() const { return object_constant_buffer_.get(); }

	virtual void update_render_data() const;
	[[deprecated]] virtual void bind_render_data() const;

	virtual void render_mesh() const;
	virtual void render_mesh( const Shader* ) const;
	virtual void render_line() const;

	void play_animation( const char_t* name, bool force, bool loop );

	// virtual void render_line() = 0;

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

}; // class ActiveObject

#define DEFINE_ACTIVE_OBJECT_COMMON_FUNCTIONS \
	ActiveObject* clone() const override { return new std::remove_const< std::remove_reference< decltype( *this ) >::type >::type( * this ); }

} // namespace blue_sky
