#ifndef DIRECT_3D_9_RESOURCE_H
#define DIRECT_3D_9_RESOURCE_H

class Direct3D9;

/**
 * ID3DX*** のラッパークラスの基底クラス
 *
 */
class Direct3D9Resource
{
private:
	Direct3D9* direct_3d_;

protected:
	Direct3D9Resource( Direct3D9* );

	const Direct3D9* direct_3d() const { return direct_3d_; }

public:
	virtual void on_lost_device() = 0;
	virtual void on_reset_device() = 0;

}; // Direct3D9Resource


#endif // DIRECT_3D_9_RESOURCE_H
