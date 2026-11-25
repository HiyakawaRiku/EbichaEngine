#include "Editor.h"
#include "SceneManager.h"
#include "Level.h"
#include <algorithm>
#include <vector>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

Editor* Editor::GetInstance() {
    static Editor instance;
    return &instance;
}

void Editor::Initialize() {
}

void Editor::Update() {
#ifdef USE_IMGUI
    // 画面全体を、ウィンドウをくっつけられる場所（ドックスペース）にする
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Editor");

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS : %.1f (%.2f ms)", fps, 1000.0f / fps);

    ImGui::Text("Scene : %s", SceneManager::GetInstance()->GetCurrentSceneName().c_str());

    ImGui::Separator();   // 区切り線

    ImGui::Checkbox("show ImGui Demo", &showDemoWindow_);

    ImGui::Separator();

    ImGui::Text("Scenes");

	SceneManager* sceneManager = SceneManager::GetInstance();
	const std::string& currentName = sceneManager->GetCurrentSceneName();

	for (const std::string& name : sceneManager->GetSceneNames()) {
		bool isCurrent = (name == currentName);
		if(ImGui::Selectable(name.c_str(), isCurrent)) {
			sceneManager->ChangeScene(name);
		}
	}

    if (ImGui::Button("Reload")) {
		sceneManager->ChangeScene(currentName);
    }

    DrawLevelPanel();

    ImGui::End();

    // デモウィンドウ（チェックが入っているときだけ）
    if (showDemoWindow_) {
        ImGui::ShowDemoWindow(&showDemoWindow_);
    }

    DrawGameView();
    DrawHierarchy();
    DrawInspector();
#endif
}

void Editor::DrawLevelPanel() {
#ifdef USE_IMGUI
    ImGui::Separator();
    ImGui::Text("Level");

    if (!level_) {
        ImGui::TextDisabled("(no level in this scene)");
        return;
    }

    ImGui::Text("File : %s", level_->GetFilePath().c_str());
    ImGui::Text("Objects : %zu", level_->GetObjectCount());

    // 今の配置を JSON に上書き保存する
    if (ImGui::Button("Save Level")) {
        levelMessage_ = level_->Save() ? "Saved." : "Save failed. (see Output window)";
    }
    ImGui::SameLine();
    // JSON を読み直す（Blender で編集したあとに使う）。選択中のオブジェクトは外れる
    if (ImGui::Button("Reload Level")) {
        levelMessage_ = level_->Reload() ? "Reloaded." : "Reload failed. (see Output window)";
    }

    if (!levelMessage_.empty()) {
        ImGui::Text("%s", levelMessage_.c_str());
    }
#endif
}

void Editor::SetGameTexture(uint64_t textureId, float width, float height) {
    gameTextureId_ = textureId;
    gameWidth_ = width;
    gameHeight_ = height;
}

void Editor::DrawGameView() {
#ifdef USE_IMGUI
    // ゲーム画面の周りに余白を作らない
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(640.0f, 360.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game");

    if (gameTextureId_ != 0 && gameWidth_ > 0.0f && gameHeight_ > 0.0f) {
        // ウィンドウの大きさに合わせて、縦横比を保ったまま拡大・縮小する
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float scale = (std::min)(avail.x / gameWidth_, avail.y / gameHeight_);
        if (scale > 0.0f) {
            ImVec2 size(gameWidth_ * scale, gameHeight_ * scale);

            // 真ん中に表示する
            ImVec2 cursor = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursor.x + (avail.x - size.x) * 0.5f,
                                       cursor.y + (avail.y - size.y) * 0.5f));

            ImGui::Image(reinterpret_cast<ImTextureID>(gameTextureId_), size);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
#endif
}

void Editor::Finalize() {
    Clear();
}

void Editor::Register(const std::string& name, IEditorInspectable* object) {
    entries_.push_back({ name, object });
}

void Editor::Unregister(IEditorInspectable* object) {
    // 選択中のものが消されるなら、選択も外す
    if (selected_ == object) {
        selected_ = nullptr;
    }
    // 一覧から、object と同じものを取り除く
    std::erase_if(entries_, [object](const Entry& e) { return e.object == object; });
}

void Editor::Clear() {
    entries_.clear();
    selected_ = nullptr;
    level_ = nullptr;        // レベルもシーンと一緒に消えるので、外しておく
    levelMessage_.clear();
}

void Editor::DrawHierarchy() {
#ifdef USE_IMGUI
    ImGui::SetNextWindowPos(ImVec2(10.0f, 260.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Hierarchy");

    for (size_t i = 0; i < entries_.size(); i++) {
        const Entry& entry = entries_[i];

        // 同じ名前のオブジェクトがあっても区別できるように、番号を ID にする
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Selectable(entry.name.c_str(), selected_ == entry.object)) {
            selected_ = entry.object;
        }
        ImGui::PopID();
    }

    ImGui::End();
#endif
}

void Editor::DrawInspector() {
#ifdef USE_IMGUI
    ImGui::SetNextWindowPos(ImVec2(960.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Inspector");

    if (selected_) {
        // Sprite なら Sprite の、Object3d なら Object3d の OnInspector が呼ばれる
        selected_->OnInspector();
    } else {
        ImGui::Text("Select an object in Hierarchy");
    }

    ImGui::End();
#endif
}