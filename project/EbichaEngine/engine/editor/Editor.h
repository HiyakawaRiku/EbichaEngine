#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "IEditorInspectable.h"

class Level;

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

	// Save / Reload の対象にするレベルをセットする（シーンの Initialize で呼ぶ）
	// シーンが切り替わるときは、Clear() で自動的に外れる
	void SetLevel(Level* level) { level_ = level; }

	// Game ウィンドウに表示するテクスチャをセットする（起動時に1回呼ぶ）
	// textureId：SRV の GPU ハンドル（.ptr）
	void SetGameTexture(uint64_t textureId, float width, float height);

private:
	Editor() = default;
	~Editor() = default;
	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	// Game・Hierarchy・Inspector のウィンドウを表示する
	void DrawGameView();
	void DrawLevelPanel();   // Editor ウィンドウの中の「Level」の部分
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

	// Save / Reload の対象のレベル（持ち主はシーン。エディタは借りているだけ）
	Level* level_ = nullptr;
	std::string levelMessage_;   // 「保存しました」などの結果の表示

	// Game ウィンドウに表示するテクスチャ（ゲーム画面）
	uint64_t gameTextureId_ = 0;
	float gameWidth_ = 0.0f;
	float gameHeight_ = 0.0f;
};
