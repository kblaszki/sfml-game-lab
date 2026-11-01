#include <asteroids/app/GameOverScene.hpp>
#include <asteroids/app/MainMenuScene.hpp>
#include <asteroids/app/PauseScene.hpp>
#include <asteroids/app/PlayScene.hpp>
#include <asteroids/app/Scenes.hpp>
#include <memory>

namespace sgl::asteroids
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

} // namespace sgl::asteroids
