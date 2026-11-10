#include "pch.h"

// シェーダーが参照し続けるテクスチャ（TextureSlot）のテスト。管理しているテクスチャはハンドルで持ち、解放されたら引けない
#include "FakeGraphics.h"

#include <blue_sky/graphics/TextureSlot.h>

namespace {

using blue_sky::graphics::TextureSlot;
using test_graphics::FakeResourceLoader;
using test_graphics::FakeTexture;

TEST( TextureSlotTest, 管理しているテクスチャはハンドルで持ち引ける )
{
	FakeResourceLoader loader;
	auto* t = loader.textures.create_named< FakeTexture >( "a" );

	TextureSlot slot( loader, t );

	EXPECT_TRUE( slot.is_managed() );
	EXPECT_EQ( slot.get( loader ), t );
	EXPECT_FALSE( slot.is_lost( loader ) );
}

TEST( TextureSlotTest, 解放したテクスチャは引けず落ちない )
{
	FakeResourceLoader loader;
	auto* t = loader.textures.create_named< FakeTexture >( "a" );

	TextureSlot slot( loader, t );
	loader.textures.remove_named( "a" );	// 解放する（以前の生ポインタはここで無効なアドレスを指した）

	EXPECT_EQ( slot.get( loader ), nullptr );
	EXPECT_TRUE( slot.is_lost( loader ) );
}

TEST( TextureSlotTest, 差し替えたテクスチャは新しい中身を引く )
{
	FakeResourceLoader loader;
	auto* t = loader.textures.create_named< FakeTexture >( "a" );

	TextureSlot slot( loader, t );

	auto replacement = std::make_unique< FakeTexture >( 2, 2 );
	auto* r = replacement.get();
	ASSERT_TRUE( loader.textures.replace( loader.textures.handle_of( t ), std::move( replacement ) ) );

	EXPECT_EQ( slot.get( loader ), r );
	EXPECT_FALSE( slot.is_lost( loader ) );
}

TEST( TextureSlotTest, 管理していないテクスチャはそのまま持つ )
{
	FakeResourceLoader loader;
	FakeTexture render_target;		// 描画先など。持ち主が別にいる

	TextureSlot slot( loader, & render_target );

	EXPECT_FALSE( slot.is_managed() );
	EXPECT_EQ( slot.get( loader ), & render_target );
	EXPECT_FALSE( slot.is_lost( loader ) );
}

TEST( TextureSlotTest, 空は引けないが解放されたとは扱わない )
{
	FakeResourceLoader loader;
	TextureSlot slot;

	EXPECT_EQ( slot.get( loader ), nullptr );
	EXPECT_FALSE( slot.is_lost( loader ) );

	auto* t = loader.textures.create_named< FakeTexture >( "a" );
	slot.set( loader, t );
	slot.set( loader, nullptr );	// 外す

	EXPECT_EQ( slot.get( loader ), nullptr );
	EXPECT_FALSE( slot.is_lost( loader ) );
}

} // namespace
