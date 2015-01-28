// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "ParticleRender.h"

using namespace ci;


void ParticleRender::setup(ci::app::App &app, unsigned width, unsigned height, float scale)
{
    mScale = scale;

    gl::Fbo::Format floatFormat;
    floatFormat.setColorInternalFormat(GL_RGBA32F_ARB);

    mFirstPassFbo = gl::Fbo(width, height, floatFormat);
    mSecondPassFbo = gl::Fbo(width, height);

    mFirstPassProg = gl::GlslProg( app.loadAsset("particle.pass1.glslv"),
                                   app.loadAsset("particle.pass1.glslf") );

    mSecondPassProg = gl::GlslProg( app.loadAsset("particle.pass2.glslv"),
                                    app.loadAsset("particle.pass2.glslf") );
}

gl::Texture& ParticleRender::getTexture()
{
    return mSecondPassFbo.getTexture(0);
}

void ParticleRender::render(const b2ParticleSystem &system)
{
    firstPass(system);
    secondPass();
}

void ParticleRender::firstPass(const b2ParticleSystem &system)
{
    mFirstPassFbo.bindFramebuffer();
    mFirstPassProg.bind();

    gl::setViewport(Area(Vec2f(0,0), mFirstPassFbo.getSize()));
    gl::setMatricesWindow(mFirstPassFbo.getSize());
    gl::clear();
    gl::enableAdditiveBlending();

    int particleCount = system.GetParticleCount();
    float radius = system.GetRadius() * 4.0f * mScale;
    const b2Vec2* positionBuffer = system.GetPositionBuffer();
    const b2ParticleColor* colorBuffer = system.GetColorBuffer();

    GLint position = mFirstPassProg.getAttribLocation("position");
    GLint color = mFirstPassProg.getAttribLocation("color");
    
    glPointSize(radius);
    glPushMatrix();
    glScalef(mScale, mScale, mScale);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, &positionBuffer[0].x);
    glVertexAttribPointer(color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuffer[0].r);
    glEnableVertexAttribArray(position);
    glEnableVertexAttribArray(color);

    glDrawArrays(GL_POINTS, 0, particleCount);

    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(color);
    glPopMatrix();
    
    mFirstPassProg.unbind();
    mFirstPassFbo.unbindFramebuffer();
}

void ParticleRender::secondPass()
{
    mSecondPassFbo.bindFramebuffer();
    mSecondPassProg.bind();

    gl::setViewport(Area(Vec2f(0,0), mSecondPassFbo.getSize()));
    gl::disableAlphaBlending();
    gl::enable(GL_TEXTURE_2D);
    
    static const float positionData[8] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    };

    GLint position = mSecondPassProg.getAttribLocation("position");

    mSecondPassProg.uniform("firstPass", 0);
    mFirstPassFbo.getTexture().bind();

    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, &positionData[0]);
    glEnableVertexAttribArray(position);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(position);

    gl::disable(GL_TEXTURE_2D);
    mSecondPassProg.unbind();
    mSecondPassFbo.unbindFramebuffer();
}
