#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <sgl/input/InputState.hpp>
#include <sgl/ui/Button.hpp>
#include <sgl/ui/MenuActions.hpp>
#include <vector>

namespace sgl
{

class MenuModel
{
public:
    explicit MenuModel(std::vector<Button> buttons);

    // returns index of the button activated this frame, or nullopt
    [[nodiscard]] std::optional<std::size_t> update(const InputState& input, const MenuActions& actions);

    [[nodiscard]] bool cancelled() const noexcept;
    [[nodiscard]] std::size_t focus() const noexcept;
    [[nodiscard]] std::span<const Button> buttons() const noexcept;

private:
    [[nodiscard]] std::optional<std::size_t> hitTest(Vec2f point) const noexcept;

    std::vector<Button> buttons_;
    std::size_t focus_{};
    bool cancelled_{};
    std::optional<std::size_t> pressIndex_;
};

} // namespace sgl
