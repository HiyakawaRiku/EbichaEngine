#include "SceneManager.h"
#include "Editor.h"
#include <cassert>

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
        // 古いシーンのオブジェクトが消える前に、エディタの登録を外す
        Editor::GetInstance()->Clear();

        if (currentScene_) {
            currentScene_->Finalize();
        }
        currentScene_ = std::move(nextScene_);
		currentSceneName_ = nextSceneName_;
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
    // シーンのオブジェクトが消える前に、エディタの登録を外す
    Editor::GetInstance()->Clear();

    if (currentScene_) {
        currentScene_->Finalize();
        currentScene_.reset();
    }
    nextScene_.reset();
    sceneFactory_.reset();
}

void SceneManager::ChangeScene(const std::string& sceneName) {
    assert(sceneFactory_ && "SetSceneFactory を先に呼んでください");

    // 工場に作ってもらう
    nextScene_ = sceneFactory_->CreateScene(sceneName);

    // 名前が間違っていたら nullptr が返ってくるので、ここで気づけるようにする
    assert(nextScene_ && "存在しないシーン名です");

	nextSceneName_ = sceneName;
}

void SceneManager::SetSceneFactory(std::unique_ptr<AbstractSceneFactory> factory) {
    sceneFactory_ = std::move(factory);
}

std::vector<std::string> SceneManager::GetSceneNames() const
{
	if (!sceneFactory_) {
        return {};
	}
	return sceneFactory_->GetSceneNames();
}
