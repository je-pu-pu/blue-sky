#pragma once

#include <bitset>
#include <limits>

namespace common
{

/**
 * �񋓌^ ( enum ) �̏W����ێ�����
 */
template< typename EnumType, size_t N >
class enum_set
{
public:

private:
	std::bitset< N > set_;

public:
	/**
	 * �ǂ̗v�f�������Ȃ���̏W�����쐬����
	 */
	enum_set() { }

	/**
	 * �w�肳�ꂽ�v�f�݂̂����W�����쐬����
	 */
	enum_set( EnumType e )
		: set_( 1 << static_cast< int >( e ) )
	{ }

	/**
	 * ���X�g�Ŏw�肳�ꂽ�v�f�����W�����쐬����
	 */
	enum_set( std::initializer_list< EnumType > es )
	{
		for ( auto e : es )
		{
			set_ |= 1 << static_cast< int >( e );
		}
	}

	/**
	 * �����Ŏw�肳�ꂽ�v�f�������Ă��邩�ǂ�����Ԃ�
	 */
	bool test( EnumType e ) const { return set_.test( static_cast< size_t >( e ) ); }

	// friend constexpr enum_set operator | ( enum_set s, EnumType e ) { s.set_ |= 1 << static_cast< int >( e ); return s; }
	// friend constexpr enum_set operator | ( EnumType e1, EnumType e2 ) { enum_set s( e1 ); return s | e2; }
};

} // namespace common

/*
ShaderStage::VS | ShaderStage::PS �Ƃ��Ă��A���̊֐��͉��̂��Ă΂�Ȃ�
template< typename EnumType >
common::enum_set< EnumType, EnumType::Max > operator | ( EnumType e1, EnumType e2 )
{
    common::enum_set< EnumType, EnumType::Max > s;
	s( e1 );
	s |= e2;

	return s;
}
*/

/*
// �r�b�g���� 32 �ɌŒ肷��� operator | ���g���邪�A�������Ȃ��̂ŋp��
template< typename EnumType >
common::enum_set< EnumType, 32 > operator | ( EnumType e1, EnumType e2 ) { common::enum_set< EnumType, 32 > s( e1 ); return s | e2; }
*/

/*
// s.set_ �� private �Ȃ̂ŃR���p�C���G���[
template< typename EnumType >
constexpr common::enum_set< EnumType, 32 > operator | ( const common::enum_set< EnumType, 32 > s, EnumType e ) { s.set_ |= e; return s; }

template< typename EnumType >
constexpr common::enum_set< EnumType, 32 > operator | ( EnumType e1, EnumType e2 ) { common::enum_set< EnumType, 32 > s( e1 ); return s | e2; }
*/