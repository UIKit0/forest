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
    seek(1);
}

void ColorChooser::seek(int steps)
{
    // Adjust the parameter at a rate such that the fastest endpoint moves at mSpeed
    mParameter += (mSpeed * steps * 1e-5) /
    max(calcInstantaneousSpeedAt(0, mParameter),
        calcInstantaneousSpeedAt(mNumPoints - 1, mParameter));
    
    if (!(fabs(mParameter) < 1e6)) {
        // Reset to preserve numerical stability if param is large or NaN
        mParameter = 0.0;
    }
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

double ColorChooser::getParameter()
{
    return mParameter;
}

ColorA ColorChooser::sample(int index)
{
    return getSample(getSamplePoint(index));
}

double ColorChooser::calcInstantaneousSpeedAt(int index, double param)
{
    double h = 1e-4;
    Vec2f v = getSamplePointAt(index, param + h) - getSamplePointAt(index, param - h);
    return v.length() / (2.0 * h);
}

Vec2f ColorChooser::getSamplePoint(int index)
{
    return getSamplePointAt(index, mParameter);
}

Vec2f ColorChooser::getSamplePointAt(int index, double param)
{
    // Lots of magic numbers... goal here is to use perlin noise to create
    // a random walk for a line that tilts and moves around the sampling space.
    // Speed isn't a concern here, since we use a numeric derivative to keep
    // the speed constant by adjusting the parameter change rate.
    
    Vec2f center = Vec2f(mPerlin->fBm(param, 1.5f) * 0.8f, mPerlin->fBm(param, 3.5f));
    center += Vec2f(0.5f, 0.5f);
    
    float g = mPerlin->fBm(param, 5.5f) * 1.5f;
    float s = 0.15f + mPerlin->fBm(param, 7.5f) * 0.2f;
    Vec2f harmonic(cosf(g) * s, sinf(g) * s);
    
    float t = index / float(mNumPoints - 1);
    return center + harmonic * (t * 2.0f - 1.0f);
}

ColorA ColorChooser::getSample(ci::Vec2f vec)
{
    return mPaletteImage.getPixel(Vec2i(vec.x * mPaletteImage.getWidth(), vec.y * mPaletteImage.getHeight()));
}
