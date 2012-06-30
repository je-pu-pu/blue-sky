Texture2D model_texture : register( t0 );
SamplerState texture_sampler : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
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
	// uint VertexID : VERTEX_ID;
};

GSPS_INPUT vs( VS_INPUT input )
{
	GSPS_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	output.TexCoord = input.TexCoord;
	// output.VertexID = vertex_id;

    return output;
}

[maxvertexcount(12)]
void gs_line( triangle GSPS_INPUT input[3], inout TriangleStream<GSPS_INPUT> TriStream )
{
	static const float PI = 3.14159265f;
	static const float line_width = 0.05f;
	static const float z_offset = 0.f; // -0.0001f;
	static const float z_fix = 0.f;
	
	for ( uint n = 0; n < 3; n++ )
	{
		uint m = ( n + 1 ) % 3;

		float angle = atan2( input[ m ].Position.y - input[ n ].Position.y, input[ m ].Position.x - input[ n ].Position.x );
		angle += PI / 2.f;

		float dx = cos( angle ) * line_width;
		float dy = sin( angle ) * line_width;

		GSPS_INPUT output[ 4 ];

		output[ 0 ].Position = input[ n ].Position + float4( -dx, -dy, z_offset, 0.f );
		output[ 0 ].TexCoord = float2( 0.f, 0.f );
		// output[ 0 ].VertexID = 0;

		output[ 1 ].Position = input[ n ].Position + float4( +dx, +dy, z_offset, 0.f );
		output[ 1 ].TexCoord = float2( 0.f, 1.f );
		// output[ 1 ].VertexID = 0;

		output[ 2 ].Position = input[ m ].Position + float4( -dx, -dy, z_offset, 0.f );
		output[ 2 ].TexCoord = float2( 1.f, 0.f );
		// output[ 2 ].VertexID = 0;

		output[ 3 ].Position = input[ m ].Position + float4( +dx, +dy, z_offset, 0.f );
		output[ 3 ].TexCoord = float2( 1.f, 1.f );
		// output[ 3 ].VertexID = 0;

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
	// return float4( 1.f, 0.f, 0.f, 0.5f );
	return model_texture.Sample( texture_sampler, input.TexCoord );
}

float4 ps_dark( GSPS_INPUT input ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 1.f );
}

BlendState NoBlend
{
	BlendEnable[ 0 ] = False;
};

BlendState Blend
{
	BlendEnable[ 0 ] = True;
	SrcBlend = SRC_ALPHA;
	DestBlend = SRC_ALPHA;
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

technique11 main
{
    pass pass1
    {
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs() ) );
        SetGeometryShader( 0 );
        SetPixelShader( CompileShader( ps_4_0, ps() ) );
    }

	pass pass_line
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_line() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_dark() ) );
	}
}