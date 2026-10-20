#pragma once

#include <memory>
#include <string>
#include "BaseScene.h"

class Input;

class SceneManager {
public:
    static SceneManager* GetInstance();

    void Initialize();
    void Update(Input* input);
    void Draw();
    void Finalize();

    // 次のシーンへの遷移予約
    void ChangeScene(std::unique_ptr<BaseScene> nextScene);

private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

private:
    std::unique_ptr<BaseScene> currentScene_ = nullptr;
    std::unique_ptr<BaseScene> nextScene_ = nullptr;
};