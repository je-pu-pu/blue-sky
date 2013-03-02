#ifndef DIRECT_3D_11_MATERIAL_H
#define DIRECT_3D_11_MATERIAL_H

#include <d3d11.h>
#include <xnamath.h>
#include <vector>

class Direct3D11;

/**
 * Direct3D11Mesh ‚Ìƒ}ƒeƒŠƒAƒ‹
 *
 */
class Direct3D11Material
{
public:
	typedef WORD Index;
	typedef std::vector< Index > IndexList;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D11*					direct_3d_;

	ID3D11Buffer*				index_buffer_;
	ID3D11ShaderResourceView*	texture_resource_view_;

	IndexList					index_list_;
	
	void create_texture_resource_view( const char* );

public:
	Direct3D11Material( Direct3D11* );
	virtual ~Direct3D11Material();

	void create_index_buffer();
	void load_texture( const char* );

	ID3D11ShaderResourceView* get_shader_resource_view() { return texture_resource_view_; }
	void set_shader_resource_view( ID3D11ShaderResourceView* view ) { texture_resource_view_ = view; }

	IndexList& get_index_list() { return index_list_; }
	const IndexList& get_index_list() const { return index_list_; }

	void render() const;

}; // class Direct3D11Material

#endif // DIRECT_3D_11_MATERIAL_H
