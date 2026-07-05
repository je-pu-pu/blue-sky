// 色保持リカラー（オフライン recolor_lab の HLSL 版）。
// t0 = source_texture : ニューラル出力（スタイル化結果）
// t1 = content_texture : 元シーン（スタイル化前）
// Lab 空間で L=暗化制限・a 控えめ・b 青方向増幅/黄カット。

Texture2D content_texture : register( t1 );  /// 元シーン（色の基準）

cbuffer ObjectConstantBuffer : register( b0 )
{
	float Blue;			// 青方向(db<0)の増幅		( default : 3.0 )
	float Yellow;		// 黄方向(db>0)の倍率		( default : 0.0 )
	float ASpread;		// a 軸のスプレッド			( default : 0.5 )
	float DarkClip;		// 暗化の許容(L 単位 0-100)	( default : 60.0 )
};

// --- sRGB <-> linear ---（色域外の負値で pow が NaN を出さないよう max(c,0) でガード）
float3 srgb_to_linear( float3 c )
{
	c = max( c, 0.0 );
	return float3(
		c.r <= 0.04045 ? c.r / 12.92 : pow( ( c.r + 0.055 ) / 1.055, 2.4 ),
		c.g <= 0.04045 ? c.g / 12.92 : pow( ( c.g + 0.055 ) / 1.055, 2.4 ),
		c.b <= 0.04045 ? c.b / 12.92 : pow( ( c.b + 0.055 ) / 1.055, 2.4 ) );
}
float3 linear_to_srgb( float3 c )
{
	c = max( c, 0.0 );
	return float3(
		c.r <= 0.0031308 ? c.r * 12.92 : 1.055 * pow( c.r, 1.0 / 2.4 ) - 0.055,
		c.g <= 0.0031308 ? c.g * 12.92 : 1.055 * pow( c.g, 1.0 / 2.4 ) - 0.055,
		c.b <= 0.0031308 ? c.b * 12.92 : 1.055 * pow( c.b, 1.0 / 2.4 ) - 0.055 );
}

static const float3 LAB_WHITE = float3( 0.95047, 1.0, 1.08883 );  // D65

float lab_f( float t )    { return t > 0.008856 ? pow( t, 1.0 / 3.0 ) : ( 7.787 * t + 16.0 / 116.0 ); }
float lab_finv( float t ) { float t3 = t * t * t; return t3 > 0.008856 ? t3 : ( t - 16.0 / 116.0 ) / 7.787; }

float3 rgb_to_lab( float3 srgb )
{
	float3 lin = srgb_to_linear( srgb );
	float3 xyz = float3(
		dot( lin, float3( 0.4124564, 0.3575761, 0.1804375 ) ),
		dot( lin, float3( 0.2126729, 0.7151522, 0.0721750 ) ),
		dot( lin, float3( 0.0193339, 0.1191920, 0.9503041 ) ) ) / LAB_WHITE;
	float fx = lab_f( xyz.x ), fy = lab_f( xyz.y ), fz = lab_f( xyz.z );
	return float3( 116.0 * fy - 16.0, 500.0 * ( fx - fy ), 200.0 * ( fy - fz ) );  // L:0-100, a, b
}

float3 lab_to_rgb( float3 lab )
{
	float fy = ( lab.x + 16.0 ) / 116.0;
	float fx = fy + lab.y / 500.0;
	float fz = fy - lab.z / 200.0;
	float3 xyz = float3( lab_finv( fx ), lab_finv( fy ), lab_finv( fz ) ) * LAB_WHITE;
	float3 lin = float3(
		dot( xyz, float3(  3.2404542, -1.5371385, -0.4985314 ) ),
		dot( xyz, float3( -0.9692660,  1.8760108,  0.0415560 ) ),
		dot( xyz, float3(  0.0556434, -0.2040259,  1.0572252 ) ) );
	return linear_to_srgb( lin );
}

float4 ps_post_effect_recolor( COMMON_POS_UV input ) : SV_Target
{
	float3 out_rgb = source_texture.Sample( texture_sampler, input.TexCoord ).rgb;   // ニューラル出力
	float3 con_rgb = content_texture.Sample( texture_sampler, input.TexCoord ).rgb;  // 元シーン

	float3 ol = rgb_to_lab( out_rgb );
	float3 cl = rgb_to_lab( con_rgb );

	float L  = cl.x + max( ol.x - cl.x, -DarkClip );     // 暗化のみ制限
	float da = ol.y - cl.y;
	float db = ol.z - cl.z;
	float a  = cl.y + da * ASpread;                       // a は控えめ
	float b  = cl.z + ( db < 0.0 ? db * Blue : db * Yellow );  // 青増幅・黄カット

	return float4( saturate( lab_to_rgb( float3( L, a, b ) ) ), 1.0 );
}

technique11 post_effect_recolor
{
	pass main
	{
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_recolor() ) );

		RASTERIZERSTATE = Default;
	}
}
