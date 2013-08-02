#ifndef DIRECT_3D_11_MATERIAL_H
#define DIRECT_3D_11_MATERIAL_H

#include "type.h"
#include <game/Material.h>
#include <d3d11.h>
#include <xnamath.h>
#include <vector>

namespace game
{

class Material;
class Texture;

}

class Direct3D11;

/**
 * Direct3D11Mesh ‚Ìƒ}ƒeƒŠƒAƒ‹
 *
 */
class Direct3D11Material : public game::Material
{
public:
	typedef WORD Index;
	typedef std::vector< Index > IndexList;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D11*					direct_3d_;
	
	ID3D11Buffer*				index_buffer_;
	game::Texture*				texture_;

	IndexList					index_list_;
	uint_t						index_count_;
	
	void create_texture_resource_view( const char* );

public:
	Direct3D11Material( Direct3D11* );
	virtual ~Direct3D11Material();

	void create_index_buffer();
	void clear_index_list();

	void load_texture( const char_t* );

	game::Texture* get_texture();
	void set_texture( game::Texture* );

	IndexList& get_index_list() { return index_list_; }
	const IndexList& get_index_list() const { return index_list_; }

	void bind_to_ia() const;
	void render() const override;

}; // class Direct3D11Material

#endif // DIRECT_3D_11_MATERIAL_H
