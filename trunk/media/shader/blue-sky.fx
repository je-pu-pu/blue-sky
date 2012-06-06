float4x4 WorldViewProjection;
float4 ambient_color = { 1.f, 0.5f, 0.5f, 1.f };
float4 object_color = { 1.f, 1.f, 1.f, 1.f };
float4 add_color = { 0.f, 0.f, 0.f, 0.f };
float brightness = 0.f;
float lens_ratio = 0.f;

void vs_main(
	in float4 in_p : POSITION,
	in float4 in_d : COLOR0,
	in float4 in_t : TEXCOORD0,
	out float4 out_p : POSITION,
	out float4 out_d : COLOR0,
	out float4 out_t : TEXCOORD0,
	out float out_f : FOG )
{
	out_p = mul( in_p, WorldViewProjection );
	out_d = ambient_color * object_color;

	out_t = in_t;
	
	out_f = 1.f; // - out_p.z / 500;
	// out_d.a = 1.f - out_p.z / 200;
}

void vs_object_color(
	in float4 in_p : POSITION,
	out float4 out_p : POSITION,
	out float4 out_d : COLOR0 )
{
	out_p = mul( in_p, WorldViewProjection );
	out_d = object_color;
}

texture tex0;
sampler2D s = sampler_state
{
	texture = null;

	mipfilter = POINT;
	minfilter = POINT;
	magfilter = POINT;

	mipfilter = ANISOTROPIC;
	minfilter = ANISOTROPIC;
	magfilter = ANISOTROPIC;
//	MaxAnisotropy = 2;

//	mipfilter = LINEAR;
//	minfilter = LINEAR;
// 	magfilter = LINEAR;


	AddressU = CLAMP;
	AddressV = CLAMP;

//	AddressU = WRAP;
//	AddressV = WRAP;
};

void ps_main(
	in float4 in_d : COLOR0,
	in float2 in_t : TEXCOORD0,
	in float2 vpos : VPOS,
	out float4 out_d : COLOR0 )
{
	float4 color = add_color * add_color.a;
	color.a = 0;

	out_d = in_d * tex2D( s, in_t ) + color;
	out_d.rgb += brightness;
}

void ps_pass(
	in float4 in_d : COLOR0,
	out float4 out_d : COLOR0 )
{
	out_d = in_d;
}

void ps_brightness(
	in float4 in_d : COLOR0,
	out float4 out_d : COLOR0 )
{
	out_d = in_d;
	out_d.rgb += brightness;
}

void vs_pass(
	in float4 in_p : POSITION,
	in float4 in_d : COLOR0,
	in float4 in_t : TEXCOORD0,
	out float4 out_p : POSITION,
	out float4 out_d : COLOR0,
	out float4 out_t : TEXCOORD0 )
{
	out_p.x = in_p.x - 0.5f / 720.f * 2.f;
	out_p.y = in_p.y + 0.5f / 480.f * 2.f;
	out_p.z = in_p.z;
	out_p.w = in_p.w;

	out_d = in_d;
	out_t = in_t;
}

sampler2D fish_eye_tex = sampler_state
{
	texture = null;

	mipfilter = LINEAR;
	minfilter = LINEAR;
	magfilter = LINEAR;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

void ps_fish_eye_lens(
	in float4 in_d : COLOR0,
	in float2 in_t : TEXCOORD0,
	in float2 vpos : VPOS,
	out float4 out_d : COLOR0 )
{
	float len = sqrt( pow( in_t.x, 2.f ) + pow( in_t.y, 2.f ) );

	in_t -= 0.5f;
	in_t *= 2.f;
	
	// in_t.x = ( in_t.x * 3 + ( 1 - pow( in_t.x, 3.f ) ) - 1 ) * 0.5f;
	// in_t.y = ( in_t.y * 3 + ( 1 - pow( in_t.y, 3.f ) ) - 1 ) * 0.5f;

	// in_t.x = cos( radians( length( in_t ) * 90.f ) );
	// in_t.y = sin( radians( length( in_t ) * 90.f ) );

	in_t.y = in_t.y * ( 1.f - lens_ratio ) + sin( radians( in_t.y * 90.f ) ) * lens_ratio;

	in_t /= 2.f;
	in_t += 0.5f;

	out_d = in_d * tex2D( fish_eye_tex, in_t );
}

void ps_crazy_lens(
	in float4 in_d : COLOR0,
	in float2 in_t : TEXCOORD0,
	out float4 out_d : COLOR0 )
{
	in_t -= 0.5f;
	in_t *= 2.f;

//	in_t.x = in_t.x * ( 1.f - lens_ratio ) + sin( radians( pow( in_t.x, 3.f ) * 90.f ) ) * lens_ratio;
	in_t.y = in_t.y * ( 1.f - lens_ratio ) + sin( radians( pow( in_t.y, 3.f ) * 90.f ) ) * lens_ratio;

	in_t /= 2.f;
	in_t += 0.5f;

	out_d = in_d * tex2D( fish_eye_tex, in_t );
}

technique technique_0
{
	pass pass_0
	{
		VertexShader = compile vs_2_0 vs_main();
		PixelShader = compile ps_2_0 ps_main();

		AlphaBlendEnable = TRUE;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;

		// FogTableMode = EXP2;
		// FogColor = 0xFF2b2b7e;
		// FogDensity = 0.002f;
	}

	pass pass_fish_eye_lens
	{
		VertexShader = compile vs_2_0 vs_pass();
		PixelShader = compile ps_2_0 ps_fish_eye_lens();

		ZEnable = FALSE;
		FogEnable = FALSE;
	}

	pass pass_crazy_lens
	{
		VertexShader = compile vs_2_0 vs_pass();
		PixelShader = compile ps_2_0 ps_crazy_lens();

		ZEnable = FALSE;
		FogEnable = FALSE;
	}

	pass pass_3
	{
		VertexShader = compile vs_2_0 vs_object_color();
		PixelShader = compile ps_2_0 ps_brightness();

		AlphaBlendEnable = TRUE;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;

		ZFunc = GREATER;
	}
}