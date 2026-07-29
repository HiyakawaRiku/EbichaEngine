#include "EbichaEngine.h"

enum ObjectType {
	plane,
	sphere,
	teapot,
	bunny,
	multiMesh,
	suzanne
};

enum LightingMode {
	none,
	lambert,
	halfLambert,
};

#include <vector>
#include <memory>
#include <string>

// --- テクスチャ管理用の構造体 ---
struct TextureData {
	int id;
	std::string name;
	std::string filePath;
};

// 動的に生成されたオブジェクト群を管理するコンテナ
std::vector<std::unique_ptr<BaseObject>> g_sceneObjects;
int selectedObjectIndex = -1; // ImGuiのリストで選択中のインデックス

// CreateObject 関数に camera を渡せるように変更
void CreateObject(ObjectType type, Camera* camera) {
	std::unique_ptr<BaseObject> newObj = nullptr;

	switch (type) {
	case ObjectType::plane: {
		auto model = std::make_unique<Model>();
		model->Initialize("resources", "plane.obj");
		newObj = std::move(model);
		break;
	}
	case ObjectType::sphere: {
		auto sphere = std::make_unique<Sphere>();
		sphere->Initialize();
		newObj = std::move(sphere);
		break;
	}
	case ObjectType::teapot: {
		auto model = std::make_unique<Model>();
		model->Initialize("resources", "teapot.obj");
		newObj = std::move(model);
		break;
	}
	case ObjectType::bunny: {
		auto model = std::make_unique<Model>();
		model->Initialize("resources", "bunny.obj");
		newObj = std::move(model);
		break;
	}
	case ObjectType::multiMesh: {
		auto model = std::make_unique<Model>();
		model->Initialize("resources", "multiMesh.obj");
		newObj = std::move(model);
		break;
	}
	case ObjectType::suzanne:
		return;
	}

	if (newObj) {
		// (0, 0, 0) の位置・回転・スケールに初期化
		newObj->transform.translate = { 0.0f, 0.0f, 0.0f };
		newObj->transform.rotate = { 0.0f, 0.0f, 0.0f };
		newObj->transform.scale = { 1.0f, 1.0f, 1.0f };

		// 初期テクスチャIDを 1 に設定
		newObj->textureId = 1;

		// 【重要】描画される前にその場で Update を呼び出し、WVP行列等を正しく計算・転送しておく
		if (camera) {
			newObj->Update(camera);
		}

		g_sceneObjects.push_back(std::move(newObj));
		selectedObjectIndex = static_cast<int>(g_sceneObjects.size()) - 1;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの一括初期化
	EbichaEngine* ebichaEngine = EbichaEngine::GetInstance();
	ebichaEngine->Initialize();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Input* input = Input::GetInstance(); // シングルトンインスタンス

	Sprite* sprite = new Sprite;
	sprite->Initialize();

	Sphere* sphere = new Sphere;
	sphere->Initialize();

	Model* modelPlane = new Model;
	modelPlane->Initialize("resources", "plane.obj");
	Model* modelTeapot = new Model;
	modelTeapot->Initialize("resources", "teapot.obj");
	Model* modelBunny = new Model;
	modelBunny->Initialize("resources", "bunny.obj");
	Model* modelMultiMesh = new Model;
	modelMultiMesh->Initialize("resources", "multiMesh.obj");

	auto normalCamera = std::make_unique<Camera>();
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize();
	Camera* activeCamera = normalCamera.get();
	bool useDebugCamera = false;

	// CPUで動かす用のTransformを作る
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	bool visibleUI = true;

	// ----------------------------------------------------
	// テクスチャの一括登録と管理
	// ----------------------------------------------------
	std::vector<TextureData> textureList = {
		{ 1, "uvChecker", "resources/uvChecker.png" },
		{ 2, "sky_sphere", "resources/sky_sphere.png" },
		{ 3, "monsterBall", "resources/monsterBall.png" },
		{ 4, "ground_leaf", "resources/ground_leaf.png" },
		{ 5, "fence", "resources/fence.png" },
		{ 6, "checkerBoard", "resources/checkerBoard.png" }
	};

	// 実際に DirectXCommon にロード
	for (const auto& tex : textureList) {
		dxCommon->InitializeTexture(tex.filePath, tex.id);
	}

	// ImGui::Combo 用の「\0」区切り文字列を作成
	std::string textureNamesCombo = "";
	for (const auto& tex : textureList) {
		textureNamesCombo += tex.name + '\0';
	}
	textureNamesCombo += '\0'; // 終端のヌル文字

	// 選択中のテクスチャインデックス（配列要素のインデックス 0〜5）
	int selectedSpriteTexIndex = 0; // Sprite用

	ObjectType objectType = ObjectType::plane;
	const char* object_names = "plane\0sphere\0teapot\0bunny\0multiMesh\0suzanne\0\0";

	LightingMode lightingMode = LightingMode::none;
	const char* lightingMode_names = "none\0lambert\0halfLambert\0\0";

	MSG msg{};
	// ウィンドウのxボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			input->Update();

			normalCamera->Update();

			if (activeCamera != normalCamera.get()) {
				activeCamera->Update();
			}
			dxCommon->PreDraw();

			ImGui::Begin("Global Setting");

			// カメラ切り替え用のチェックボックス
			if (ImGui::Checkbox("Use Debug Camera", &useDebugCamera)) {
				if (useDebugCamera) {
					activeCamera = debugCamera.get();
				}
				else {
					activeCamera = normalCamera.get();
				}
			}
			if (ImGui::CollapsingHeader("camera", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::DragFloat3("Rotate", &normalCamera->transform_.rotate.x, 0.01f);
				ImGui::DragFloat3("Translate", &normalCamera->transform_.translate.x, 0.01f);
			}
			ImGui::Separator();

			int currentIndex = static_cast<int>(dxCommon->blendMode_);
			if (ImGui::Combo("Blend Mode", &currentIndex, dxCommon->blendMode_names)) {
				dxCommon->blendMode_ = static_cast<BlendMode>(currentIndex);
				dxCommon->SetBlendMode(dxCommon->blendMode_);
			};

			ImGui::Separator();
			ImGui::Text("Sprite Settings");
			ImGui::Checkbox("Show Sprite", &visibleUI);
			if (ImGui::CollapsingHeader("sprite", ImGuiTreeNodeFlags_DefaultOpen)) {

				// スプライト用テクスチャ切り替えコンボボックス
				ImGui::Combo("Sprite Texture", &selectedSpriteTexIndex, textureNamesCombo.c_str());

				// colorSprite (Sprite用マテリアルカラー)
				float colorSprite4[4] = { sprite->color.x, sprite->color.y, sprite->color.z, sprite->color.w };
				ImGui::ColorEdit4("colorSprite", colorSprite4);
				sprite->color = { colorSprite4[0], colorSprite4[1], colorSprite4[2], colorSprite4[3] };

				// translateSprite
				ImGui::DragFloat3("translateSprite", &sprite->transform.translate.x, 0.5f);

				ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
				ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
				ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

			}
			ImGui::Separator();

			int lightingIndex = static_cast<int>(lightingMode);
			if (ImGui::Combo("Lighting Mode", &lightingIndex, lightingMode_names)) {
				lightingMode = static_cast<LightingMode>(lightingIndex);
			}
			switch (lightingMode) {
			case LightingMode::none:
				for (auto& obj : g_sceneObjects) {
					obj->lightingType = LightType_None;
				}
				break;
			case LightingMode::lambert:
				for (auto& obj : g_sceneObjects) {
					obj->lightingType = LightType_Lambert;
				}
				break;
			case LightingMode::halfLambert:
				for (auto& obj : g_sceneObjects) {
					obj->lightingType = LightType_HalfLambert;
				}
				break;
			}

			// LightColor
			float lightColor4[4] = { sphere->directionalLightData->color.x, sphere->directionalLightData->color.y, sphere->directionalLightData->color.z, sphere->directionalLightData->color.w };
			if (ImGui::ColorEdit4("LightColor", lightColor4)) {
				Vector4 newColor = { lightColor4[0], lightColor4[1], lightColor4[2], lightColor4[3] };
				sphere->directionalLightData->color = newColor;
				for (auto& obj : g_sceneObjects) {
					if (obj->directionalLightData) obj->directionalLightData->color = newColor;
				}
			}

			// LightDirection
			if (ImGui::SliderFloat3("LightDirection", &sphere->directionalLightData->direction.x, -1.0f, 10.0f, "%.2f")) {
				Vector3 newDir = sphere->directionalLightData->direction;
				for (auto& obj : g_sceneObjects) {
					if (obj->directionalLightData) obj->directionalLightData->direction = newDir;
				}
			}

			// Intensity
			if (ImGui::DragFloat("Intensity", &sphere->directionalLightData->intensity, 0.01f)) {
				float newIntensity = sphere->directionalLightData->intensity;
				for (auto& obj : g_sceneObjects) {
					if (obj->directionalLightData) obj->directionalLightData->intensity = newIntensity;
				}
			}

			// ライト位置の可視化
			Vector3 lightDir = sphere->directionalLightData->direction;
			float len = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
			if (len > 0.001f) {
				lightDir.x /= len;
				lightDir.y /= len;
				lightDir.z /= len;
			}

			Vector3 lightPos = {
				-lightDir.x * 5.0f,
				-lightDir.y * 5.0f,
				-lightDir.z * 5.0f
			};

			DebugRenderer::AddWireSphere(lightPos, 0.5f, 12, { 1.0f, 1.0f, 0.0f, 1.0f });

			ImGui::End();

			// オブジェクトの更新
			sprite->Update(activeCamera);
			sphere->Update(activeCamera);
			modelPlane->Update(activeCamera);
			modelTeapot->Update(activeCamera);
			modelBunny->Update(activeCamera);
			modelMultiMesh->Update(activeCamera);

			for (auto& obj : g_sceneObjects) {
				obj->Update(activeCamera);
			}

			// 床のグリッド
			DebugRenderer::AddGrid(20.0f, 20, { 0.5f, 0.5f, 0.5f, 1.0f });

			if (input->PushKey(DIK_P)) {
				activeCamera->transform_.rotate.x -= 0.03f;
			}

#ifdef _DEBUG

			ImGui::Begin("Settings");

			// --- 1. 生成するモデルタイプの選択 ---
			int objectIndex = static_cast<int>(objectType);
			if (ImGui::Combo("Select Object", &objectIndex, object_names)) {
				objectType = static_cast<ObjectType>(objectIndex);
			}

			// --- 2. Create ボタン ---
			if (ImGui::Button("Create")) {
				CreateObject(objectType, activeCamera);
			}

			ImGui::Separator();
			ImGui::Text("Created Objects (%d)", static_cast<int>(g_sceneObjects.size()));

			// --- 3. 生成されたオブジェクトの一覧・編集・削除 ---
			if (!g_sceneObjects.empty()) {
				if (ImGui::BeginListBox("Created List")) {
					for (int i = 0; i < g_sceneObjects.size(); i++) {
						std::string label = "Object " + std::to_string(i + 1);
						const bool isSelected = (selectedObjectIndex == i);
						if (ImGui::Selectable(label.c_str(), isSelected)) {
							selectedObjectIndex = i;
						}
						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndListBox();
				}

				if (selectedObjectIndex >= 0 && selectedObjectIndex < g_sceneObjects.size()) {
					if (ImGui::Button("Delete Selected")) {
						g_sceneObjects.erase(g_sceneObjects.begin() + selectedObjectIndex);

						if (selectedObjectIndex >= g_sceneObjects.size()) {
							selectedObjectIndex = static_cast<int>(g_sceneObjects.size()) - 1;
						}
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Delete All")) {
					g_sceneObjects.clear();
					selectedObjectIndex = -1;
				}

				// --- 選択中のオブジェクトの個別編集領域 ---
				if (selectedObjectIndex >= 0 && selectedObjectIndex < g_sceneObjects.size()) {
					auto& target = g_sceneObjects[selectedObjectIndex];
					ImGui::Separator();
					ImGui::Text("Edit Target: Object %d", selectedObjectIndex + 1);

					// 【追加】選択中のオブジェクト専用のテクスチャ切り替えコンボボックス
					int currentTexIndex = 0;
					for (int i = 0; i < textureList.size(); i++) {
						if (textureList[i].id == target->textureId) {
							currentTexIndex = i;
							break;
						}
					}

					if (ImGui::Combo("Texture", &currentTexIndex, textureNamesCombo.c_str())) {
						// 選択したテクスチャIDをターゲットオブジェクトに保存
						target->textureId = textureList[currentTexIndex].id;
					}

					float colorBuf[4] = { target->color.x, target->color.y, target->color.z, target->color.w };
					if (ImGui::ColorEdit4("Color", colorBuf)) {
						target->color = { colorBuf[0], colorBuf[1], colorBuf[2], colorBuf[3] };
					}

					ImGui::DragFloat3("Scale", &target->transform.scale.x, 0.1f);
					ImGui::DragFloat3("Rotate", &target->transform.rotate.x, 0.1f);
					ImGui::DragFloat3("Translate", &target->transform.translate.x, 0.1f);
				}
			}
			else {
				ImGui::Text("No objects created.");
			}

			ImGui::End();

#endif
			if (useDebugCamera) {
				debugCamera->DrawFrustum(normalCamera.get());
			}

			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			sprite->materialData->uvTransform = uvTransformMatrix;

			// ----------------------------------------------------
			// 描画処理
			// ----------------------------------------------------
			if (visibleUI) {
				int spriteTexId = textureList[selectedSpriteTexIndex].id;
				sprite->Draw(spriteTexId);
			}

			// 【重要】動的に生成された全オブジェクトを「各自が持つテクスチャID」で描画
			for (auto& obj : g_sceneObjects) {
				obj->Draw(obj->textureId);
			}

			DebugRenderer::Flush(activeCamera);

			dxCommon->PostDraw();

		}
	}

	DebugRenderer::Finalize();
	ebichaEngine->Finalize();

	return 0;
}