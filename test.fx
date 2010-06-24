float4x4 WorldViewProjection;

void vs_main(
	in float4 in_p : POSITION,
	in float4 in_n : NORMAL0,
	in float4 in_d : COLOR0,
	in float4 in_t : TEXCOORD0,
	out float4 out_p : POSITION,
	out float4 out_d : COLOR0,
	out float4 out_t : TEXCOORD0,
	out float  out_f : FOG )
{
	out_p = mul( in_p, WorldViewProjection );
//	out_p[0];

	float4 diffuse ={ 0.6f, 1.f, 0.6f, 1.f };
	float4 light = { -0.25f, -0.75f, 0.5f, 1.f };
	out_d = diffuse * max( 0, dot( in_n, normalize( light ) ) ) + in_d;

	out_d[0] += abs( in_p[0] ) * 0.01 * 0.5;
	out_d[1] += abs( in_p[1] ) * 0.3  * 0.5;
	out_d[2] += abs( in_p[2] ) * 0.1  * 0.5;

	out_t = in_t;

	out_f = 1.f;
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

sampler TexSampler = sampler_state
{
	MipFilter = NISOTROPIC;
	MinFilter = ANISOTROPIC;
	MagFilter = ANISOTROPIC;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

void ps_main(
	in float4 in_d : COLOR0,
	in float2 in_t : TEXCOORD0,
	in float2 vpos : VPOS,
	out float4 out_d : COLOR0 )
{
	in_d[3] = 1.f;
	out_d = tex2D( TexSampler, in_t );
	// out_d = in_d;
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