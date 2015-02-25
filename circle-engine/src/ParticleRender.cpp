// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "ParticleRender.h"

using namespace ci;
using namespace std;


void ParticleRender::setup(ci::app::App &app, unsigned width, unsigned height, float scale)
{
    mScale = scale;
    mBrightness = 2.0f;
    mFeedbackGain = 0.352f;
    mFeedbackFlow = 6.0f;
    
    gl::Fbo::Format floatFormat;
    floatFormat.setColorInternalFormat(GL_RGBA32F_ARB);

    mFirstPassFbo = gl::Fbo(width, height, floatFormat);
    mSecondPassFbo[0] = gl::Fbo(width, height);
    mSecondPassFbo[1] = gl::Fbo(width, height);

    mFirstPassProg = gl::GlslProg( app.loadResource("particle.pass1.glslv"),
                                   app.loadResource("particle.pass1.glslf") );

    mSecondPassProg = gl::GlslProg( app.loadResource("particle.pass2.glslv"),
                                    app.loadResource("particle.pass2.glslf") );

    mPointTexture = loadImage(app.loadResource("particle.png"));
}

gl::Texture& ParticleRender::getTexture()
{
    return mSecondPassFbo[mFrame].getTexture(0);
}

void ParticleRender::setForceGrid(ImageSourceRef image, Rectf extent)
{
    mForceGridTexture = image;
    mForceGridExtent = extent;
}

void ParticleRender::render(const b2ParticleSystem &system, const ci::gl::Texture& feedbackMask)
{
    mFrame = !mFrame;
    firstPass(system);
    secondPass(feedbackMask);
}

void ParticleRender::firstPass(const b2ParticleSystem &system)
{
    mFirstPassFbo.bindFramebuffer();
    mFirstPassProg.bind();

    gl::setViewport(Area(Vec2f(0,0), mFirstPassFbo.getSize()));
    gl::setMatricesWindow(mFirstPassFbo.getSize());
    gl::clear();
    gl::enableAdditiveBlending();
    gl::enable(GL_POINT_SPRITE);

    int particleCount = system.GetParticleCount();
    float radius = system.GetRadius() * 8.0f * mScale;
    const b2Vec2* positionBuffer = system.GetPositionBuffer();
    const b2ParticleColor* colorBuffer = system.GetColorBuffer();
    const int32* expirationBuffer = system.GetExpirationTimeBuffer();

    GLint position = mFirstPassProg.getAttribLocation("position");
    GLint color = mFirstPassProg.getAttribLocation("color");
    GLint expiration = mFirstPassProg.getAttribLocation("expiration");

    mFirstPassProg.uniform("currentTime", system.GetQuantizedTimeElapsed());
    mFirstPassProg.uniform("fadeDuration", 30);

    mFirstPassProg.uniform("texture", 0);
    mPointTexture.bind();
    
    glPointSize(radius);
    glPushMatrix();
    glScalef(mScale, mScale, mScale);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, &positionBuffer[0].x);
    glVertexAttribPointer(color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuffer[0].r);
    glVertexAttribPointer(expiration, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, &expirationBuffer[0]);
    glEnableVertexAttribArray(position);
    glEnableVertexAttribArray(color);
    glEnableVertexAttribArray(expiration);

    glDrawArrays(GL_POINTS, 0, particleCount);

    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(color);
    glDisableVertexAttribArray(expiration);
    glPopMatrix();
    
    mFirstPassProg.unbind();
    mFirstPassFbo.unbindFramebuffer();
    gl::disable(GL_POINT_SPRITE);
}

void ParticleRender::secondPass(const ci::gl::Texture& feedbackMask)
{
    mSecondPassFbo[mFrame].bindFramebuffer();
    mSecondPassProg.bind();

    Vec2f size = mSecondPassFbo[mFrame].getSize();
    gl::setViewport(Area(Vec2f(0,0), size));
    gl::disableAlphaBlending();
    gl::enable(GL_TEXTURE_2D);
    
    static const float positionData[8] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    };

    GLint position = mSecondPassProg.getAttribLocation("position");

    mSecondPassProg.uniform("brightness", mBrightness);
    mSecondPassProg.uniform("feedbackGain", mFeedbackGain);
    mSecondPassProg.uniform("feedbackFlow", mFeedbackFlow);
    
    mSecondPassProg.uniform("firstPass", 0);
    mFirstPassFbo.getTexture().bind(0);

    mSecondPassProg.uniform("feedback", 1);
    mSecondPassFbo[!mFrame].getTexture().bind(1);
    
    mSecondPassProg.uniform("forceGrid", 2);
    mForceGridTexture.bind(2);

    mSecondPassProg.uniform("feedbackMask", 3);
    feedbackMask.bind(3);

    mSecondPassProg.uniform("texelSize", Vec2f(1.0f / mFirstPassFbo.getWidth(), 1.0f / mFirstPassFbo.getHeight()));

    mSecondPassProg.uniform("forceGridUpperLeft", mForceGridExtent.getUpperLeft());
    mSecondPassProg.uniform("forceGridLowerRight", mForceGridExtent.getLowerRight());
    
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, &positionData[0]);
    glEnableVertexAttribArray(position);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(position);

    gl::disable(GL_TEXTURE_2D);
    mSecondPassProg.unbind();
    mSecondPassFbo[mFrame].unbindFramebuffer();
}
