cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
};

struct GSPSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

GSPSInput vs( float4 Pos : POSITION, uint vertex_id : SV_VertexID )
{
	static float4 colors[ 3 ] = {
		{ 1.f, 0.f, 0.f, 0.5f },
		{ 0.f, 1.f, 0.f, 0.5f },
		{ 0.f, 0.f, 1.f, 0.5f },
	};

	GSPSInput output = { Pos, colors[ vertex_id % 3 ] };

	output.Position = mul( output.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );

    return output;
}

[maxvertexcount(12)]
void gs( triangle GSPSInput input[3], inout TriangleStream<GSPSInput> TriStream, uint patchID : SV_PrimitiveID )
{
	TriStream.Append( input[ 0 ] );
	TriStream.Append( input[ 1 ] );
	TriStream.Append( input[ 2 ] );
}

[maxvertexcount(12)]
void gs_line( triangle GSPSInput input[3], inout TriangleStream<GSPSInput> TriStream, uint patchID : SV_PrimitiveID )
{
	static const float PI = 3.14159265f;
	
	for ( uint n = 0; n < 3; n++ )
	{
		uint m = ( n + 1 ) % 3;
		float angle = atan2( input[ m ].Position.y - input[ n ].Position.y, input[ m ].Position.x - input[ n ].Position.x );
		angle += PI / 2.f;

		float dx = cos( angle ) * 0.05f;
		float dy = sin( angle ) * 0.05f;

		GSPSInput output[ 4 ] = { input[ n ], input[ n ], input[ n ], input[ n ] };

		output[ 0 ].Position = input[ n ].Position + float4( -dx, -dy, 0.f, 0.f );
		output[ 1 ].Position = input[ n ].Position + float4( +dx, +dy, 0.f, 0.f );
		output[ 2 ].Position = input[ m ].Position + float4( -dx, -dy, 0.f, 0.f );
		output[ 3 ].Position = input[ m ].Position + float4( +dx, +dy, 0.f, 0.f );

		TriStream.Append( output[ 0 ] );
		TriStream.Append( output[ 1 ] );
		TriStream.Append( output[ 2 ] );
		TriStream.Append( output[ 3 ] );
		
		TriStream.RestartStrip();
	}
}

float4 ps( GSPSInput input ) : SV_Target
{
	return input.Color;
}

technique11 main
{
    pass pass1
    {
        SetVertexShader( CompileShader( vs_4_0, vs() ) );
        SetGeometryShader( CompileShader( gs_4_0, gs() ) );
        SetPixelShader( CompileShader( ps_4_0, ps() ) );
    }
}