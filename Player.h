#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "vector3.h"

namespace blue_sky
{

/**
 * �v���C���[
 *
 */
class Player
{
private:
	vector3 position_;	///< ���W
	vector3 velocity_;	///< �ړ�
	
	bool jumping_;	// �W�����v���t���O
	float floor_height_; ///< ����̍���
	
	float max_speed();
public:
	Player();
	~Player() { }
	
	vector3& position() { return position_; }
	vector3& velocity() { return velocity_; }

	/// �X�V
	void update();
	
	/// �W�����v����
	void jump();

	bool jumping() { return jumping_; }

	float get_floor_height() const { return floor_height_; }
	void set_floor_height( float h ) { floor_height_ = h; }
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
