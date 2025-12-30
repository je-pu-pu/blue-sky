#pragma once

#include <core/type.h>

namespace core::graphics
{

/**
 * インスタンスバッファの抽象基底クラス
 */
class InstanceBuffer
{
public:
	using Matrix = core::Matrix;

	virtual ~InstanceBuffer() = default;

	virtual void update( const Matrix* matrices, size_t count ) = 0;
	virtual void bind_to_vs() const = 0;
	virtual void unbind_from_vs() const = 0;
	virtual size_t get_current_count() const = 0;

}; // class InstanceBuffer

} // namespace core::graphics
