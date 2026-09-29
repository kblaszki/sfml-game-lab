#pragma once

namespace sgl
{

class RenderQueue;

class RendererI
{
public:
    virtual ~RendererI() = default;
    virtual void begin() = 0;
    virtual void submit(RenderQueue&) = 0;
    virtual void end() = 0;
};

} // namespace sgl
