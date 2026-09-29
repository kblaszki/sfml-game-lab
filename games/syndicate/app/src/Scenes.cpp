#include <memory>
#include <syndicate/app/GameOverScene.hpp>
#include <syndicate/app/MainMenuScene.hpp>
#include <syndicate/app/PauseMenuScene.hpp>
#include <syndicate/app/PlayScene.hpp>
#include <syndicate/app/Scenes.hpp>
#include <syndicate/app/SetupScene.hpp>
#include <utility>

namespace sgl::syndicate
{

sgl::SceneFactory mainMenu(const SyndicateServices& services)
{
    return [services]() { return std::make_unique<MainMenuScene>(services); };
}

sgl::SceneFactory setup(const SyndicateServices& services)
{
    return [services]() { return std::make_unique<SetupScene>(services); };
}

sgl::SceneFactory play(const SyndicateServices& services, MatchConfig config, std::vector<Seat> seats)
{
    return [services, config, seats = std::move(seats)]() mutable {
        return std::make_unique<PlayScene>(services, config, std::move(seats));
    };
}

sgl::SceneFactory pause(const SyndicateServices& services)
{
    return [services]() { return std::make_unique<PauseMenuScene>(services); };
}

sgl::SceneFactory makeGameOver(const SyndicateServices& services, GameOver over)
{
    return [services, over = std::move(over)]() mutable {
        return std::make_unique<GameOverScene>(services, std::move(over));
    };
}

} // namespace sgl::syndicate
