// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "FadecandyGL.h"

using namespace ci;
using namespace std;


void FadecandyGL::setup()
{
    setupFramebuffer(0);

    opc.connectConnectEventHandler(&OPCClient::onConnect, &opc);
    opc.connectErrorEventHandler(&OPCClient::onError, &opc);
}

void FadecandyGL::update(const ci::gl::Texture& texture, std::vector<ci::Vec2f> points, const ci::Matrix44f& transform)
{
    /*
     * Use GL_POINTS to gather all sampling points from the input texture and
     * pack them into an FBO, which we then read into the network packet buffer.
     * This avoids reading the entire input texture into system memory.
     */

    if (!mFramebuffer || mDrawPoints.size() != points.size()) {
        setupFramebuffer(points.size());
    }

    mFramebuffer.bindFramebuffer();

    gl::setViewport(mFramebuffer.getBounds());
    gl::setMatricesWindow(mFramebuffer.getSize());
    gl::disableAlphaBlending();
    gl::color(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(transform.m);

    texture.bind();
    gl::enable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &mDrawPoints[0].x);
    glTexCoordPointer(2, GL_FLOAT, 0, &points[0].x);
    
    glDrawArrays(GL_POINTS, 0, points.size());

    gl::disable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    // Read back the whole framebuffer in RGB format
    mPacketBuffer.reserve(sizeof(OPCClient::Header) + (mFramebuffer.getWidth() * mFramebuffer.getHeight() * 3));
    glReadPixels(0, 0, mFramebuffer.getWidth(), mFramebuffer.getHeight(),
                 GL_RGB, GL_UNSIGNED_BYTE,
                 OPCClient::Header::view(mPacketBuffer).data());

    unsigned drawBytes = mDrawPoints.size() * 3;
    mPacketBuffer.resize(sizeof(OPCClient::Header) + drawBytes);
    OPCClient::Header::view(mPacketBuffer).init(0, opc.SET_PIXEL_COLORS, drawBytes);
    opc.write(mPacketBuffer);
    opc.update();

    mFramebuffer.unbindFramebuffer();
}

void FadecandyGL::setupFramebuffer(unsigned numPoints)
{
    unsigned width = 64;
    unsigned height = std::max<unsigned>(64, (numPoints + width - 1) / height);

    mFramebuffer = gl::Fbo(width, height);
    
    mDrawPoints.resize(numPoints);
    for (unsigned i = 0; i < numPoints; i++) {
        mDrawPoints[i] = Vec2f(width - 1 - (i % width), height - 1 - (i / width));
    }
}
