Texture2D model_texture : register( t0 );
Texture2D line_texture : register( t0 );

SamplerState texture_sampler
{
	Filter = ANISOTROPIC; // MIN_POINT_MAG_LINEAR_MIP_POINT;
//	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
//	MinLOD = 5.f;
//	MaxLOD = FLOAT32_MAX;
//	ComparisonFunc = NEVER;
};

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float t;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
};

struct GSPS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};

GSPS_INPUT vs( VS_INPUT input, uint vertex_id : SV_VertexID )
{
	GSPS_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	output.TexCoord = input.TexCoord;

	// �Ԃ炷
	float a = ( vertex_id ) / 10.f + t * 10.f;
	output.Position.x += cos( a ) * 0.01f;
	output.Position.y += sin( t ) * 0.01f;

	// output.Position = input.Position;

    return output;
}

[maxvertexcount(12)]
void gs_pass( triangle GSPS_INPUT input[3], inout TriangleStream<GSPS_INPUT> TriStream )
{
	for ( uint n = 0; n < 3; n++ )
	{
		GSPS_INPUT output;
		output.Position = input[ n ].Position;
		output.TexCoord = input[ n ].TexCoord;

		TriStream.Append( output );
	}

	TriStream.RestartStrip();
}

[maxvertexcount(12)]
void gs_line( triangle GSPS_INPUT input[3], inout TriangleStream<GSPS_INPUT> TriStream )
{
	static const float screen_width = 800.f;
	static const float screen_height = 600.f;
	static const float screen_ratio = ( screen_height / screen_width );

	static const float PI = 3.14159265f;
	static const float line_width_scale = 0.5f;
	static const float line_width = 32.f / screen_height * line_width_scale;
	static const float z_offset = -0.00001f;
	static const float z_fix = 0.f;
	static const float w_fix = 1.f;
	
	static const float line_v_width = 32.f / 1024.f;

	for ( uint n = 0; n < 3; n++ )
	{
		input[ n ].Position /= input[ n ].Position.w;
//		input[ n ].Position.y /= input[ n ].Position.w;
		input[ n ].Position.z += z_offset;
		input[ n ].Position.w = w_fix;

		// debug
		// input[ n ].Position.z = z_fix;
	}

	for ( uint n = 0; n < 3; n++ )
	{
		uint m = ( n + 1 ) % 3;

		float ly = ( input[ m ].Position.y * screen_ratio ) - ( input[ n ].Position.y * screen_ratio );
		float lx = input[ m ].Position.x - input[ n ].Position.x;
		
		float line_index = uint( ( input[ n ].Position.x + input[ m ].Position.y + input[ n ].Position.z ) * 30.f ) % 3;
		float line_v_origin = ( line_index * line_v_width );

		float angle = atan2( ly, lx );
		angle += PI / 2.f;

		float dx = cos( angle ) * line_width * screen_ratio;
		float dy = sin( angle ) * line_width;

		GSPS_INPUT output[ 4 ];

		output[ 0 ].Position = input[ n ].Position + float4( -dx, -dy, 0.f, 0.f );
		output[ 0 ].TexCoord = float2( 0.f, line_v_origin );

		output[ 1 ].Position = input[ n ].Position + float4( +dx, +dy, 0.f, 0.f );
		output[ 1 ].TexCoord = float2( 0.f, line_v_origin + line_v_width );

		output[ 2 ].Position = input[ m ].Position + float4( -dx, -dy, 0.f, 0.f );
		output[ 2 ].TexCoord = float2( 1.f, line_v_origin );

		output[ 3 ].Position = input[ m ].Position + float4( +dx, +dy, 0.f, 0.f );
		output[ 3 ].TexCoord = float2( 1.f, line_v_origin + line_v_width );

		/*
		output[ 0 ].Position.z = z_fix;
		output[ 1 ].Position.z = z_fix;
		output[ 2 ].Position.z = z_fix;
		output[ 3 ].Position.z = z_fix;
		*/

		TriStream.Append( output[ 0 ] );
		TriStream.Append( output[ 1 ] );
		TriStream.Append( output[ 2 ] );
		TriStream.Append( output[ 3 ] );
		
		TriStream.RestartStrip();
	}
}

float4 ps( GSPS_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord );
}

float4 ps_line( GSPS_INPUT input ) : SV_Target
{
	return line_texture.Sample( texture_sampler, input.TexCoord );
}

float4 ps_debug( GSPS_INPUT input ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 0.25f );
}

BlendState NoBlend
{
	BlendEnable[ 0 ] = True;
};

BlendState Blend
{
	BlendEnable[ 0 ] = True;

	// SrcBlendAlpha = SRC_ALPHA;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};

DepthStencilState NoWriteDepth
{
	DepthWriteMask = ZERO;
};

DepthStencilState WriteDepth
{
	// DepthEnable = False;
	DepthWriteMask = ALL;
};

RasterizerState WireFrame
{
	FILLMODE = WIREFRAME;
};

technique11 main
{
	pass pass1
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs() ) );
		SetGeometryShader( NULL );
        // SetGeometryShader( CompileShader( gs_4_0, gs_pass() ) );
        SetPixelShader( CompileShader( ps_4_0, ps() ) );
    }

	pass pass_line
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_line() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_line() ) );

		// RASTERIZERSTATE = WireFrame;
	}
}