// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/ImageIo.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Perlin.h"


class ColorChooser
{
public:
    void setup(ci::ImageSourceRef image, ci::Perlin* perlin, int numPoints);

    void update();
    void seek(int steps);
    void draw();
    ci::ColorA sample(int index);

    ci::Vec2f getSamplePoint(int index);
    ci::ColorA getSample(ci::Vec2f vec);

private:
    ci::Surface8u mPaletteImage;
    ci::gl::Texture mPaletteTexture;
    ci::Perlin* mPerlin;
    int mNumPoints;
    double mParameter;
};

