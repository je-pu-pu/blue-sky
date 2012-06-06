
#include <vg/vgu.h>
#include <gl/glut.h>
#include <iostream>

#include <windows.h>

#include <math.h>

#include <list>

static BITMAP			bmp;
static std::list< HBITMAP >	hbmp_list;

const double M_PI = 3.14159265358979323846;

void glut_display()
{
	glClearColor( 0.8f, 0.8f, 0.9f, 1.f );
	glClear( GL_COLOR_BUFFER_BIT );

	// std::cout << "d" << std::endl;

	VGfloat white[] = {1,1,1,1};

		glColor4f( 1.f, 0.f, 0.f, 1.f );
	/*ATH_USER_TO_SURFACE);
	*/

	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_POLYGON_SMOOTH );

	static float a = 0;
	const float x = M_PI * 2 / 4;

	a += 0.0001f;
	float b = a + x;
	float c = b + x;
	float d = c + x;
		
	glLineWidth( 5 );

	for ( int n = 0; n < 1000; n++ )
	{
		glBegin( GL_POLYGON );
		glColor4f( 1.f, 0.f, 0.f, 1.f );

		glTexCoord2f( 0.0, 0.0 );
		glVertex2f( cos( a ) * 5.f, sin( a ) * 5.f );

		glTexCoord2f( 1.0, 0.0 );
		glVertex2f( cos( b ) * 5.f, sin( b ) * 5.f );

		glTexCoord2f( 1.0, 1.0 );
		glVertex2f( cos( c ) * 5.f, sin( c ) * 5.f );

		glTexCoord2f( 0.0, 1.0 );
		glVertex2f( cos( d ) * 5.f, sin( d ) * 5.f );
		glEnd();
	}

	/*
	VGPath path = vgCreatePath( VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL );
	vguEllipse( path, 200.f, 200.f, 300.f, 300.f );
	vgDrawPath( path, VG_FILL_PATH );
	*/

	// std::cout << path << std::endl;

	/*
	glBegin( GL_POLYGON );
	glColor4f( 1.f, 0.f, 0.f, 1.f );
	glVertex2f( 10, 20 );	
	glVertex2f( 100, 20 );
	glVertex2f( 100, 200 );
	glEnd();

	
	*/


	glFlush();

	// glutSwapBuffers();
}

void glut_idle()
{
	glutPostRedisplay();
}

void glut_reshape( int w, int h )
{
	glViewport( 0, 0, w, h );

	float r = static_cast< float >( w ) / static_cast< float >( h );

	glLoadIdentity();
	glOrtho( -5 * r, 5 * r, -5, 5, -1.0, 1.0 );
}

/**
 *
 */
int main( int argc, char** argv )
{
	const int w = 640;
	const int h = 480;

	glutInit( & argc, argv );
	glutInitWindowSize( w, h );
	glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA );
	// glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_STENCIL | GLUT_MULTISAMPLE );

	glutCreateWindow( "vgu test" );

	glutDisplayFunc( glut_display );
	glutIdleFunc( glut_idle );
	glutReshapeFunc( glut_reshape );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	// load texture
	HBITMAP hbmp;
	hbmp = (HBITMAP) LoadImage( NULL, "pen1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION );
	if( hbmp == NULL ) return false;
	GetObject( hbmp, sizeof(BITMAP), &bmp );
	hbmp_list.push_back( hbmp );
	// swap R n B
	RGBTRIPLE *cl = (RGBTRIPLE*)bmp.bmBits;
	for( int i = 0; i < bmp.bmWidth * bmp.bmHeight; i++ ){
		std::swap( cl->rgbtRed, cl->rgbtBlue );
		cl++;
	}
	// settings
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
	glEnable( GL_TEXTURE_2D );

	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_RGB, GL_UNSIGNED_BYTE, bmp.bmBits );

	// vgCreateContextSH( w, h );

	glutMainLoop();

	return 0;
}