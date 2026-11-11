#include "EffectStateDesc.h"

#include <cfloat>
#include <cstring>

namespace core::graphics::direct_3d_11
{

namespace
{

using effect::equals_ignore_case;
using effect::StateAssignment;
using effect::StateBlockDesc;

template< typename T >
struct NamedValue
{
	const char* name;
	T value;
};

const NamedValue< D3D11_BLEND > BLEND_TABLE[] = {
	{ "ZERO", D3D11_BLEND_ZERO },
	{ "ONE", D3D11_BLEND_ONE },
	{ "SRC_COLOR", D3D11_BLEND_SRC_COLOR },
	{ "INV_SRC_COLOR", D3D11_BLEND_INV_SRC_COLOR },
	{ "SRC_ALPHA", D3D11_BLEND_SRC_ALPHA },
	{ "INV_SRC_ALPHA", D3D11_BLEND_INV_SRC_ALPHA },
	{ "DEST_ALPHA", D3D11_BLEND_DEST_ALPHA },
	{ "INV_DEST_ALPHA", D3D11_BLEND_INV_DEST_ALPHA },
	{ "DEST_COLOR", D3D11_BLEND_DEST_COLOR },
	{ "INV_DEST_COLOR", D3D11_BLEND_INV_DEST_COLOR },
	{ "SRC_ALPHA_SAT", D3D11_BLEND_SRC_ALPHA_SAT },
	{ "BLEND_FACTOR", D3D11_BLEND_BLEND_FACTOR },
	{ "INV_BLEND_FACTOR", D3D11_BLEND_INV_BLEND_FACTOR },
	{ "SRC1_COLOR", D3D11_BLEND_SRC1_COLOR },
	{ "INV_SRC1_COLOR", D3D11_BLEND_INV_SRC1_COLOR },
	{ "SRC1_ALPHA", D3D11_BLEND_SRC1_ALPHA },
	{ "INV_SRC1_ALPHA", D3D11_BLEND_INV_SRC1_ALPHA },
};

const NamedValue< D3D11_BLEND_OP > BLEND_OP_TABLE[] = {
	{ "ADD", D3D11_BLEND_OP_ADD },
	{ "SUBTRACT", D3D11_BLEND_OP_SUBTRACT },
	{ "REV_SUBTRACT", D3D11_BLEND_OP_REV_SUBTRACT },
	{ "MIN", D3D11_BLEND_OP_MIN },
	{ "MAX", D3D11_BLEND_OP_MAX },
};

const NamedValue< D3D11_COMPARISON_FUNC > COMPARISON_TABLE[] = {
	{ "NEVER", D3D11_COMPARISON_NEVER },
	{ "LESS", D3D11_COMPARISON_LESS },
	{ "EQUAL", D3D11_COMPARISON_EQUAL },
	{ "LESS_EQUAL", D3D11_COMPARISON_LESS_EQUAL },
	{ "GREATER", D3D11_COMPARISON_GREATER },
	{ "NOT_EQUAL", D3D11_COMPARISON_NOT_EQUAL },
	{ "GREATER_EQUAL", D3D11_COMPARISON_GREATER_EQUAL },
	{ "ALWAYS", D3D11_COMPARISON_ALWAYS },
};

const NamedValue< D3D11_STENCIL_OP > STENCIL_OP_TABLE[] = {
	{ "KEEP", D3D11_STENCIL_OP_KEEP },
	{ "ZERO", D3D11_STENCIL_OP_ZERO },
	{ "REPLACE", D3D11_STENCIL_OP_REPLACE },
	{ "INCR_SAT", D3D11_STENCIL_OP_INCR_SAT },
	{ "DECR_SAT", D3D11_STENCIL_OP_DECR_SAT },
	{ "INVERT", D3D11_STENCIL_OP_INVERT },
	{ "INCR", D3D11_STENCIL_OP_INCR },
	{ "DECR", D3D11_STENCIL_OP_DECR },
};

const NamedValue< D3D11_DEPTH_WRITE_MASK > DEPTH_WRITE_MASK_TABLE[] = {
	{ "ZERO", D3D11_DEPTH_WRITE_MASK_ZERO },
	{ "ALL", D3D11_DEPTH_WRITE_MASK_ALL },
};

const NamedValue< D3D11_FILL_MODE > FILL_MODE_TABLE[] = {
	{ "WIREFRAME", D3D11_FILL_WIREFRAME },
	{ "SOLID", D3D11_FILL_SOLID },
};

const NamedValue< D3D11_CULL_MODE > CULL_MODE_TABLE[] = {
	{ "NONE", D3D11_CULL_NONE },
	{ "FRONT", D3D11_CULL_FRONT },
	{ "BACK", D3D11_CULL_BACK },
};

const NamedValue< D3D11_TEXTURE_ADDRESS_MODE > ADDRESS_TABLE[] = {
	{ "WRAP", D3D11_TEXTURE_ADDRESS_WRAP },
	{ "MIRROR", D3D11_TEXTURE_ADDRESS_MIRROR },
	{ "CLAMP", D3D11_TEXTURE_ADDRESS_CLAMP },
	{ "BORDER", D3D11_TEXTURE_ADDRESS_BORDER },
	{ "MIRROR_ONCE", D3D11_TEXTURE_ADDRESS_MIRROR_ONCE },
};

const NamedValue< D3D11_FILTER > FILTER_TABLE[] = {
	{ "MIN_MAG_MIP_POINT", D3D11_FILTER_MIN_MAG_MIP_POINT },
	{ "MIN_MAG_POINT_MIP_LINEAR", D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR },
	{ "MIN_POINT_MAG_LINEAR_MIP_POINT", D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT },
	{ "MIN_POINT_MAG_MIP_LINEAR", D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR },
	{ "MIN_LINEAR_MAG_MIP_POINT", D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT },
	{ "MIN_LINEAR_MAG_POINT_MIP_LINEAR", D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
	{ "MIN_MAG_LINEAR_MIP_POINT", D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT },
	{ "MIN_MAG_MIP_LINEAR", D3D11_FILTER_MIN_MAG_MIP_LINEAR },
	{ "ANISOTROPIC", D3D11_FILTER_ANISOTROPIC },
	{ "COMPARISON_MIN_MAG_MIP_POINT", D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT },
	{ "COMPARISON_MIN_MAG_POINT_MIP_LINEAR", D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR },
	{ "COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT", D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT },
	{ "COMPARISON_MIN_POINT_MAG_MIP_LINEAR", D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR },
	{ "COMPARISON_MIN_LINEAR_MAG_MIP_POINT", D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT },
	{ "COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR", D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR },
	{ "COMPARISON_MIN_MAG_LINEAR_MIP_POINT", D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT },
	{ "COMPARISON_MIN_MAG_MIP_LINEAR", D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR },
	{ "COMPARISON_ANISOTROPIC", D3D11_FILTER_COMPARISON_ANISOTROPIC },
};

/// 1 つの状態ブロックの値を読む（読めないものは warnings に足す）
class Reader
{
private:
	const StateBlockDesc& block_;
	std::vector< std::string >& warnings_;

public:
	Reader( const StateBlockDesc& block, std::vector< std::string >& warnings )
		: block_( block )
		, warnings_( warnings )
	{ }

	void warn( const StateAssignment& a, const std::string& message )
	{
		warnings_.push_back( a.location + ": " + block_.name + ": " + message );
	}

	void unknown_key( const StateAssignment& a )
	{
		warn( a, "知らないキーです（無視します）: " + a.key );
	}

	void bad_value( const StateAssignment& a )
	{
		warn( a, a.key + " の値を読めません（無視します）: " + a.value );
	}

	/// 添字を使えないキーに添字があれば知らせる。添字が無ければ true
	bool no_index( const StateAssignment& a )
	{
		if ( a.index >= 0 )
		{
			warn( a, a.key + " には添字を付けられません（無視します）" );
			return false;
		}

		return true;
	}

	bool read_bool( const StateAssignment& a, BOOL& out )
	{
		if ( equals_ignore_case( a.value, "TRUE" ) || a.value == "1" ) { out = TRUE; return true; }
		if ( equals_ignore_case( a.value, "FALSE" ) || a.value == "0" ) { out = FALSE; return true; }

		bad_value( a );
		return false;
	}

	template< typename T, size_t N >
	bool read_enum( const StateAssignment& a, const NamedValue< T > ( & table )[ N ], T& out )
	{
		for ( const auto& e : table )
		{
			if ( equals_ignore_case( a.value, e.name ) )
			{
				out = e.value;
				return true;
			}
		}

		bad_value( a );
		return false;
	}

	bool read_number( const StateAssignment& a, double& out )
	{
		if ( ! effect::parse_number( a.value, out ) )
		{
			bad_value( a );
			return false;
		}

		return true;
	}

	template< typename T >
	void read_integer( const StateAssignment& a, T& out )
	{
		double v = 0;

		if ( read_number( a, v ) )
		{
			out = static_cast< T >( static_cast< long long >( v ) );
		}
	}

	void read_float( const StateAssignment& a, FLOAT& out )
	{
		double v = 0;

		if ( read_number( a, v ) )
		{
			out = static_cast< FLOAT >( v );
		}
	}
};

bool is_key( const StateAssignment& a, const char* key )
{
	return equals_ignore_case( a.key, key );
}

} // namespace

D3D11_BLEND_DESC make_blend_desc( const StateBlockDesc& block, std::vector< std::string >& warnings )
{
	// FX11 の既定値（IndependentBlendEnable は true。書き込みマスクはすべての色）
	D3D11_BLEND_DESC desc { };
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = TRUE;

	for ( auto& rt : desc.RenderTarget )
	{
		rt.BlendEnable = FALSE;
		rt.SrcBlend = D3D11_BLEND_ONE;
		rt.DestBlend = D3D11_BLEND_ZERO;
		rt.BlendOp = D3D11_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D11_BLEND_ONE;
		rt.DestBlendAlpha = D3D11_BLEND_ZERO;
		rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	Reader r( block, warnings );

	for ( const auto& a : block.assignments )
	{
		if ( is_key( a, "AlphaToCoverageEnable" ) )
		{
			if ( r.no_index( a ) ) { r.read_bool( a, desc.AlphaToCoverageEnable ); }
			continue;
		}

		if ( is_key( a, "IndependentBlendEnable" ) )
		{
			if ( r.no_index( a ) ) { r.read_bool( a, desc.IndependentBlendEnable ); }
			continue;
		}

		// ここから下はレンダーターゲットごとのキー。添字が無ければ 0 番（effect の配列への代入と同じ）
		const int index = a.index < 0 ? 0 : a.index;

		if ( index >= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )
		{
			r.warn( a, a.key + " の添字が範囲の外です（無視します）: " + std::to_string( index ) );
			continue;
		}

		auto& rt = desc.RenderTarget[ index ];

		if ( is_key( a, "BlendEnable" ) ) { r.read_bool( a, rt.BlendEnable ); }
		else if ( is_key( a, "SrcBlend" ) ) { r.read_enum( a, BLEND_TABLE, rt.SrcBlend ); }
		else if ( is_key( a, "DestBlend" ) ) { r.read_enum( a, BLEND_TABLE, rt.DestBlend ); }
		else if ( is_key( a, "BlendOp" ) ) { r.read_enum( a, BLEND_OP_TABLE, rt.BlendOp ); }
		else if ( is_key( a, "SrcBlendAlpha" ) ) { r.read_enum( a, BLEND_TABLE, rt.SrcBlendAlpha ); }
		else if ( is_key( a, "DestBlendAlpha" ) ) { r.read_enum( a, BLEND_TABLE, rt.DestBlendAlpha ); }
		else if ( is_key( a, "BlendOpAlpha" ) ) { r.read_enum( a, BLEND_OP_TABLE, rt.BlendOpAlpha ); }
		else if ( is_key( a, "RenderTargetWriteMask" ) ) { r.read_integer( a, rt.RenderTargetWriteMask ); }
		else { r.unknown_key( a ); }
	}

	return desc;
}

D3D11_DEPTH_STENCIL_DESC make_depth_stencil_desc( const StateBlockDesc& block, std::vector< std::string >& warnings )
{
	D3D11_DEPTH_STENCIL_DESC desc { };
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = FALSE;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	for ( auto* face : { & desc.FrontFace, & desc.BackFace } )
	{
		face->StencilFailOp = D3D11_STENCIL_OP_KEEP;
		face->StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		face->StencilPassOp = D3D11_STENCIL_OP_KEEP;
		face->StencilFunc = D3D11_COMPARISON_ALWAYS;
	}

	Reader r( block, warnings );

	for ( const auto& a : block.assignments )
	{
		if ( ! r.no_index( a ) )
		{
			continue;
		}

		if ( is_key( a, "DepthEnable" ) ) { r.read_bool( a, desc.DepthEnable ); }
		else if ( is_key( a, "DepthWriteMask" ) ) { r.read_enum( a, DEPTH_WRITE_MASK_TABLE, desc.DepthWriteMask ); }
		else if ( is_key( a, "DepthFunc" ) ) { r.read_enum( a, COMPARISON_TABLE, desc.DepthFunc ); }
		else if ( is_key( a, "StencilEnable" ) ) { r.read_bool( a, desc.StencilEnable ); }
		else if ( is_key( a, "StencilReadMask" ) ) { r.read_integer( a, desc.StencilReadMask ); }
		else if ( is_key( a, "StencilWriteMask" ) ) { r.read_integer( a, desc.StencilWriteMask ); }
		else if ( is_key( a, "FrontFaceStencilFail" ) ) { r.read_enum( a, STENCIL_OP_TABLE, desc.FrontFace.StencilFailOp ); }
		else if ( is_key( a, "FrontFaceStencilDepthFail" ) ) { r.read_enum( a, STENCIL_OP_TABLE, desc.FrontFace.StencilDepthFailOp ); }
		else if ( is_key( a, "FrontFaceStencilPass" ) ) { r.read_enum( a, STENCIL_OP_TABLE, desc.FrontFace.StencilPassOp ); }
		else if ( is_key( a, "FrontFaceStencilFunc" ) ) { r.read_enum( a, COMPARISON_TABLE, desc.FrontFace.StencilFunc ); }
		else if ( is_key( a, "BackFaceStencilFail" ) ) { r.read_enum( a, STENCIL_OP_TABLE, desc.BackFace.StencilFailOp ); }
		else if ( is_key( a, "BackFaceStencilDepthFail" ) ) { r.read_enum( a, STENCIL_OP_TABLE, desc.BackFace.StencilDepthFailOp ); }
		else if ( is_key( a, "BackFaceStencilPass" ) ) { r.read_enum( a, STENCIL_OP_TABLE, desc.BackFace.StencilPassOp ); }
		else if ( is_key( a, "BackFaceStencilFunc" ) ) { r.read_enum( a, COMPARISON_TABLE, desc.BackFace.StencilFunc ); }
		else { r.unknown_key( a ); }
	}

	return desc;
}

D3D11_RASTERIZER_DESC make_rasterizer_desc( const StateBlockDesc& block, std::vector< std::string >& warnings )
{
	D3D11_RASTERIZER_DESC desc { };
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_BACK;
	desc.FrontCounterClockwise = FALSE;
	desc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
	desc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	Reader r( block, warnings );

	for ( const auto& a : block.assignments )
	{
		if ( ! r.no_index( a ) )
		{
			continue;
		}

		if ( is_key( a, "FillMode" ) ) { r.read_enum( a, FILL_MODE_TABLE, desc.FillMode ); }
		else if ( is_key( a, "CullMode" ) ) { r.read_enum( a, CULL_MODE_TABLE, desc.CullMode ); }
		else if ( is_key( a, "FrontCounterClockwise" ) ) { r.read_bool( a, desc.FrontCounterClockwise ); }
		else if ( is_key( a, "DepthBias" ) ) { r.read_integer( a, desc.DepthBias ); }
		else if ( is_key( a, "DepthBiasClamp" ) ) { r.read_float( a, desc.DepthBiasClamp ); }
		else if ( is_key( a, "SlopeScaledDepthBias" ) ) { r.read_float( a, desc.SlopeScaledDepthBias ); }
		else if ( is_key( a, "DepthClipEnable" ) ) { r.read_bool( a, desc.DepthClipEnable ); }
		else if ( is_key( a, "ScissorEnable" ) ) { r.read_bool( a, desc.ScissorEnable ); }
		else if ( is_key( a, "MultisampleEnable" ) ) { r.read_bool( a, desc.MultisampleEnable ); }
		else if ( is_key( a, "AntialiasedLineEnable" ) ) { r.read_bool( a, desc.AntialiasedLineEnable ); }
		else { r.unknown_key( a ); }
	}

	return desc;
}

D3D11_SAMPLER_DESC make_sampler_desc( const StateBlockDesc& block, std::vector< std::string >& warnings )
{
	// FX11 の既定値（MaxAnisotropy は 16、BorderColor は 0）
	D3D11_SAMPLER_DESC desc { };
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS;
	desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	for ( auto& c : desc.BorderColor )
	{
		c = D3D11_DEFAULT_BORDER_COLOR_COMPONENT;
	}

	desc.MinLOD = -FLT_MAX;
	desc.MaxLOD = FLT_MAX;

	Reader r( block, warnings );

	for ( const auto& a : block.assignments )
	{
		if ( ! r.no_index( a ) )
		{
			continue;
		}

		if ( is_key( a, "Filter" ) ) { r.read_enum( a, FILTER_TABLE, desc.Filter ); }
		else if ( is_key( a, "AddressU" ) ) { r.read_enum( a, ADDRESS_TABLE, desc.AddressU ); }
		else if ( is_key( a, "AddressV" ) ) { r.read_enum( a, ADDRESS_TABLE, desc.AddressV ); }
		else if ( is_key( a, "AddressW" ) ) { r.read_enum( a, ADDRESS_TABLE, desc.AddressW ); }
		else if ( is_key( a, "MipLODBias" ) ) { r.read_float( a, desc.MipLODBias ); }
		else if ( is_key( a, "MaxAnisotropy" ) ) { r.read_integer( a, desc.MaxAnisotropy ); }
		else if ( is_key( a, "ComparisonFunc" ) ) { r.read_enum( a, COMPARISON_TABLE, desc.ComparisonFunc ); }
		else if ( is_key( a, "MinLOD" ) ) { r.read_float( a, desc.MinLOD ); }
		else if ( is_key( a, "MaxLOD" ) ) { r.read_float( a, desc.MaxLOD ); }
		else if ( is_key( a, "BorderColor" ) )
		{
			std::array< float, 4 > color { };

			if ( effect::parse_float4( a.value, color ) )
			{
				std::memcpy( desc.BorderColor, color.data(), sizeof( desc.BorderColor ) );
			}
			else
			{
				r.bad_value( a );
			}
		}
		else { r.unknown_key( a ); }
	}

	return desc;
}

} // namespace core::graphics::direct_3d_11
