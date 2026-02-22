#pragma once

#include <core/type.h>

namespace core::graphics {

struct TextStyle {
	Color text_color = Color::White;
	Color outline_color = Color::Black;
	float outline_width = 0.f;  // スクリーンピクセル単位
	float font_size = 0.f;      // 0.f = デフォルトサイズを使用
};

} // namespace core::graphics
