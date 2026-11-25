#pragma once

#include <memory>
#include <string>
#include <vector>
#include "BaseScene.h"
#include "AbstractSceneFactory.h"

class Input;

class SceneManager {
public:
    static SceneManager* GetInstance();

    void Initialize();
    void Update(Input* input);
    void Draw();
    void Finalize();

    // 次のシーンへの遷移予約
    void ChangeScene(const std::string& sceneName);
    void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> factory);

	const std::string& GetCurrentSceneName() const{
		return currentSceneName_;
	}

    // 切り替えられるシーンの名前の一覧を返す
    std::vector<std::string> GetSceneNames() const;

private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

private:
    std::unique_ptr<BaseScene> currentScene_ = nullptr;
    std::unique_ptr<BaseScene> nextScene_ = nullptr;
    std::string currentSceneName_;
	std::string nextSceneName_;

	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
};