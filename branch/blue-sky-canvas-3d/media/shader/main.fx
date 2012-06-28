struct GSPSInput
{
	float4 Position : SV_POSITION;
	// float4 Color : SV_COLOR;
};

GSPSInput vs( float4 Pos : POSITION ) : SV_POSITION
{
	GSPSInput output;
	output.Position = Pos;

	/*
	float4 colors[ 3 ] = {
		{ 1.f, 0.f, 0.f, 1.f },
		{ 0.f, 1.f, 0.f, 1.f },
		{ 0.f, 0.f, 1.f, 1.f },
	};

    output.Color = colors[ id % 3 ];
	*/

    return output;
}

[maxvertexcount(6)]
void gs( triangle GSPSInput input[3], inout TriangleStream<GSPSInput> TriStream )
{
	for ( int n = 0; n < 3; n++ )
	{
		GSPSInput output = input[ n ];
		TriStream.Append( output );
	}

	TriStream.RestartStrip();

	for ( int n = 0; n < 3; n++ )
	{
		GSPSInput output = input[ n ];

		float4 x = { 0.5, 0.5, 0.5, 0.5 };

		output.Position = input[ n ].Position + x;
		
		TriStream.Append( output );
	}

	TriStream.RestartStrip();
}

float4 ps( GSPSInput input ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 1.f );
	
	// return input.Color;
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