#include <dodger/app/GameOverScene.hpp>
#include <dodger/app/MainMenuScene.hpp>
#include <dodger/app/PauseScene.hpp>
#include <dodger/app/PlayScene.hpp>
#include <dodger/app/Scenes.hpp>
#include <memory>

namespace sgl::dodger
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

} // namespace sgl::dodger
