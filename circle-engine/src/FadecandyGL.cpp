// Fadecandy + OpenGL + Cinder adaptor (c) 2015 Micah Elizabeth Scott
// MIT license

#include "FadecandyGL.h"

using namespace ci;
using namespace std;


void FadecandyGL::setup(ci::app::App &app, std::vector<ci::Vec2f>& points, const ci::MatrixAffine2f& transform)
{
    opc.connectConnectEventHandler(&OPCClient::onConnect, &opc);
    opc.connectErrorEventHandler(&OPCClient::onError, &opc);

    mProg = gl::GlslProg( app.loadAsset("fadecandy.glslv"), app.loadAsset("fadecandy.glslf") );

    unsigned height = std::max<unsigned>(kMinHeight, (points.size() + kWidth - 1) / kWidth);
    mFramebuffer = gl::Fbo(kWidth, height);
    
    // This float texture is the same size as mFramebuffer, and contains sampling locations
    Surface32f modelPoints(kWidth, height, false);
    
    for (unsigned i = 0; i < points.size(); i++) {
        Vec2f vec = transform.transformPoint(points[i]);
        modelPoints.setPixel(Vec2i(i % kWidth, i / kWidth), ColorAf(vec.x, vec.y, 0.0, 1.0));
    }
            
    mModelPoints = gl::Texture(modelPoints);
    mModelPoints.setMinFilter(GL_NEAREST);
    mModelPoints.setMagFilter(GL_NEAREST);

    mNumPoints = points.size();
}

void FadecandyGL::update(const ci::gl::Texture& sourceTexture)
{
    mFramebuffer.bindFramebuffer();
    mProg.bind();
    
    gl::setViewport(Area(Vec2f(0,0), mFramebuffer.getSize()));
    gl::disableAlphaBlending();
    
    static const float positionData[8] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    };
    
    GLint position = mProg.getAttribLocation("position");
    
    mProg.uniform("sourceTexture", 0);
    sourceTexture.bind(0);

    mProg.uniform("modelPoints", 1);
    mModelPoints.bind(1);
    
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, &positionData[0]);
    glEnableVertexAttribArray(position);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(position);
    
    gl::disable(GL_TEXTURE_2D);
    mProg.unbind();

    // Read back the whole framebuffer in RGB format
    mPacketBuffer.reserve(sizeof(OPCClient::Header) + (mFramebuffer.getWidth() * mFramebuffer.getHeight() * 3));
    glReadPixels(0, 0, mFramebuffer.getWidth(), mFramebuffer.getHeight(),
                 GL_RGB, GL_UNSIGNED_BYTE,
                 OPCClient::Header::view(mPacketBuffer).data());

    mPacketBuffer.resize(sizeof(OPCClient::Header) + mNumPoints * 3);
    OPCClient::Header::view(mPacketBuffer).init(0, opc.SET_PIXEL_COLORS, mNumPoints * 3);
    opc.write(mPacketBuffer);
    opc.update();

    mFramebuffer.unbindFramebuffer();
}
