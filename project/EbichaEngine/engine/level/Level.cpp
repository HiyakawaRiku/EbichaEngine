#include "Level.h"
#include "LevelIO.h"
#include "Editor.h"
#include "Logger.h"

Level::~Level()
{
	Clear();
}

bool Level::Load(const std::string& filePath)
{
	// ① JSON を読み込む（失敗したら、今のオブジェクトはそのまま）
	LevelData data;
	if (!LevelIO::Load(filePath, data)) {
		return false;
	}

	// ② 今のオブジェクトを片付けてから、作り直す
	Clear();
	filePath_ = filePath;
	name_ = data.name;

	Editor* editor = Editor::GetInstance();
	for (const LevelObjectData& objectData : data.objects) {
		auto object = std::make_unique<LevelObject>();
		object->Initialize(objectData);
		editor->Register("[" + objectData.type + "] " + objectData.name, object.get());
		objects_.push_back(std::move(object));
	}

	Logger::Log("Level : loaded " + filePath + " (" + std::to_string(objects_.size()) + " objects)\n");
	return true;
}

bool Level::Save() const
{
	if (filePath_.empty()) {
		return false;
	}

	LevelData data;
	data.name = name_;
	for (const auto& object : objects_) {
		data.objects.push_back(object->ToData());
	}

	bool ok = LevelIO::Save(filePath_, data);
	if (ok) {
		Logger::Log("Level : saved " + filePath_ + "\n");
	}
	return ok;
}

bool Level::Reload()
{
	if (filePath_.empty()) {
		return false;
	}
	// Load の中で filePath_ を書き換えるので、コピーを渡す
	std::string path = filePath_;
	return Load(path);
}

void Level::Update()
{
	for (auto& object : objects_) {
		object->Update();
	}
}

void Level::Draw()
{
	for (auto& object : objects_) {
		object->Draw();
	}
}

void Level::Clear()
{
	// 消す前に、エディタの登録を外す（宙ぶらりんポインタを防ぐ）
	Editor* editor = Editor::GetInstance();
	for (auto& object : objects_) {
		editor->Unregister(object.get());
	}
	objects_.clear();
}
