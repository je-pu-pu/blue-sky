#ifndef BLUE_SKY_GRID_CELL_H
#define BLUE_SKY_GRID_CELL_H

namespace blue_sky
{

/**
 * 1 �O���b�h�̃f�[�^
 *
 */
class GridCell
{
private:
	unsigned char height_;				///< ����
	unsigned char bound_;				///< ���˕Ԃ�̋���

public:
	GridCell();
	GridCell( unsigned char, unsigned char );
	~GridCell();

	unsigned char& height() { return height_; }
	unsigned char height() const { return height_; }

	unsigned char& bound() { return bound_; }
	unsigned char bound() const { return bound_; }

	class height_less
	{
	public:
		bool operator () ( const GridCell& a, const GridCell& b ) const
		{
			return a.height() < b.height();
		}

		bool operator () ( const GridCell* a, const GridCell* b ) const
		{
			return operator () ( *a, *b );
		}
	};
};

} // namespace blue_sky

#endif // BLUE_SKY_GRID_CELL_H