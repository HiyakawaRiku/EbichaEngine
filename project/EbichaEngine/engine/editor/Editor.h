#pragma once
#include <string>
#include <vector>
#include "IEditorInspectable.h"

// 開発用のエディタ（ImGuiで表示する）
// USE_IMGUI が定義されていない構成（Release）では、画面には何も表示しない
class Editor
{
public:
	static Editor* GetInstance();

	void Initialize();
	void Update();     // ImGui::NewFrame と ImGui::Render の間で呼ぶ
	void Finalize();

	// インスペクターに表示するオブジェクトを登録する（シーンの Initialize で呼ぶ）
	void Register(const std::string& name, IEditorInspectable* object);

	// 登録を1つ外す（シーンの途中でオブジェクトを消すときに、消す前に呼ぶ）
	void Unregister(IEditorInspectable* object);

	// 登録をすべて外す（シーンが切り替わるときに SceneManager が呼ぶ）
	void Clear();

private:
	Editor() = default;
	~Editor() = default;
	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	// Hierarchy と Inspector のウィンドウを表示する
	void DrawHierarchy();
	void DrawInspector();

private:
	bool showDemoWindow_ = true;

	// 登録された1件分の情報
	// object の持ち主はシーン。エディタは借りているだけ
	struct Entry {
		std::string name;
		IEditorInspectable* object = nullptr;
	};
	std::vector<Entry> entries_;

	// 今選ばれているオブジェクト（何も選んでいなければ nullptr）
	IEditorInspectable* selected_ = nullptr;
};
