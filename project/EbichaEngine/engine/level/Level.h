#pragma once
#include <memory>
#include <string>
#include <vector>
#include "LevelObject.h"

// JSON ファイル1つ分のオブジェクトをまとめて持つ
// 作ったオブジェクトは、エディタの Hierarchy に自動で登録する
class Level {
public:
	~Level();

	// JSON を読み込んで、オブジェクトを作る
	// 失敗したら false を返す（そのときは、今あるオブジェクトはそのまま残す）
	// ※ Object3d がカメラを使うので、Object3dCommon にカメラをセットしたあとで呼ぶ
	bool Load(const std::string& filePath);

	// 今の配置を、読み込んだ JSON ファイルに上書き保存する
	bool Save() const;

	// 読み込んだ JSON ファイルを、もう一度読み込む（Blender で編集したあとに使う）
	bool Reload();

	void Update();
	void Draw();

	// オブジェクトをすべて消す（エディタの登録も外す）
	void Clear();

	const std::string& GetFilePath() const { return filePath_; }
	size_t GetObjectCount() const { return objects_.size(); }

private:
	std::string filePath_;
	std::string name_;
	std::vector<std::unique_ptr<LevelObject>> objects_;
};
