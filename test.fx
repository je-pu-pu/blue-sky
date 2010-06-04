float4x4 WorldViewProjection;
float a;

void vs_main( in float4 in_p : POSITION, in float4 in_d : COLOR0, out float4 out_p : POSITION, out float4 out_d : COLOR0 )
{
	out_p = mul( in_p, WorldViewProjection );
	out_p[0];

	out_d = in_d;
}

void ps_main( in float4 in_d : COLOR0, in float2 vpos : VPOS, out float4 out_d : COLOR0 )
{
	out_d = in_d;

	if ( int( vpos[0] / 4 ) % 2 == 0 || int( vpos[1] / 8 ) % 2 == 0 )
	{
		out_d = in_d;
	}
	else
	{
		out_d[0] = in_d[0] - vpos[1] / 1000;
		out_d[1] = in_d[1] - vpos[1] / 1000;
		out_d[2] = in_d[2] - vpos[0] / 1000;
	}
}