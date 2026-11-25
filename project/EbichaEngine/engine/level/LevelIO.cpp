#include "LevelIO.h"
#include "Logger.h"
#include "nlohmann/json.hpp"
#include <fstream>

// ordered_json：書き出すときに、項目を「追加した順」で並べる（普通の json はアルファベット順に並び替える）
using json = nlohmann::ordered_json;

namespace {

	// JSON の [x, y, z] を Vector3 にする。項目がない・形が違うときは defaultValue を返す
	Vector3 ReadVector3(const json& parent, const char* key, const Vector3& defaultValue) {
		if (!parent.contains(key)) {
			return defaultValue;
		}
		const json& a = parent[key];
		if (!a.is_array() || a.size() != 3) {
			return defaultValue;
		}
		return { a[0].get<float>(), a[1].get<float>(), a[2].get<float>() };
	}

	// Vector3 を JSON の [x, y, z] にする
	json WriteVector3(const Vector3& v) {
		return json::array({ v.x, v.y, v.z });
	}

}

namespace LevelIO {

	bool Load(const std::string& filePath, LevelData& outData) {
		// ① ファイルを開く
		std::ifstream file(filePath);
		if (!file.is_open()) {
			Logger::Log("LevelIO::Load : cannot open file : " + filePath + "\n");
			return false;
		}

		try {
			// ② JSON として読み込む
			json root = json::parse(file);

			// ③ 中身を LevelData に入れていく
			LevelData data;
			data.name = root.value("name", "");

			if (root.contains("objects") && root["objects"].is_array()) {
				for (const json& o : root["objects"]) {
					LevelObjectData obj;
					obj.type = o.value("type", "");
					obj.name = o.value("name", "");
					obj.fileName = o.value("file_name", "");

					if (o.contains("transform")) {
						const json& t = o["transform"];
						obj.translation = ReadVector3(t, "translation", { 0.0f, 0.0f, 0.0f });
						obj.rotation    = ReadVector3(t, "rotation",    { 0.0f, 0.0f, 0.0f });
						obj.scaling     = ReadVector3(t, "scaling",     { 1.0f, 1.0f, 1.0f });
					}

					data.objects.push_back(obj);
				}
			}

			// 全部読めたときだけ、呼んだ側のデータを書き換える
			outData = std::move(data);
			return true;
		}
		catch (const json::exception& e) {
			// JSON の書き方が間違っている、型が違う、などのとき
			Logger::Log(std::string("LevelIO::Load : JSON error : ") + e.what() + "\n");
			return false;
		}
	}

	bool Save(const std::string& filePath, const LevelData& data) {
		// ① LevelData を JSON にする
		json root;
		root["name"] = data.name;
		root["objects"] = json::array();

		for (const LevelObjectData& obj : data.objects) {
			json o;
			o["type"] = obj.type;
			o["name"] = obj.name;
			if (!obj.fileName.empty()) {
				o["file_name"] = obj.fileName;   // MESH のときだけ書く
			}
			o["transform"]["translation"] = WriteVector3(obj.translation);
			o["transform"]["rotation"]    = WriteVector3(obj.rotation);
			o["transform"]["scaling"]     = WriteVector3(obj.scaling);

			root["objects"].push_back(o);
		}

		// ② ファイルに書き出す
		std::ofstream file(filePath);
		if (!file.is_open()) {
			Logger::Log("LevelIO::Save : cannot open file : " + filePath + "\n");
			return false;
		}
		file << root.dump(2);   // 2 は字下げの幅（人が読みやすいように整形する）
		return true;
	}

}
