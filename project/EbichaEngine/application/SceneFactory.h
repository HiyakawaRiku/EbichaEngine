#pragma once
#include "AbstractSceneFactory.h"

// このゲームで使うシーンを作る工場
class SceneFactory : public AbstractSceneFactory {
public:
    std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) override;
};