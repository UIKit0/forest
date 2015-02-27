// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "ColorChooser.h"
#include "cinder/Perlin.h"

using namespace ci;
using namespace std;


void ColorChooser::setup(ImageSourceRef image, Perlin* perlin, int numPoints)
{
    mPerlin = perlin;
    mPaletteImage = image;
    mPaletteTexture = mPaletteImage;
    mNumPoints = numPoints;
    mParameter = 0.0;
    mSpeed = 2.0;
}

void ColorChooser::update()
{
    mParameter += mSpeed;
}

void ColorChooser::seek(int steps)
{
    mParameter += mSpeed * steps * 100.0;
}

void ColorChooser::draw()
{
    Rectf outline = Rectf(0.0f, 0.0f, 1.0f, 1.0f);

    gl::color(1.0f, 1.0f, 1.0f, 1.0f);
    gl::disableAlphaBlending();
    gl::draw(mPaletteTexture, outline);

    gl::enableAlphaBlending();
    gl::color(0.4f, 0.4f, 0.4f, 1.0f);
    gl::drawStrokedRect(outline);

    // Point outline
    glPointSize(12.0f);
    gl::disable(GL_POINT_SPRITE);
    gl::begin(GL_POINTS);
    for (unsigned i = 0; i < mNumPoints; i++) {
        Vec2f point = getSamplePoint(i);
        gl::color(0.0f, 0.0f, 0.0f);
        gl::vertex(point);
    }
    gl::end();

    glPointSize(8.0f);
    gl::disable(GL_POINT_SPRITE);
    gl::begin(GL_POINTS);
    for (unsigned i = 0; i < mNumPoints; i++) {
        Vec2f point = getSamplePoint(i);
        gl::color(getSample(point));
        gl::vertex(point);
    }
    gl::end();
}

ColorA ColorChooser::sample(int index)
{
    return getSample(getSamplePoint(index));
}

Vec2f ColorChooser::getSamplePoint(int index)
{
    float f = mParameter * 3e-5;
    
    Vec2f center = Vec2f(mPerlin->fBm(f, 1.5f), mPerlin->fBm(f, 3.5f));
    center *= 1.33f;
    center += Vec2f(0.5f, 0.5f);
    
    float g = mPerlin->fBm(f, 5.5f) * M_PI;
    float s = 0.15f + mPerlin->fBm(f, 7.5f) * 0.2f;
    Vec2f harmonic(cosf(g) * s, sinf(g) * s);

    float t = index / float(mNumPoints - 1);
    return center + harmonic * (t * 2.0f - 1.0f);
}

ColorA ColorChooser::getSample(ci::Vec2f vec)
{
    return mPaletteImage.getPixel(Vec2i(vec.x * mPaletteImage.getWidth(), vec.y * mPaletteImage.getHeight()));
}
