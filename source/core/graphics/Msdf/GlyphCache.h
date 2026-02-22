#pragma once

#include "MsdfFont.h"
#include "GlyphAtlas.h"
#include <unordered_map>
#include <list>
#include <cstdint>

namespace core::graphics {

struct CachedGlyph {
	UVRect uv;
	GlyphMetrics metrics;
	int cell_index;
};

class GlyphCache {
	MsdfFont* font_;
	GlyphAtlas* atlas_;

	std::unordered_map< uint32_t, CachedGlyph > cache_;
	std::list< uint32_t > lru_;

	int msdf_size_;
	double msdf_range_;

	static constexpr size_t MAX_CACHED_GLYPHS = 2048;

	bool generate_glyph( uint32_t codepoint, CachedGlyph& result );
	void evict_oldest();

public:
	GlyphCache( MsdfFont* font, GlyphAtlas* atlas, int msdf_size = 48, double msdf_range = 4.0 );

	const CachedGlyph* get( uint32_t codepoint );
	void clear();
};

} // namespace core::graphics
