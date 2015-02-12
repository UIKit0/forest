// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/app/App.h"
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "Box2D/Box2D.h"


class ParticleRender
{
public:
    void setup(ci::app::App &app, unsigned width, unsigned height, float scale);

    // Vector LUT for perturbing feedback image according to strand vector grid
    void setForceGrid(ci::ImageSourceRef image, ci::Rectf extent);

    // Render particle system and video feedback to texture
    void render(const b2ParticleSystem &system, const ci::gl::Texture& feedbackMask);

    // Read results of last render()
    ci::gl::Texture& getTexture();
    
    // Tweakable things
    float           mBrightness;
    float           mFeedbackGain;
    float           mFeedbackExp;
    float           mFeedbackFlow;

private:
    ci::gl::Fbo     mFirstPassFbo;
    ci::gl::Fbo     mSecondPassFbo[2];
    ci::gl::Texture mForceGridTexture;
    ci::Rectf       mForceGridExtent;
    float           mScale;
    bool            mFrame;

    ci::gl::GlslProg mFirstPassProg;
    ci::gl::GlslProg mSecondPassProg;
    ci::gl::Texture  mPointTexture;

    void firstPass(const b2ParticleSystem &system);
    void secondPass(const ci::gl::Texture& feedbackMask);
};