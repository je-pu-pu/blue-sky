#ifndef DIRECT_3D_11_FADER_H
#define DIRECT_3D_11_FADER_H

#include "Direct3D11Rectangle.h"
#include "Direct3D11Color.h"

/**
 * Direct3D 11 Fader
 *
 * ��ʂ��t�F�[�h�C��, �t�F�[�h�A�E�g������
 */
class Direct3D11Fader : public Direct3D11Rectangle
{
private:
	Color			fade_color_;		///< �t�F�[�h�A�E�g���̐F
	Color			current_color_;		///< ���݂̐F
	float_t			fade_;				///< �t�F�[�h�l ( 0.f .. 1.f )

	void update_color();
public:
	Direct3D11Fader( Direct3D* );

	bool fade_in( float_t speed = 0.01f );
	bool fade_out( float_t speed = 0.01f );

	void full_in();
	void full_out();

	void set_color( const Color& c ) { fade_color_ = c; current_color_ = Color( c.r(), c.g(), c.b(), current_color_.a() ); }
	const Color& get_color() const { return current_color_; }

}; // class Direct3D11Fader

#endif // DIRECT_3D_11_FADER_H
