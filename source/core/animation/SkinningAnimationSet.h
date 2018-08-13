#pragma once

#include "SkinningAnimation.h"
#include <core/type.h>
#include <common/exception.h>
#include <vector>
#include <map>

namespace core
{

/**
 * スキニングアニメーションの複数セット
 *
 */
class SkinningAnimationSet
{
public:
	typedef std::map< const string_t, SkinningAnimation > SkinningAnimationMap;
	typedef std::vector< Matrix* > MatrixList;
	typedef std::vector< uint_t > BoneIndexList;
	typedef std::vector< BoneIndexList > ChildBoneIndexList;
	typedef std::vector< uint_t > ParentBoneIndexList;

private:
	SkinningAnimationMap	skinning_animation_map_;
	MatrixList				bone_offset_matrix_list_;
	ParentBoneIndexList		parent_bone_index_list_;
	ChildBoneIndexList		child_bone_index_list_;

public:
	SkinningAnimationSet()
	{
		parent_bone_index_list_.resize( 1 );
		parent_bone_index_list_[ 0 ] = 0;
	}

	~SkinningAnimationSet()
	{
		for ( auto i = bone_offset_matrix_list_.begin(); i != bone_offset_matrix_list_.end(); ++i )
		{
			delete *i;
		}
	}

	/**
	 * 指定した名前に対応したスキニングアニメーションを取得する
	 *
	 * @param name スキニングアニメーションの名前
	 * @return スキニングアニメーション
	 */
	SkinningAnimation& get_skinning_animation( const char_t* name )
	{
		auto i = skinning_animation_map_.find( name );

		if ( i == skinning_animation_map_.end() )
		{
			i = skinning_animation_map_.insert( std::make_pair( name, SkinningAnimation( bone_offset_matrix_list_.size() ) ) ).first;
		}

		return i->second;
	}

	const SkinningAnimation* get_skinning_animation( const char_t* name ) const
	{
		auto i = skinning_animation_map_.find( name );

		if ( i == skinning_animation_map_.end() )
		{
			return 0;
		}

		return & i->second;
	}


	/**
	 * 指定した名前とボーンインデックスに対応するアニメーションを取得する
	 *
	 * @param name スキニングアニメーションの名前
	 * @param bone_index ボーンインデックス
	 * @return アニメーション
	 */
	Animation& get_bone_animation( const char_t* name, uint_t bone_index )
	{
		return get_skinning_animation( name ).get_bone_animation_by_bone_index( bone_index );
	}

	/*** @todo 整理する */
	Matrix& get_bone_offset_matrix_by_bone_index( uint_t bone_index ) { return *bone_offset_matrix_list_[ bone_index ]; }
	const Matrix& get_bone_offset_matrix_by_bone_index( uint_t bone_index ) const { return *bone_offset_matrix_list_[ bone_index ]; }

	/**
	 * ボーンの数を取得する
	 *
	 * @return ボーンの数
	 */
	uint_t get_bone_count() const
	{
		return bone_offset_matrix_list_.size();
	}

	/**
	 * ボーンの数を設定する
	 *
	 * @param count ボーンの数
	 */
	void set_bone_count( uint_t count )
	{
		if ( ! bone_offset_matrix_list_.empty() )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( "duplicate call set_bonoe_count()" );
		}

		bone_offset_matrix_list_.resize( count );

		for ( auto i = bone_offset_matrix_list_.begin(); i != bone_offset_matrix_list_.end(); ++i )
		{
			*i = new Matrix( Matrix::identity() );
		}

		for ( auto i = skinning_animation_map_.begin(); i != skinning_animation_map_.end(); ++i )
		{
			i->second.resize_bone_animation_list( count );
		}

		parent_bone_index_list_.resize( count );
		child_bone_index_list_.resize( count );
	}

	/**
	 * 親ボーンのインデックスに子ボーンのインデックスを対応させる
	 *
	 * @param bone_index 親ボーンのインデックス
	 * @param child_bone_index ボーンのインデックス
	 */
	void add_child_bone_index( uint_t bone_index, uint_t child_bone_index )
	{
		if ( bone_index >= child_bone_index_list_.size() )
		{
			child_bone_index_list_.resize( bone_index + 1 );
		}

		child_bone_index_list_[ bone_index ].push_back( child_bone_index );

		if ( child_bone_index >= parent_bone_index_list_.size() )
		{
			parent_bone_index_list_.resize( child_bone_index + 1 );
		}

		parent_bone_index_list_[ child_bone_index ] = bone_index;
	}

	/**
	 * インデックスで指定したボーンの子ボーンの数を取得する
	 *
	 * @param bone_index ボーンのインデックス
	 */
	uint_t get_child_bone_count( uint_t bone_index ) const
	{
		if ( bone_index >= child_bone_index_list_.size() )
		{
			return 0;
		}

		return child_bone_index_list_[ bone_index ].size();
	}

	/**
	 * 子ボーンのインデックスを取得する
	 *
	 * @param bone_index ボーンのインデックス
	 * @param n 子ボーンの親ボーン内でのインデックス
	 * @return 子ボーンの全ボーン内でのインデックス
	 */
	uint_t get_child_bone_index( uint_t bone_index, uint_t n ) const
	{
		return child_bone_index_list_[ bone_index ][ n ];
	}

	/**
	 * 親ボーンのインデックスを取得する
	 *
	 * @param bone_index ボーンのインデックス
	 * @return 親ボーンの全ボーン内でのインデックス
	 */
	uint_t get_parent_bone_index( uint_t bone_index ) const
	{
		return parent_bone_index_list_[ bone_index ];
	}

	/**
	 * 最適化する
	 *
	 */
	void optimize()
	{
		for ( auto i = skinning_animation_map_.begin(); i != skinning_animation_map_.end(); ++i )
		{
			i->second.optimize();
		}
	}

	/**
	 * アニメーションの長さを計算する
	 *
	 */
	void calculate_length()
	{
		for ( auto i = skinning_animation_map_.begin(); i != skinning_animation_map_.end(); ++i )
		{
			i->second.calculate_length();
		}
	}

}; // class SkinningAnimation

} // namespace core
