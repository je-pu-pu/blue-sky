#include <irrlicht.h>
#include <iostream>
#include <vector>

using namespace irr;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

int main()
{
	// create device
	IrrlichtDevice* device = createDevice( video::EDT_OPENGL, core::dimension2d<u32>( 800, 600 ), 16, false );
	// IrrlichtDevice* device = createDevice( video::EDT_DIRECT3D9, core::dimension2d<u32>( 800, 600 ), 16, false );
	if ( device == 0 ) return 1;

	device->setWindowCaption( L"blu-sky" );

	video::IVideoDriver* driver = device->getVideoDriver();

	// video::ITexture* images = driver->getTexture( "circle.png" );
	video::ITexture* images = driver->getTexture( "brush5.png" );
	// driver->makeColorKeyTexture( images, core::position2d<s32>(0,0) );

	gui::IGUIFont* font = device->getGUIEnvironment()->getBuiltInFont();
	// gui::IGUIFont* font2 = device->getGUIEnvironment()->getFont("../../media/fonthaettenschweiler.bmp");

	core::rect<s32> imp1(349,15,385,78);
	core::rect<s32> imp2(387,15,423,78);

	driver->getMaterial2D().TextureLayer[0].BilinearFilter = false;
	driver->getMaterial2D().TextureLayer[0].TrilinearFilter = true;
	driver->getMaterial2D().AntiAliasing=video::EAAM_QUALITY;
	driver->getMaterial2D().setTexture( 0, images );
	// driver->getMaterial2D().ColorMaterial(
	driver->getMaterial2D().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	driver->getMaterial2D().ZBuffer = video::ECFN_LESS;
	// driver->getMaterial2D().ZWriteEnable = true;
	// driver->setAllowZWriteOnTransparent( true );
	// driver->getMaterial2D().MaterialType = video::EMT_ONETEXTURE_BLEND;

	// driver->getMaterial2D().getTexture( 0 )->regenerateMipMapLevels();

	driver->setMaterial( driver->getMaterial2D() );
	// driver->setRenderStates2DMode( true, true, true );

	std::cout << "hasmap : " << driver->getMaterial2D().getTexture( 0 )->hasMipMaps() << std::endl;
	std::cout << "zbuf : " << ( int ) driver->getMaterial2D().ZBuffer << std::endl;

	const float z = 10.f;
	static float h = 1.f;
	static float x = 100.f;
	static float y = 100.f;;

	std::vector<video::S3DVertex> vs;
	std::vector<u32> is;

	// const int circle_count = 10000 / 2;
	const int circle_count = 100000;

	vs.reserve( circle_count * 4 );
	is.reserve( circle_count * 4 );

	for ( int n = 0; n < circle_count; n++ )
	{
		vs.push_back( video::S3DVertex( 0, 0, 0, 0, 0, 0, video::SColor( 0, 0xFF, 0xFF, 0xFF ), 0, 1 ) );
		vs.push_back( video::S3DVertex( 0, 0, 0, 0, 0, 0, video::SColor( 0, 0, 0, 0 ), 1, 1 ) );
		vs.push_back( video::S3DVertex( 0, 0, 0, 0, 0, 0, video::SColor( 0, 0xFF, 0xAA, 0x11 ), 1, 0 ) );
		vs.push_back( video::S3DVertex( 0, 0, 0, 0, 0, 0, video::SColor( 0, 0xFF, 0xAA, 0x11 ), 0, 0 ) );

		is.push_back( n * 4 + 0 );
		is.push_back( n * 4 + 1 );
		is.push_back( n * 4 + 2 );
		is.push_back( n * 4 + 3 );
	}

	core::matrix4 m;
	// m.setRotationCenter( core::vector3df( 100.f, 100.f, 0.f ), core::vector3df( 0.f, 0.f, 120.f ) );
	m.setRotationCenter( core::vector3df( 100.f, 100.f, 0.f ), core::vector3df( 0.f, 0.f, 0.f ) );
	m.setRotationDegrees( core::vector3df( 0.f, 0.f, 15.f ) );
	// driver->setTransform( video::ETS_WORLD, m );

	vs[ 0 ].Pos = core::vector3df( 100, 100, 0 );
	vs[ 1 ].Pos = core::vector3df( 500, 100, 0 );
	vs[ 2 ].Pos = core::vector3df( 500, 200, 0 );
	vs[ 3 ].Pos = core::vector3df( 100, 200, 0 );
	
	vs[ 0 ].Color = video::SColor( 255, 0xFF, 0xAA, 0x11 );
	vs[ 1 ].Color = video::SColor( 255, 0xFF, 0xAA, 0x11 );
	vs[ 2 ].Color = video::SColor( 255, 0xFF, 0xAA, 0x11 );
	vs[ 3 ].Color = video::SColor( 255, 0xFF, 0xAA, 0x11 );

	m.rotateVect( vs[ 0 ].Pos );
	m.rotateVect( vs[ 1 ].Pos );
	m.rotateVect( vs[ 2 ].Pos );
	m.rotateVect( vs[ 3 ].Pos );

	while ( device->run() && driver )
	{
		if ( device->isWindowActive() )
		{
			static float a = 0;

			a += 0.01f;

			u32 time = device->getTimer()->getTime();

			driver->beginScene( true, true, video::SColor( 255, 0xEE, 0xEE, 0xFF ) );

			float nn = a;

			core::matrix4 m;
			// m.setRotationCenter( core::vector3df( 100.f, 100.f, 0.f ), core::vector3df( 0.f, 0.f, 120.f ) );
			m.setRotationRadians( core::vector3df( 0.f, 0.f, a ) );
			m.setRotationCenter( core::vector3df( 100.f, 100.f, 0.f ), core::vector3df( -1-0.f, -100.f, a ) );
			// driver->setTransform( video::ETS_WORLD, m );

			m.setTranslation( core::vector3df( 100.f, 100.f, 100.f ) );
			driver->setTransform( video::ETS_WORLD, m );

			vs[ 0 ].Pos = core::vector3df( 100, 100, 0 );
			vs[ 1 ].Pos = core::vector3df( 500, 100, 0 );
			vs[ 2 ].Pos = core::vector3df( 500, 200, 0 );
			vs[ 3 ].Pos = core::vector3df( 100, 200, 0 );

			m.rotateVect( vs[ 0 ].Pos );
			m.rotateVect( vs[ 1 ].Pos );
			m.rotateVect( vs[ 2 ].Pos );
			m.rotateVect( vs[ 3 ].Pos );

			const float pen_interval = 3.1f;
			const float max_pen_width = 100.f;
			const float min_pen_width = 2.5f;
			

			float nnn = nn;
			float nnnn = sin( a );

			for ( int n = 4; n < circle_count; n++ )
			{
				nn += 0.01f;
				nnn += 0.082f;
				nnnn += 0.099f;

				float xx = x + n * pen_interval;
				float yy = y + n * 1.1f;
				float zz = -n * 0.01f;
				float hh = h + ( cos( nn ) * sin( nnn ) + 1.5f ) * ( max_pen_width * 0.2f );
				
				if ( hh > max_pen_width ) hh = max_pen_width;
				if ( hh < min_pen_width ) hh = min_pen_width;

				vs[ n * 4 + 0 ].Pos = core::vector3df( xx - hh, yy - hh, zz );
				vs[ n * 4 + 1 ].Pos = core::vector3df( xx + hh, yy - hh, zz );
				vs[ n * 4 + 2 ].Pos = core::vector3df( xx + hh, yy + hh, zz );
				vs[ n * 4 + 3 ].Pos = core::vector3df( xx - hh, yy + hh, zz );

				vs[ n * 4 + 0 ].Pos.rotateXYBy( a );
				vs[ n * 4 + 1 ].Pos.rotateXYBy( a );
				vs[ n * 4 + 2 ].Pos.rotateXYBy( a );
				vs[ n * 4 + 3 ].Pos.rotateXYBy( a );
			}

			driver->draw2DVertexPrimitiveList( & vs[ 0 ], vs.size(), & is[ 0 ], vs.size() / 4, video::EVT_STANDARD, scene::EPT_QUADS, video::EIT_32BIT );
			// driver->drawVertexPrimitiveList( & vs[ 0 ], vs.size(), & is[ 0 ], vs.size() / 4, video::EVT_STANDARD, scene::EPT_QUADS, video::EIT_32BIT );

			driver->endScene();

			static int lastFPS = -1;
			int fps = driver->getFPS();

			if (lastFPS != fps)
			{
				core::stringw tmp(L"Movement Example - Irrlicht Engine [");
				tmp += driver->getName();
				tmp += L"] fps: ";
				tmp += fps;

				device->setWindowCaption(tmp.c_str());
				lastFPS = fps;
			}
		}
	}

	device->drop();

	return 0;
}
