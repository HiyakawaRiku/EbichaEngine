#pragma once
#include <memory>
#include <string>
#include "BaseScene.h"

// シーン工場の「約束」だけを決めた抽象クラス
// 具体的に何のシーンを作るかは、派生クラス（application側）が決める
class AbstractSceneFactory {
public:
    virtual ~AbstractSceneFactory() = default;

    // 名前を渡すと、そのシーンを作って返す
    virtual std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) = 0;
};