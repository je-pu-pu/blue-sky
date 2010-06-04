float4x4 WorldViewProjection;
float a;

void vs_main( in float4 in_p : POSITION, in float4 in_d : COLOR0, out float4 out_p : POSITION, out float4 out_d : COLOR0 )
{
	out_p = mul( in_p, WorldViewProjection );
	out_d = in_d;
}

void ps_main( in float4 in_d : COLOR0, in float2 vpos : VPOS, out float4 out_d : COLOR0 )
{
	out_d = in_d;
	return;

	/*
	// if ( vpos[0] % 2 == 1 )
	if ( 1 )
	{
		// out_d = in_d;
		out_d[0] = sin( in_d[0] + a + vpos[0] );
		out_d[1] = sin( in_d[1] + a + vpos[1] );
		out_d[2] = sin( in_d[2] + a );
		out_d[3] = sin( in_d[3] + a );

		depth = 0;
	}
	else
	{
		out_d[0] = 1;
		out_d[1] = 0;
		out_d[2] = 0;
		out_d[3] = 1;

		depth = 0;
	}
	*/
}