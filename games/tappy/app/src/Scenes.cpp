#include <memory>
#include <tappy/app/GameOverScene.hpp>
#include <tappy/app/MainMenuScene.hpp>
#include <tappy/app/PauseScene.hpp>
#include <tappy/app/PlayScene.hpp>
#include <tappy/app/Scenes.hpp>

namespace sgl::tappy
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

} // namespace sgl::tappy
