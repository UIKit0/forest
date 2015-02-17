// Fadecandy + OpenGL + Cinder adaptor (c) 2015 Micah Elizabeth Scott
// MIT license

#include "FadecandyGL.h"

using namespace ci;
using namespace std;


void FadecandyGL::setup(ci::app::App &app)
{
    samplingRadius = 8.0;
    mProg = gl::GlslProg( app.loadResource("fadecandy.glslv"), app.loadResource("fadecandy.glslf") );
}

void FadecandyGL::setModel(const vector<Vec2f>& points)
{
    mNumPoints = points.size();

    unsigned height = std::max<unsigned>(kMinHeight, (mNumPoints + kWidth - 1) / kWidth);
    mFramebuffer = gl::Fbo(kWidth, height);
    
    // This float texture is the same size as mFramebuffer, and contains sampling locations
    Surface32f modelPoints(kWidth, height, false);
    
    // Red = X, Green = Y, Blue = Mask (1.0)
    for (unsigned i = 0; i < mNumPoints; i++) {
        modelPoints.setPixel(Vec2i(i % kWidth, i / kWidth), ColorAf(points[i].x, points[i].y, 1.0, 1.0));
    }
            
    mModelTexture = gl::Texture(modelPoints);
    mModelTexture.setMinFilter(GL_NEAREST);
    mModelTexture.setMagFilter(GL_NEAREST);

    mModelVbo = gl::Vbo(GL_ARRAY_BUFFER);
    mModelVbo.bufferData(mNumPoints * 2 * sizeof(GLfloat), &points[0].x, GL_STATIC_DRAW);
}

void FadecandyGL::update(const ci::gl::Texture& sourceTexture, const ci::Matrix33f& sourceTransform)
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
    
    mProg.uniform("modelTexture", 1);
    mModelTexture.bind(1);
    
    mProg.uniform("sourceTransform", sourceTransform);
    mProg.uniform("sampleRadius", (sourceTransform * Vec3f(samplingRadius, 1.0f, 1.0f)).xy().length());
    
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

void FadecandyGL::drawModel(const ci::Matrix33f& windowTransform)
{
    gl::enableAlphaBlending();
    gl::color(1.0f, 1.0f, 1.0f, 0.25f);
    glPointSize((windowTransform * Vec3f(samplingRadius, 1.0, 1.0)).xy().length());

    mModelVbo.bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glDrawArrays(GL_POINTS, 0, mNumPoints);
    glDisableClientState(GL_VERTEX_ARRAY);
    mModelVbo.unbind();
}


