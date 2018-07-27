#pragma once

#include <core/type.h>

#include <game/Mesh.h>
#include <game/VertexGroup.h>
#include <game/Texture.h>

#include <common/safe_ptr.h>
#include <common/math.h>

#include <memory>
#include <vector>
#include <cassert>

class FbxFileLoader;

namespace blue_sky::graphics
{

/**
 * blue-sky 頂点情報
 * 
 * @todo 柔軟な頂点構造に対応する
 */
struct Vertex : public game::Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;

	Vertex()
		: Position( 0.f, 0.f, 0.f )
		, Normal( 0.f, 0.f, 0.f )
		, TexCoord( 0.f, 0.f )
	{

	}

	bool operator < ( const Vertex& v ) const
	{
		return
			( Position <  v.Position ) ||
			( Position == v.Position && Normal <  v.Normal ) ||
			( Position == v.Position && Normal == v.Normal && TexCoord < v.TexCoord );
	}
};

/**
 * blue-sky メッシュ
 *
 */
class Mesh : public game::Mesh
{
public:
	typedef blue_sky::graphics::Vertex		Vertex;
	typedef game::VertexGroup				VertexGroup;
	typedef game::Texture					Texture;
	
	typedef Vector3							Position;
	typedef Vector3							Normal;
	typedef Vector2							TexCoord;

	typedef std::vector< Position >			PositionList;
	typedef std::vector< Normal >			NormalList;
	typedef std::vector< TexCoord >			TexCoordList;

	typedef std::vector< Vertex >			VertexList;
	typedef std::vector< std::unique_ptr< VertexGroup > > VertexGroupList;

	typedef VertexGroup::Index Index;
	typedef std::vector< VertexGroup::IndexList::size_type > IndexCountList;

	class Buffer
	{
	private:
		Mesh* mesh_;

	protected:
		VertexList& get_vertex_list() { return mesh_->vertex_list_; }
		const VertexList& get_vertex_list() const { return mesh_->vertex_list_; }
		
		VertexWeightList& get_vertex_weight_list() { return mesh_->vertex_weight_list_; }
		const VertexWeightList& get_vertex_weight_list() const { return mesh_->vertex_weight_list_; }
		
		VertexGroupList& get_vertex_group_list() { return mesh_->vertex_group_list_; }
		const VertexGroupList& get_vertex_group_list() const { return mesh_->vertex_group_list_; }

		IndexCountList& get_index_count_list() { return mesh_->index_count_list_; }
		const IndexCountList& get_index_count_list() const { return mesh_->index_count_list_; }

	public:
		Buffer()
			: mesh_( 0 )
		{ }

		virtual ~Buffer()
		{ }

		void set_mesh( Mesh* m ) { mesh_ = m; }

		virtual void create_vertex_buffer() = 0;
		virtual void create_index_buffer() = 0;

		virtual void bind() const = 0;
		virtual void render( uint_t ) const = 0;

	};

protected:
	VertexList			vertex_list_;
	VertexWeightList	vertex_weight_list_;
	VertexGroupList		vertex_group_list_;
	IndexCountList		index_count_list_;

	std::unique_ptr< Buffer > buffer_;

public:
	explicit Mesh( Buffer* b )
		: buffer_( b )
	{
		buffer_->set_mesh( this );
	}

	virtual ~Mesh()	{ }

	void add_vertex( const game::Vertex& v ) override { vertex_list_.push_back( static_cast< const Vertex& >( v ) ); }
	uint_t get_vertex_count() const override { return vertex_list_.size(); }

	void add_vertex_weight( const VertexWeight& w ) override { vertex_weight_list_.push_back( w ); }

	VertexGroup* create_vertex_group() override;
	VertexGroup* get_vertex_group_at( uint_t ) override;
	
	uint_t get_rendering_vertex_group_count() const override { return index_count_list_.size(); }

	void invert_vertex_z() override;
	void flip_polygon() override;

	inline VertexList& get_vertex_list() { return vertex_list_; }
	inline const VertexList& get_vertex_list() const { return vertex_list_; }

	void optimize();

	void create_vertex_buffer() { buffer_->create_vertex_buffer(); }
	void create_index_buffer() { buffer_->create_index_buffer(); }

	void clear_vertex_list();
	void clear_vertex_weight_list();
	void clear_vertex_group_list();

	void bind() const override { buffer_->bind(); }
	void render( uint_t n ) const override { buffer_->render( n ); }

}; // class Mesh

} // namespace blue_sky::graphics
