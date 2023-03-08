// Copyright (c) 2023. Johan Lind

#include "jleRendering.h"

#include "jleIncludeGL.h"

#include "jleFramebufferMultisample.h"
#include "jleProfiler.h"
#include "jleQuadRendering.h"
#include "jleTextRendering.h"
#include "plog/Log.h"

void
jleRendering::viewportDimensions(int x, int y, unsigned int width, unsigned int height)
{
    glViewport(x, y, static_cast<int>(width), static_cast<int>(height));
}

void
jleRendering::render(jleFramebufferInterface &framebufferOut, const jleCamera &camera)
{
    JLE_SCOPE_PROFILE_CPU(jleRendering_render)

    // TODO: Remove this static buffer here and don't resize it all the time...
    static jleFramebufferMultisample msaa{framebufferOut.width(), framebufferOut.height(), 4};

    {
        // TODO : Fix this, it takes ~5 MS to resize the MSAA framebuffer!!!
        JLE_SCOPE_PROFILE_GPU(jleRendering_RenderMSAAResize)
        msaa.resize(framebufferOut.width(), framebufferOut.height());
    }

    msaa.bind();

    // framebufferOut.bind();

    const auto backgroundColor = camera.getBackgroundColor();
    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    _3dRenderer->queuerender(msaa, camera);
    //_quads->queuerender(framebufferOut, camera);
    //_texts->render(framebufferOut, camera);

    // framebufferOut.bindDefault();
    jleFramebufferInterface::bindDefault();

    msaa.blitToOther(framebufferOut);
}

void
jleRendering::setup()
{
    LOG_VERBOSE << "Creating text rendering";
    this->_texts = std::make_unique<jleTextRendering>();
    LOG_VERBOSE << "Creating quad rendering";
    this->_quads = std::make_unique<jleQuadRendering>();
    LOG_VERBOSE << "Creating 3D rendering";
    this->_3dRenderer = std::make_unique<jle3DRenderer>();
}

void
jleRendering::clearBuffersForNextFrame()
{
    _quads->clearBuffersForNextFrame();
    _texts->clearBuffersForNextFrame();
    _3dRenderer->clearBuffersForNextFrame();
}

jleQuadRendering &
jleRendering::quads()
{
    return *_quads;
}

jleTextRendering &
jleRendering::texts()
{
    return *_texts;
}
jle3DRenderer &
jleRendering::rendering3d()
{
    return *_3dRenderer;
}
