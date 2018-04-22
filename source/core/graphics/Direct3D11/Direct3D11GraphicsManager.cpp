#include "Direct3D11GraphicsManager.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11Effect.h"
#include "Direct3D11Fader.h"
#include "Direct3D11.h"

#include <core/graphics/DirectWrite/DirectWrite.h>

#include "ConstantBuffer.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

#include <common/string.h>

namespace blue_sky
{

/**
 * コンストラクタ
 *
 * @param direct_3d Direct3D
 */
Direct3D11GraphicsManager::Direct3D11GraphicsManager( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, game_render_data_( new GameConstantBuffer( direct_3d ) )
	, frame_render_data_( new FrameConstantBuffer( direct_3d ) )
	, frame_drawing_render_data_( new FrameDrawingConstantBuffer( direct_3d ) )
	, shared_object_render_data_( new ObjectConstantBuffer( direct_3d ) )
{

}

/**
 * デストラクタ
 *
 */
Direct3D11GraphicsManager::~Direct3D11GraphicsManager()
{
	
}

/**
 * 更新処理
 *
 */
void Direct3D11GraphicsManager::update()
{
	if ( is_fading_in_ )
	{
		fade_in( fade_speed_ );
	}
	else
	{
		fade_out( fade_speed_ );
	}
}

/**
 * メッシュを生成する
 *
 * @return メッシュ
 */
Direct3D11GraphicsManager::Mesh* Direct3D11GraphicsManager::create_mesh()
{
	return new Direct3D11Mesh( direct_3d_ );
}

/**
 * ラインを生成する
 *
 * @return ライン
 */
Direct3D11GraphicsManager::Line* Direct3D11GraphicsManager::create_line()
{
	return create_drawing_line();
}

/**
 * 手描き風メッシュを生成する
 *
 * @return 手描き風メッシュ
 */
DrawingMesh* Direct3D11GraphicsManager::create_drawing_mesh()
{
	return new DrawingMesh( create_mesh() );
}

/**
 * 手描き風ラインを生成する
 *
 * @return ライン
 */
DrawingLine* Direct3D11GraphicsManager::create_drawing_line()
{
	return new DrawingLine( direct_3d_ );
}

/**
 * テクスチャを読み込む
 *
 * @param name テクスチャ名
 * @param file_path テクスチャファイルパス
 */
Direct3D11GraphicsManager::Texture* Direct3D11GraphicsManager::load_texture( const char_t* name, const char_t* file_path )
{
	return direct_3d_->getTextureManager()->load( name, file_path );
}

/**
 * 指定した名前のテクスチャを取得する
 *
 * @param name テクスチャ名
 */
Direct3D11GraphicsManager::Texture* Direct3D11GraphicsManager::get_texture( const char_t* name )
{
	return direct_3d_->getTextureManager()->load( name, ( string_t( "media/model/" ) + name + ".png" ).c_str() );
}

/**
 * 描画のセットアップを行う
 *
 */
void Direct3D11GraphicsManager::setup_rendering() const
{
	direct_3d_->setInputLayout( "main" );

	direct_3d_->clear_default_view();

	direct_3d_->set_default_render_target();
	direct_3d_->set_default_viewport();
}

/**
 * 入力レイアウトを指定する
 *
 */
void Direct3D11GraphicsManager::set_input_layout( const char_t* name ) const
{
	direct_3d_->setInputLayout( name );
}

/**
 * 指定したテクニックの全てのパスでレンダリング処理を実行する
 *
 * @param technique_name テクニック名
 * @param function レンダリング処理
 */
void Direct3D11GraphicsManager::render_technique( const char_t* technique_name, const std::function< void() >& function ) const
{
	const auto* technique = direct_3d_->getEffect()->getTechnique( technique_name );
	
	if ( ! technique )
	{
		return;
	}

	for ( const auto& pass : technique->getPassList() )
	{
		pass->apply();

		function();
	}
}

/**
 * フェードイン・フェードアウト用の色を設定する
 *
 * @param color 色
 */
void Direct3D11GraphicsManager::set_fade_color( const Color& color )
{
	direct_3d_->getFader()->set_color( color );
}

void Direct3D11GraphicsManager::start_fade_in( float_t speed )
{
	is_fading_in_ = true;
	fade_speed_ = speed;
}
	
void Direct3D11GraphicsManager::start_fade_out( float_t speed )
{
	is_fading_in_ = false;
	fade_speed_ = speed;
}

void Direct3D11GraphicsManager::fade_in( float_t speed )
{
	direct_3d_->getFader()->fade_in( speed );
}
	
void Direct3D11GraphicsManager::fade_out( float_t speed )
{
	direct_3d_->getFader()->fade_out( speed );
}

/**
 * フェーダーを描画する
 *
 */
void Direct3D11GraphicsManager::render_fader() const
{
	ObjectConstantBufferData buffer_data;
	buffer_data.world = Matrix().set_identity().transpose();
	buffer_data.color = direct_3d_->getFader()->get_color();
	
	get_shared_object_render_data()->update( & buffer_data );

	set_input_layout( "main" );

	render_technique( "|main2d", [this]
	{
		get_shared_object_render_data()->bind_to_vs();
		get_shared_object_render_data()->bind_to_ps();

		direct_3d_->getFader()->render();
	} );
}

/**
 * 画面に文字列を描画する
 *
 * @param x 
 * @param y
 * @param text 文字列
 * @param color 色
 * @todo 高速化する
 */
void Direct3D11GraphicsManager::draw_text( float_t left, float_t top, float_t right, float bottom, const char_t* text, const Color& color ) const
{
	if ( ! direct_3d_->getFont() )
	{
		return;
	}

	{
		direct_3d_->begin2D();
		direct_3d_->getFont()->begin();

		direct_3d_->getFont()->draw_text( left, top, right, bottom, common::convert_to_wstring( string_t( text ) ).c_str(), color );

		direct_3d_->getFont()->end();
		direct_3d_->end2D();
	}

	{
		direct_3d_->setInputLayout( "main" );

		direct_3d_->begin3D();
		direct_3d_->renderText();
		direct_3d_->end3D();
	}
}

/**
 * 画面の中央に文字列を描画する
 *
 * @param text 文字列
 * @param color 色
 * @todo 高速化する
 */
void Direct3D11GraphicsManager::draw_text_at_center( const char_t* text, const Color& color ) const
{
	if ( ! direct_3d_->getFont() )
	{
		return;
	}

	direct_3d_->begin2D();
	direct_3d_->getFont()->begin();

	float_t left = 0.f;
	float_t top = ( direct_3d_->get_height() - direct_3d_->getFont()->get_font_height() ) / 2.f;
	float_t right = static_cast< float_t >( direct_3d_->get_width() );
	float_t bottom = top + direct_3d_->getFont()->get_font_height();

	direct_3d_->getFont()->draw_text_center( left, top, right, bottom, common::convert_to_wstring( string_t( text ) ).c_str(), color );

	direct_3d_->getFont()->end();
	direct_3d_->end2D();

	direct_3d_->begin3D();
	direct_3d_->renderText();
	direct_3d_->end3D();
}

/**
 * 指定した名前のテクスチャをアンロードする
 *
 * @param name テクスチャ名
 */
void Direct3D11GraphicsManager::unload_texture( const char_t* name )
{
	direct_3d_->getTextureManager()->unload( name );
}

/**
 * 全てのテクスチャをアンロードする
 *
 */
void Direct3D11GraphicsManager::unload_texture_all()
{
	direct_3d_->getTextureManager()->unload_all();
}


} // namespace blue_sky