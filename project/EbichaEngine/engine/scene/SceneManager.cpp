#include "SceneManager.h"

SceneManager* SceneManager::GetInstance() {
    static SceneManager instance;
    return &instance;
}

void SceneManager::Initialize() {
    currentScene_ = nullptr;
    nextScene_ = nullptr;
}

void SceneManager::Update(Input* input) {
    // シーン切り替え予約がある場合、更新処理の前に切り替える
    if (nextScene_) {
        if (currentScene_) {
            currentScene_->Finalize();
        }
        currentScene_ = std::move(nextScene_);
        nextScene_ = nullptr;

        currentScene_->Initialize();
    }

    // 現在のシーンの更新
    if (currentScene_) {
        currentScene_->Update(input);
    }
}

void SceneManager::Draw() {
    if (currentScene_) {
        currentScene_->Draw();
    }
}

void SceneManager::Finalize() {
    if (currentScene_) {
        currentScene_->Finalize();
        currentScene_.reset();
    }
    nextScene_.reset();
}

void SceneManager::ChangeScene(std::unique_ptr<BaseScene> nextScene) {
    nextScene_ = std::move(nextScene);
}