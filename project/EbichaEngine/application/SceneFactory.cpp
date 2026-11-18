#include "SceneFactory.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName)
{
	if (sceneName == "TITLE") {
		return std::make_unique<TitleScene>();
	}
	if (sceneName == "GAMEPLAY") {
		return std::make_unique<GameScene>();
	}
	if (sceneName == "CLEAR") {
		return std::make_unique<ClearScene>();
	}
	if (sceneName == "GAMEOVER") {
		return std::make_unique<GameOverScene>();
	}
	// 知らない名前が来たら nullptr を返す
	return nullptr;
}