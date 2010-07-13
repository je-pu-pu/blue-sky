#ifndef BLUE_SKY_GRID_OBJECT_H
#define BLUE_SKY_GRID_OBJECT_H

class Direct3D9Mesh;

namespace blue_sky
{

class GridData;

/**
 * グリッド上に配置されるオブジェクト
 *
 */
class GridObject
{
public:
	typedef Direct3D9Mesh Mesh;

private:
	int x_;
	int y_;
	int z_;
	int rotate_degree_;

	GridData* grid_data_;


public:
	GridObject( int, int, int, int, GridData* );
	virtual ~GridObject();

	int x() const { return x_; }
	int y() const { return y_; }
	int z() const { return z_; }
	int rotate_degree() const { return rotate_degree_; }

	int width() const;
	int depth() const;
	
	GridData* grid_data() { return grid_data_; }
	const GridData* grid_data() const { return grid_data_; }

	const Mesh* mesh( int ) const;
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_OBJECT_H