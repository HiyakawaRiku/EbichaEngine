#pragma once
#include <memory>
#include <string>
#include "LevelData.h"
#include "IEditorInspectable.h"
#include "Object3d.h"

// レベルに置かれたオブジェクト1つ分
//   MESH  ：file_name のモデルを表示する
//   EMPTY ：見た目を持たない「空のオブジェクト」。エディタで表示しているときだけ axis.obj を目印に描く
// 位置・回転・大きさは、中の Object3d が持っている（エンジンの座標）
class LevelObject : public IEditorInspectable {
public:
	// JSON（Blender の座標）のデータから作る
	void Initialize(const LevelObjectData& data);

	// 今の状態を、JSON（Blender の座標）のデータにする
	LevelObjectData ToData() const;

	void Update();
	void Draw();

	void OnInspector() override;

	const std::string& GetName() const { return name_; }
	const std::string& GetType() const { return type_; }

private:
	std::string type_;
	std::string name_;
	std::string fileName_;

	std::unique_ptr<Object3d> object3d_;
};
