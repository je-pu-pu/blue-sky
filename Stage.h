#ifndef BLUE_SKY_STAGE_H
#define BLUE_SKY_STAGE_H

namespace blue_sky
{

/**
 * ステージ
 *
 */
class Stage
{
public:
	typedef unsigned char MapChipType;

private:
	int width_;
	int depth_;

	MapChipType* map_chip_;

public:
	Stage( int, int );
	~Stage();

	int width() const { return width_; }
	int depth() const { return depth_; }

	MapChipType& map_chip( int, int );
	MapChipType map_chip( int, int ) const;
};

} // namespace blue_sky

#endif // BLUE_SKY_STAGE_H