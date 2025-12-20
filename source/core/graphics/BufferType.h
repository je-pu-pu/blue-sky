#pragma once

namespace core::graphics
{

/**
 * @brief バッファの種類
 */
enum class BufferType {
	DEFAULT,		// 通常のバッファ ( 更新しないバッファ )
	UPDATABLE		// CPU からの書き込みを行えるバッファ
};

} // namespace core::graphics
