#pragma once

#include <cstdint>

#define MSDFGEN_PUBLIC
#define MSDFGEN_EXT_PUBLIC
#include <msdfgen/msdfgen.h>
#include <msdfgen/ext/import-font.h>

namespace core::graphics {

struct GlyphMetrics {
	double advance;
	double bearing_x;
	double bearing_y;
	double width;
	double height;
};

class MsdfFont {
	msdfgen::FreetypeHandle* ft_;
	msdfgen::FontHandle* font_;
	msdfgen::FontMetrics font_metrics_;

public:
	MsdfFont();
	~MsdfFont();

	bool load(const char* font_path);
	void unload();

	bool get_glyph_shape(uint32_t codepoint, msdfgen::Shape& shape, double& advance) const;
	bool get_glyph_metrics(uint32_t codepoint, GlyphMetrics& metrics) const;
	bool get_kerning(uint32_t left, uint32_t right, double& kerning) const;

	const msdfgen::FontMetrics& get_font_metrics() const { return font_metrics_; }
	bool is_loaded() const { return font_ != nullptr; }
};

} // namespace core::graphics
