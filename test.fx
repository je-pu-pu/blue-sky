float4x4 WorldViewProjection;
float fog;

void vs_main(
	in float4 in_p : POSITION,
	in float4 in_n : NORMAL0,
	in float4 in_d : COLOR0,
	in float4 in_t : TEXCOORD0,
	out float4 out_p : POSITION,
	out float4 out_d : COLOR0,
	out float4 out_t : TEXCOORD0 )
{
	out_p = mul( in_p, WorldViewProjection );

	float4 ambient = { 1.f, 1.f, 1.f, 1.f };
	// float4 ambient = { 1.0f, 0.8f, 0.4f, 1.f };
	// float4 ambient = { 0.4f, 0.4f, 0.6f, 1.f };
	out_d = ambient;

	out_t = in_t;

	// out_f = 0.5f;
}

void vs_main_vc(
	in float4 in_p : POSITION,
	in float4 in_d : COLOR0,
	out float4 out_p : POSITION,
	out float4 out_d : COLOR0 )
{
	out_p = mul( in_p, WorldViewProjection );
	out_d = in_d;
}

texture tex0;
sampler2D s = sampler_state
{
	texture = null;
	mipfilter = LINEAR;
	minfilter = LINEAR;
	magfilter = LINEAR;

	AddressU = xxx;
	AddressV = xxx;
};

void ps_main(
	in float4 in_d : COLOR0,
	in float2 in_t : TEXCOORD0,
	in float2 vpos : VPOS,
	out float4 out_d : COLOR0 )
{
	// in_d[3] = 1.f;
	out_d = in_d * tex2D( s, in_t );
	// out_d = ;
	return;
	
	out_d = in_d;
	
	/*
	float g = ( in_d[0] + in_d[1] + in_d[2] ) / 3;
	float4 gray = { g, g, g, in_d[3] };
	
	float g_rate = clamp( sqrt( pow( abs( ( 720 / 2 ) - vpos[0] ), 2 ) + pow( abs( ( 480 / 2 ) - vpos[1] ), 2 ) ) / 480, 0, 1 );
	out_d = in_d * ( 1 - g_rate ) + g * g_rate;
	// out_d = g_rate;
	*/

	/*
	out_d[ 0 ] += sqrt( pow( abs( ( 720 / 2 ) - vpos[0] ), 2 ) + pow( abs( ( 480 / 2 ) - vpos[1] ), 2 ) ) / ( 300 ) * 0.5;
	out_d[ 1 ] += sqrt( pow( abs( ( 720 / 2 ) - vpos[0] ), 2 ) + pow( abs( ( 480 / 2 ) - vpos[1] ), 2 ) ) / ( 480 ) * 0.5;
	out_d[ 2 ] += sqrt( pow( abs( ( 720 / 2 ) - vpos[0] ), 2 ) + pow( abs( ( 480 / 2 ) - vpos[1] ), 2 ) ) / ( 880 ) * 0.5;
	*/

	/*
	if ( sin( in_d[0] ) < 0 && sin( in_d[2] ) < 0 )
	{
		out_d = in_d;
	}
	else
	{
		out_d[0] = in_d[0] - vpos[1] / 1000;
		out_d[1] = in_d[1] - vpos[0] / 1000;
		out_d[2] = in_d[2] - vpos[0] / 1000;
	}
	*/

	/*
	out_d[0] = in_d[0] - vpos[1] / 1000;
	out_d[1] = in_d[1] - vpos[1] / 1000;
	out_d[2] = in_d[2] - vpos[0] / 1000;
	out_d[3] = in_d[2];
	*/
}