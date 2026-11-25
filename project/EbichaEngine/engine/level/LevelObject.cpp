#include "LevelObject.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Logger.h"
#include <filesystem>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace {

	// EMPTY の目印に使うモデル
	const char* kEmptyMarkerModel = "axis.obj";

	// ===== 座標の変換 =====
	// Blender：Z が上・右手系 ／ エンジン（DirectX）：Y が上・左手系
	// Y と Z を入れ替え、回転は向きを反転する。
	// ※ 読み込み（Blender → エンジン）と保存（エンジン → Blender）は、必ずこの2つをペアで使う

	Transform BlenderToEngine(const LevelObjectData& d) {
		Transform t;
		t.translate = { d.translation.x, d.translation.z, d.translation.y };
		t.rotate    = { -d.rotation.x, -d.rotation.z, -d.rotation.y };
		t.scale     = { d.scaling.x, d.scaling.z, d.scaling.y };
		return t;
	}

	void EngineToBlender(const Vector3& translate, const Vector3& rotate, const Vector3& scale, LevelObjectData& d) {
		d.translation = { translate.x, translate.z, translate.y };
		d.rotation    = { -rotate.x, -rotate.z, -rotate.y };
		d.scaling     = { scale.x, scale.z, scale.y };
	}

	// resources フォルダにモデルのファイルがあるか（ModelManager は "resources" から読み込む）
	bool ModelFileExists(const std::string& fileName) {
		return !fileName.empty() && std::filesystem::exists(std::filesystem::path("resources") / fileName);
	}

}

void LevelObject::Initialize(const LevelObjectData& data)
{
	type_ = data.type;
	name_ = data.name;
	fileName_ = data.fileName;

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());

	// 表示するモデルを決める（MESH でファイルがなければ、EMPTY と同じ目印を出す）
	std::string modelFile = kEmptyMarkerModel;
	if (type_ == "MESH") {
		if (ModelFileExists(fileName_)) {
			modelFile = fileName_;
		} else {
			Logger::Log("LevelObject : model not found : resources/" + fileName_ + " (" + name_ + ")\n");
		}
	}
	ModelManager::GetInstance()->LoadModel(modelFile);
	object3d_->SetModel(modelFile);

	// Blender の座標 → エンジンの座標
	Transform t = BlenderToEngine(data);
	object3d_->SetTranslate(t.translate);
	object3d_->SetRotate(t.rotate);
	object3d_->SetScale(t.scale);
}

LevelObjectData LevelObject::ToData() const
{
	LevelObjectData data;
	data.type = type_;
	data.name = name_;
	data.fileName = fileName_;

	// エンジンの座標 → Blender の座標
	EngineToBlender(object3d_->GetTranslate(), object3d_->GetRotate(), object3d_->GetScale(), data);
	return data;
}

void LevelObject::Update()
{
	object3d_->Update();
}

void LevelObject::Draw()
{
	if (type_ == "EMPTY") {
		// 空のオブジェクトは、エディタで表示しているときだけ目印を描く
#ifdef USE_IMGUI
		object3d_->Draw();
#endif
		return;
	}
	object3d_->Draw();
}

void LevelObject::OnInspector()
{
#ifdef USE_IMGUI
	ImGui::Text("Type : %s", type_.c_str());
	if (!fileName_.empty()) {
		ImGui::Text("File : %s", fileName_.c_str());
	}
	ImGui::Separator();
	// 位置・回転・大きさ・ブレンドは、中の Object3d のインスペクターで編集する
	object3d_->OnInspector();
#endif
}
