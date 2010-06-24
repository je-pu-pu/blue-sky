#ifndef BLUE_SKY_GRID_DATA_H
#define BLUE_SKY_GRID_DATA_H

namespace blue_sky
{

/**
 * グリッド上に配置されるオブジェクトのデータ
 *
 */
class GridData
{
public:
	typedef unsigned char ChipType;

private:
	/*
	int x_;
	int y_;
	int z_;
	*/

	int width_;
	int depth_;

	ChipType* chip_;

public:
	GridData( int, int );
	virtual ~GridData();

	/*
	int x() { return x_; }
	int y() { return y_; }
	int z() { return z_; }
	*/

	int width() const { return width_; }
	int depth() const { return depth_; }

	ChipType& chip( int, int );
	ChipType chip( int, int ) const;

	void put( int, int, int, const GridData* );
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_DATA_H