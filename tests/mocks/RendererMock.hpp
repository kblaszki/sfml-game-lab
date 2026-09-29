#pragma once

#include <gmock/gmock.h>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/render/RendererI.hpp>

namespace sgl
{

class RendererMock : public RendererI
{
public:
    MOCK_METHOD(void, begin, (), (override));
    MOCK_METHOD(void, submit, (RenderQueue&), (override));
    MOCK_METHOD(void, end, (), (override));
};

} // namespace sgl
