#include <sgl/ui/MenuModel.hpp>

namespace sgl
{

MenuModel::MenuModel(std::vector<Button> buttons)
    : buttons_{std::move(buttons)}
{
}

std::optional<std::size_t> MenuModel::hitTest(Vec2f point) const noexcept
{
    for(std::size_t i = 0; i < buttons_.size(); ++i)
    {
        if(buttons_[i].rect.contains(point))
        {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<std::size_t> MenuModel::update(const InputState& input, const MenuActions& actions)
{
    if(input.action(actions.cancel).pressed)
    {
        cancelled_ = true;
        pressIndex_.reset();
        return std::nullopt;
    }

    if(!buttons_.empty())
    {
        if(input.action(actions.up).pressed)
        {
            focus_ = (focus_ + buttons_.size() - 1) % buttons_.size();
        }
        if(input.action(actions.down).pressed)
        {
            focus_ = (focus_ + 1) % buttons_.size();
        }
    }

    if(const auto pointer = input.pointer())
    {
        if(const auto hit = hitTest(*pointer))
        {
            focus_ = *hit;
        }

        if(input.pointerPressed())
        {
            pressIndex_ = hitTest(*pointer);
        }

        if(input.pointerReleased())
        {
            const auto releaseHit = hitTest(*pointer);
            const auto armed = pressIndex_;
            pressIndex_.reset();
            if(armed && releaseHit && *armed == *releaseHit)
            {
                return *armed;
            }
        }
    }
    else
    {
        pressIndex_.reset();
    }

    if(!buttons_.empty() && input.action(actions.confirm).pressed)
    {
        return focus_;
    }

    return std::nullopt;
}

bool MenuModel::cancelled() const noexcept
{
    return cancelled_;
}

std::size_t MenuModel::focus() const noexcept
{
    return focus_;
}

std::span<const Button> MenuModel::buttons() const noexcept
{
    return buttons_;
}

} // namespace sgl
