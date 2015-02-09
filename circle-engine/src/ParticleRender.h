// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/app/App.h"
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "Box2D/Box2D.h"


class ParticleRender
{
public:
    void setup(ci::app::App &app, unsigned width, unsigned height, float scale);
    void render(const b2ParticleSystem &system, const ci::Rectf &feedback);
    ci::gl::Texture &getTexture();
    
    float           mBrightness;
    float           mFeedbackGain;

private:
    ci::gl::Fbo     mFirstPassFbo;
    ci::gl::Fbo     mSecondPassFbo[2];
    float           mScale;
    bool            mFrame;

    ci::gl::GlslProg mFirstPassProg;
    ci::gl::GlslProg mSecondPassProg;
    ci::gl::Texture  mPointTexture;

    void firstPass(const b2ParticleSystem &system);
    void secondPass(const ci::Rectf &feedback);
};