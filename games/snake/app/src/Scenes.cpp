#include <memory>
#include <snake/app/GameOverScene.hpp>
#include <snake/app/MainMenuScene.hpp>
#include <snake/app/PauseScene.hpp>
#include <snake/app/PlayScene.hpp>
#include <snake/app/Scenes.hpp>

namespace sgl::snake
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

} // namespace sgl::snake
