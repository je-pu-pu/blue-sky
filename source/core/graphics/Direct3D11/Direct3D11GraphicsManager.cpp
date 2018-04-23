#include "Direct3D11GraphicsManager.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11SkyBox.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11Effect.h"
#include "Direct3D11Fader.h"
#include "Direct3D11Axis.h"
#include "Direct3D11BulletDebugDraw.h"
#include "Direct3D11.h"

#include <GameMain.h>
#include <core/Vector3.h>
#include <core/graphics/DirectWrite/DirectWrite.h>

#include "ConstantBuffer.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

#include <common/string.h>

namespace blue_sky
{

/**
 * �R���X�g���N�^
 *
 * @param direct_3d Direct3D
 */
Direct3D11GraphicsManager::Direct3D11GraphicsManager( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, game_render_data_( new GameConstantBuffer( direct_3d ) )
	, frame_render_data_( new FrameConstantBuffer( direct_3d ) )
	, frame_drawing_render_data_( new FrameDrawingConstantBuffer( direct_3d ) )
	, shared_object_render_data_( new ObjectConstantBuffer( direct_3d ) )
	, debug_axis_( new Direct3D11Axis( direct_3d_ ) )
{
	
}

/**
 * �f�X�g���N�^
 *
 */
Direct3D11GraphicsManager::~Direct3D11GraphicsManager()
{
	
}

/**
 * �X�V����
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
 * ���b�V���𐶐�����
 *
 * @return ���b�V��
 */
Direct3D11GraphicsManager::Mesh* Direct3D11GraphicsManager::create_mesh()
{
	return new Direct3D11Mesh( direct_3d_ );
}

/**
 * ���C���𐶐�����
 *
 * @return ���C��
 */
Direct3D11GraphicsManager::Line* Direct3D11GraphicsManager::create_line()
{
	return create_drawing_line();
}

/**
 * ��`�������b�V���𐶐�����
 *
 * @return ��`�������b�V��
 */
DrawingMesh* Direct3D11GraphicsManager::create_drawing_mesh()
{
	return new DrawingMesh( create_mesh() );
}

/**
 * ��`�������C���𐶐�����
 *
 * @return ���C��
 */
DrawingLine* Direct3D11GraphicsManager::create_drawing_line()
{
	return new DrawingLine( direct_3d_ );
}

/**
 * �e�N�X�`����ǂݍ���
 *
 * @param name �e�N�X�`����
 * @param file_path �e�N�X�`���t�@�C���p�X
 */
Direct3D11GraphicsManager::Texture* Direct3D11GraphicsManager::load_texture( const char_t* name, const char_t* file_path )
{
	return direct_3d_->getTextureManager()->load( name, file_path );
}

/**
 * �w�肵�����O�̃e�N�X�`�����擾����
 *
 * @param name �e�N�X�`����
 */
Direct3D11GraphicsManager::Texture* Direct3D11GraphicsManager::get_texture( const char_t* name )
{
	return direct_3d_->getTextureManager()->load( name, ( string_t( "media/model/" ) + name + ".png" ).c_str() );
}

/**
 * �w�肵�����O�̃e�N�X�`�����A�����[�h����
 *
 * @param name �e�N�X�`����
 */
void Direct3D11GraphicsManager::unload_texture( const char_t* name )
{
	direct_3d_->getTextureManager()->unload( name );
}

/**
 * �S�Ẵe�N�X�`�����A�����[�h����
 *
 */
void Direct3D11GraphicsManager::unload_texture_all()
{
	direct_3d_->getTextureManager()->unload_all();
}

/**
 * �X�J�C�{�b�N�X��ݒ肷��
 *
 * @param name �X�J�C�{�b�N�X��
 */
void Direct3D11GraphicsManager::set_sky_box( const char_t* name )
{
	if ( ! sky_box_ )
	{
		sky_box_render_data_ = std::make_unique< ObjectConstantBufferWithData >( direct_3d_ );
		sky_box_render_data_->data().color = Color::White;
	}

	delete sky_box_.release();
	sky_box_ = std::make_unique< Direct3D11SkyBox >( direct_3d_, name );
}

/**
 * �X�J�C�{�b�N�X�̐ݒ����������
 *
 */
void Direct3D11GraphicsManager::unset_sky_box()
{
	delete sky_box_.release();
	delete sky_box_render_data_.release();
}

/**
 * �X�J�C�{�b�N�X�����ݐݒ肳��Ă��邩�ǂ�����Ԃ�
 *
 * @return �X�J�C�{�b�N�X�����ݐݒ肳��Ă��邩�ǂ���
 */
bool Direct3D11GraphicsManager::is_sky_box_set() const
{
	return static_cast< bool >( sky_box_ );
}

/**
 * �n�ʂ�ݒ肷��
 *
 * @param name �n�ʃ��f����
 */
void Direct3D11GraphicsManager::set_ground( const char_t* name )
{
	/// @tood Direct3D11Mesh ���C���������� load_obj() ��ǂ�ł��o�b�t�@�����X�g���Ȃ��悤�ɂ���
	unset_ground();

	ground_ = std::make_unique< Direct3D11Mesh >( direct_3d_ );
	ground_->load_obj( ( string_t( "media/model/" ) + name + ".obj" ).c_str() );

	ground_render_data_ = std::make_unique< ObjectConstantBufferWithData >( direct_3d_ );
	ground_render_data_->data().world.set_identity();
	ground_render_data_->data().color = Color::White;
}

/**
 * �n�ʂ̐ݒ����������
 *
 */
void Direct3D11GraphicsManager::unset_ground()
{
	delete ground_.release();
	delete ground_render_data_.release();
}

/**
 * �����̐F��ݒ肷��
 *
 * @param color �����̐F
 */
void Direct3D11GraphicsManager::set_ambient_color( const Color& color )
{
	if ( sky_box_render_data_ )
	{
		sky_box_render_data_->data().color = color;
	}

	if ( ground_render_data_ )
	{
		ground_render_data_->data().color = color;
	}
}

/**
 * �e�̐F��ݒ肷��
 *
 * @param color �e�̐F
 */
void Direct3D11GraphicsManager::set_shadow_color( const Color& color )
{
	frame_drawing_render_data_->data().shadow_color = color;
}

/**
 * �e�̎��e�N�X�`���̐F��ݒ肷��
 *
 * @param color �e�̎��e�N�X�`���̐F
 */
void Direct3D11GraphicsManager::set_shadow_paper_color( const Color& color )
{
	frame_drawing_render_data_->data().shadow_paper_color = color;
}

/**
 * �菑�����\���̃A�N�Z���g��ݒ肷��
 *
 * @param accent �A�N�Z���g
 */
void Direct3D11GraphicsManager::set_drawing_accent( float_t accent )
{
	frame_drawing_render_data_->data().accent = accent;
}

/**
 * �菑�������̃^�C�v��ݒ肷��
 *
 * @param type �菑�������̃^�C�v
 */
void Direct3D11GraphicsManager::set_drawing_line_type( int_t type )
{
	frame_drawing_render_data_->data().line_type = math::clamp< int >( type, 0, DrawingLine::LINE_TYPE_MAX - 1 );
}

void Direct3D11GraphicsManager::set_eye_position( const Vector3& pos )
{
	if ( sky_box_ )
	{
		sky_box_render_data_->data().world = Matrix().set_translation( pos.x(), pos.y(), pos.z() ).transpose();
	}
}

/**
 * �`��̃Z�b�g�A�b�v���s��
 *
 */
void Direct3D11GraphicsManager::setup_rendering() const
{
	direct_3d_->setInputLayout( "main" );

	direct_3d_->clear_default_view();

	direct_3d_->set_default_render_target();
	direct_3d_->set_default_viewport();

	/// @todo ���t���[���s���K�v�����邩�H
	frame_drawing_render_data_->update();
}

/**
 * ���̓��C�A�E�g���w�肷��
 *
 */
void Direct3D11GraphicsManager::set_input_layout( const char_t* name ) const
{
	direct_3d_->setInputLayout( name );
}

/**
 * �w�肵���e�N�j�b�N�̑S�Ẵp�X�Ń����_�����O���������s����
 *
 * @param technique_name �e�N�j�b�N��
 * @param function �����_�����O����
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
 * �w�i ( �X�J�C�{�b�N�X�E�n�� ) ��`�悷��
 *
 */
void Direct3D11GraphicsManager::render_background() const
{
	render_technique( "|sky_box", [this]
	{
		get_frame_render_data()->bind_to_vs();

		// sky box
		if ( sky_box_ )
		{
			sky_box_render_data_->update();
			sky_box_render_data_->bind_to_vs();

			sky_box_->render();
		}

		// ground
		if ( ground_ )
		{
			ground_render_data_->update();
			ground_render_data_->bind_to_vs();

			ground_->render();
		}
	} );
}

/**
 * �t�F�[�h�C���E�t�F�[�h�A�E�g�p�̐F��ݒ肷��
 *
 * @param color �F
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
 * �t�F�[�_�[��`�悷��
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
 * ��ʂɕ������`�悷��
 *
 * @param x 
 * @param y
 * @param text ������
 * @param color �F
 * @todo ����������
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
 * ��ʂ̒����ɕ������`�悷��
 *
 * @param text ������
 * @param color �F
 * @todo ����������
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
 * �f�o�b�O�p�̎���`�悷��
 *
 */
void Direct3D11GraphicsManager::render_debug_axis_model() const
{
	debug_axis_->render();
}

/**
 * Bullet �f�o�b�O
 *
 */
void Direct3D11GraphicsManager::render_debug_bullet() const
{
	render_technique( "|bullet", [this]
	{
		get_game_render_data()->bind_to_vs();
		get_frame_render_data()->bind_to_vs();

		GameMain::get_instance()->get_bullet_debug_draw()->render();
	} );
}

} // namespace blue_sky