#include "Editor.h"
#include "SceneManager.h"
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

    ImGui::End();

    // デモウィンドウ（チェックが入っているときだけ）
    if (showDemoWindow_) {
        ImGui::ShowDemoWindow(&showDemoWindow_);
    }

    DrawHierarchy();
    DrawInspector();
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