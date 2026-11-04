#include <memory>
#include <tower/app/GameOverScene.hpp>
#include <tower/app/MainMenuScene.hpp>
#include <tower/app/PauseScene.hpp>
#include <tower/app/PlayScene.hpp>
#include <tower/app/Scenes.hpp>

namespace sgl::tower
{

sgl::SceneFactory mainMenu(const AppServices& services)
{
    return [services]() { return std::make_unique<MainMenuScene>(services); };
}

sgl::SceneFactory play(const AppServices& services)
{
    return [services]() { return std::make_unique<PlayScene>(services); };
}

sgl::SceneFactory pause(const AppServices& services)
{
    return [services]() { return std::make_unique<PauseScene>(services); };
}

sgl::SceneFactory gameOver(const AppServices& services, int score)
{
    return [services, score]() { return std::make_unique<GameOverScene>(services, score); };
}

} // namespace sgl::tower
