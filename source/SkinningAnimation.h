#ifndef SKINNING_ANIMATION_H
#define SKINNING_ANIMATION_H

#include "Animation.h"

/**
 * スキニングアニメーション
 *
 */
template< typename Matrix >
class SkinningAnimation
{
public:
	typedef std::vector< Matrix > MatrixList;

	typedef std::vector< Animation > AnimationList;
	typedef std::map< const string_t, AnimationList > AnimationSet;

	typedef AnimationKeyFrame KeyFrame;
	
	typedef AnimationChannel Channel;
	typedef Channel::KeyFrameList KeyFrameList;

private:
	MatrixList bone_offset_matrix_list_;
	AnimationSet animation_set_;

public:
	SkinningAnimation()
	{ }

	Animation& get_animation( const char_t* name, uint_t n, bool force = true )
	{
		if ( force && animation_set_[ name ].size() <= n )
		{
			animation_set_[ name ].resize( n + 1 );
		}

		return animation_set_[ name ][ n ];
	}

	MatrixList& get_bone_offset_matrix_list() { return bone_offset_matrix_list_; }
	const MatrixList& get_bone_offset_matrix_list() const { return bone_offset_matrix_list_; }

	AnimationList* get_animation_list( const char_t* name ) { return & animation_set_[ name ]; }
	
	const AnimationList* get_animation_list( const char_t* name ) const
	{
		AnimationSet::const_iterator i = animation_set_.find( name );
		
		if ( i == animation_set_.end() )
		{
			return 0;
		}

		return & i->second;
	}

}; // class SkinningAnimation

#endif // SKINNING_ANIMATION_H
