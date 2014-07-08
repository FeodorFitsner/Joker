/**
 * @file
 * @copyright (C) 2012-2014 Phonations
 * @license http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
 */

#include "PhTools/PhDebug.h"
#include "PhGraphicTexturedRect.h"

PhGraphicTexturedRect::PhGraphicTexturedRect(int x, int y, int w, int h)
	: PhGraphicRect(x, y, w, h),
	_currentTexture(0),
	_previousTexture(0),
	_tu(1.0f),
	_tv(1.0f),
	_textureWidth(0),
	_textureHeight(0)
{

}

PhGraphicTexturedRect::~PhGraphicTexturedRect()
{
}

bool PhGraphicTexturedRect::initTextures() {
	// Have OpenGL generate a texture object handle for us
	if(_currentTexture == 0) {
		glGenTextures( 1, &_currentTexture );
		glGenTextures( 1, &_previousTexture );
		if(_currentTexture == 0 or _previousTexture == 0) {
			PHDEBUG << "glGenTextures() errored: is opengl context ready?";
			return false;
		}
	}
	return true;
}

/* Swap the current and previous textures to achieve double-buffering and avoid waiting
 * for the OpenGL driver to finish rendering.
 * More info here: https://developer.apple.com/library/mac/documentation/graphicsimaging/conceptual/opengl-macprogguide/opengl_texturedata/opengl_texturedata.html#//apple_ref/doc/uid/TP40001987-CH407-SW16
 */
void PhGraphicTexturedRect::swapTextures()
{
	GLuint tempTexture;

	tempTexture = _currentTexture;
	_currentTexture = _previousTexture;
	_previousTexture = tempTexture;
}

bool PhGraphicTexturedRect::createTextureFromSurface(SDL_Surface *surface)
{
	swapTextures();

	glEnable( GL_TEXTURE_2D );
	if(!initTextures()) {
		return false;
	}

	// Bind the texture object
	glBindTexture( GL_TEXTURE_2D, _currentTexture );

	GLenum textureFormat = 0;

	switch (surface->format->BytesPerPixel) {
	case 1:
		textureFormat = GL_ALPHA;
		break;
	case 3: // no alpha channel
		if (surface->format->Rmask == 0x000000ff)
			textureFormat = GL_RGB;
		else
			textureFormat = GL_BGR;
		break;
	case 4: // contains an alpha channel
		if (surface->format->Rmask == 0x000000ff)
			textureFormat = GL_RGBA;
		else
			textureFormat = GL_BGRA;

		break;
	default:
		PHDEBUG << "Warning: the image is not truecolor...";
		return false;
	}

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D( GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0,
	              textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

bool PhGraphicTexturedRect::createTextureFromARGBBuffer(void *data, int width, int height)
{
	swapTextures();

	glEnable( GL_TEXTURE_2D );

	if(!initTextures()) {
		return false;
	}

	// Bind the texture object
	glBindTexture( GL_TEXTURE_2D, _currentTexture );


	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
	              GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return true;
}

bool PhGraphicTexturedRect::createTextureFromRGBBuffer(void *data, int width, int height)
{
	swapTextures();

	glEnable( GL_TEXTURE_2D );

	if(!initTextures()) {
		return false;
	}

	if((width != _textureWidth) || (height != _textureHeight)) {
		_textureWidth = width;
		_textureHeight = height;

		PHDEBUG << QString("%1x%2").arg(width).arg(height);

		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, _previousTexture );

		// Edit the texture object's image data using the information SDL_Surface gives us
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		              GL_RGB, GL_UNSIGNED_BYTE, data);

		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, _currentTexture );

		// Edit the texture object's image data using the information SDL_Surface gives us
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		              GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, _currentTexture );
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return true;
}


bool PhGraphicTexturedRect::createTextureFromYUVBuffer(void *data, int width, int height)
{
	swapTextures();

	glEnable( GL_TEXTURE_2D );

	if(!initTextures()) {
		return false;
	}

	// Bind the texture object
	glBindTexture( GL_TEXTURE_2D, _currentTexture );

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
	              GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}


void PhGraphicTexturedRect::draw()
{
	PhGraphicRect::draw();

	glBindTexture(GL_TEXTURE_2D, _currentTexture);

	glEnable(GL_TEXTURE_2D);

	//        (0,0) ------ (1,0)
	//          |            |
	//          |            |
	//        (0,1) ------ (1,1)

	glBegin(GL_QUADS);  //Begining the cube's drawing
	{
		glTexCoord3f(0, 0, 1);      glVertex3i(this->x(),      this->y(), this->z());
		glTexCoord3f(_tu, 0, 1);    glVertex3i(this->x() + this->width(), this->y(), this->z());
		glTexCoord3f(_tu, _tv, 1);  glVertex3i(this->x() + this->width(), this->y() + this->height(),  this->z());
		glTexCoord3f(0, _tv, 1);    glVertex3i(this->x(),      this->y() + this->height(),  this->z());
	}
	glEnd();


	glDisable(GL_TEXTURE_2D);
}

void PhGraphicTexturedRect::setTextureCoordinate(float tu, float tv)
{
	_tu = tu;
	_tv = tv;
}

