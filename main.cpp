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

	//Triangle* triangle = new Triangle;
	//triangle->Initialize({ -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f });

	//Triangle* triangle2 = new Triangle;
	//triangle2->Initialize({ -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, -0.5f, -0.5f, 1.0f });

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
	//Model* modelSuzanne = new Model;
	//modelSuzanne->Initialize("resources", "suzanne.obj");

	auto normalCamera = std::make_unique<Camera>();
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize();
	Camera* activeCamera = normalCamera.get();
	bool useDebugCamera = false;

	//// 初期化
	//DebugCamera debugCamera;
	//debugCamera.Initialize();


	// CPUで動かす用のTransformを作る
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	bool useMonsterBall = true;
	bool visibleUI = true;

	dxCommon->InitializeTexture("resources/uvChecker.png", 1);
	dxCommon->InitializeTexture("resources/sky_sphere.png", 2);
	dxCommon->InitializeTexture("resources/monsterBall.png", 3);
	dxCommon->InitializeTexture("resources/ground_leaf.png", 4);
	dxCommon->InitializeTexture("resources/fence.png", 5);
	dxCommon->InitializeTexture("resources/checkerBoard.png", 6);

	ObjectType objectType = ObjectType::plane;
	const char* object_names = "plane\0sphere\0teapot\0bunny\0multiMesh\0suzanne\0\0";

	LightingMode lightingMode = LightingMode::none;
	const char* lightingMode_names = "none\0lambert\0halfLambert\0\0";


	float colorModel4[4] = { modelPlane->color.x, modelPlane->color.y, modelPlane->color.z, modelPlane->color.w };
	float colorModelBunny4[4] = { modelBunny->color.x, modelBunny->color.y, modelBunny->color.z, modelBunny->color.w };
	float colorModelTeapot4[4] = { modelTeapot->color.x, modelTeapot->color.y, modelTeapot->color.z, modelTeapot->color.w };
	float color4[4] = { sphere->color.x, sphere->color.y, sphere->color.z, sphere->color.w };

	// モデルの生成状態フラグ（初期状態は false）
	bool isModelCreated = false;

	MSG msg{};
	//ウィンドウのxボタンが押されるまでループ
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
				// 【劇的変化】インスタンスを破壊せず、指す先を切り替えるだけ！
				if (useDebugCamera) {
					activeCamera = debugCamera.get();
				}
				else {
					activeCamera = normalCamera.get();
				}
			}


			// CameraTranslate
			//ImGui::DragFloat3("CameraScale", &normalCamera->transform_.scale.x, 0.1f);
			ImGui::DragFloat3("CameraRotate", &normalCamera->transform_.rotate.x, 0.01f);
			ImGui::DragFloat3("CameraTranslate", &normalCamera->transform_.translate.x, 0.01f);
			//ImGui::SliderFloat3("CameraRotate", &normalCamera->transform_.rotate.x, -3.14f, 3.14f);

			int currentIndex = static_cast<int>(dxCommon->blendMode_);
			if (ImGui::Combo("Blend Mode", &currentIndex, dxCommon->blendMode_names)) {
				dxCommon->blendMode_ = static_cast<BlendMode>(currentIndex);
				dxCommon->SetBlendMode(dxCommon->blendMode_);
			};

			ImGui::Checkbox("Show UI", &visibleUI);

			if (visibleUI) {
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

			int lightingIndex = static_cast<int>(lightingMode);
			if (ImGui::Combo("Lighting Mode", &lightingIndex, lightingMode_names)) {
				lightingMode = static_cast<LightingMode>(lightingIndex);
			}
			switch (lightingMode) {
			case LightingMode::none:	for (auto& obj : g_sceneObjects) {

				obj->lightingType = LightType_None;
			}
								   break;
			case LightingMode::lambert:	for (auto& obj : g_sceneObjects) {

				obj->lightingType = LightType_Lambert;
			}
									  break;
			case LightingMode::halfLambert:	for (auto& obj : g_sceneObjects) {

				obj->lightingType = LightType_HalfLambert;
			}
										  break;
			}
			// ----------------------------------------------------
// Global Setting ウィンドウ内（ImGui）
// ----------------------------------------------------

// LightColor
			float lightColor4[4] = { sphere->directionalLightData->color.x, sphere->directionalLightData->color.y, sphere->directionalLightData->color.z, sphere->directionalLightData->color.w };
				if (ImGui::ColorEdit4("LightColor", lightColor4)) {
					Vector4 newColor = { lightColor4[0], lightColor4[1], lightColor4[2], lightColor4[3] };
					sphere->directionalLightData->color = newColor;
						// 生成された全オブジェクトにも反映[cite: 10]
						for (auto& obj : g_sceneObjects) {
							if (obj->directionalLightData) obj->directionalLightData->color = newColor;
						}
				}

			// LightDirection (ImGuiで直接アドレスを渡して操作)
			if (ImGui::SliderFloat3("LightDirection", &sphere->directionalLightData->direction.x, -1.0f, 1.0f, "%.2f")) {
				Vector3 newDir = sphere->directionalLightData->direction;
					// 生成された全オブジェクトにも反映[cite: 10]
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


			// ----------------------------------------------------
			// ライト位置の可視化（DebugRenderer::Flush の直前に記述）
			// ----------------------------------------------------
			Vector3 lightDir = sphere->directionalLightData->direction;

				// 方向ベクトルを正規化（長さ1にする）して計算を安定させる
				float len = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
			if (len > 0.001f) {
				lightDir.x /= len;
				lightDir.y /= len;
				lightDir.z /= len;
			}

			// 光が射してくる反対方向（空）に 5.0f 離した位置を計算
			Vector3 lightPos = {
				-lightDir.x * 5.0f,
				-lightDir.y * 5.0f,
				-lightDir.z * 5.0f
			};

			// 計算された位置に黄色のワイヤーフレーム球体を描画
			DebugRenderer::AddWireSphere(lightPos, 0.5f, 12, { 1.0f, 1.0f, 0.0f, 1.0f });

			ImGui::End();

			//// ビュー行列・射影行列を取得
			//Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
			//Matrix4x4 projectionMatrix = debugCamera.GetProjectionMatrix();
			//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

			// Transformを更新（例：Y軸回転）

			// GPU上のリソース（定数バッファ）の中身を書き換える
			//triangle->Update(activeCamera);
			//triangle2->Update(activeCamera);
// 既存のUpdate呼び出し
			sprite->Update(activeCamera);
			sphere->Update(activeCamera);
			modelPlane->Update(activeCamera);
			modelTeapot->Update(activeCamera);
			modelBunny->Update(activeCamera);
			modelMultiMesh->Update(activeCamera);

			// ----------------------------------------------------
			// 【追加】動的に生成された全オブジェクトの更新 (ポリモーフィズム)
			// ----------------------------------------------------
			for (auto& obj : g_sceneObjects) {
				obj->Update(activeCamera);
			}

			// グレーのグリッドを床に配置
			DebugRenderer::AddGrid(20.0f, 20, { 0.5f, 0.5f, 0.5f, 1.0f });

			//// 原点に緑色のワイヤーフレーム球体を表示 (半径2.0f, 分割数16)
			//DebugRenderer::AddWireSphere({ 0.0f, 0.0f, 0.0f }, 1.0f, 16, { 0.0f, 1.0f, 0.0f, 1.0f });

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

			// --- 2. Create ボタン (何回でも(0,0,0)に生成) ---
			if (ImGui::Button("Create")) {
				CreateObject(objectType, activeCamera); // activeCamera を渡す
			}

			ImGui::Separator();
			ImGui::Text("Created Objects (%d)", static_cast<int>(g_sceneObjects.size()));

			// --- 3. 生成されたオブジェクトの一覧・編集・削除 ---
			if (!g_sceneObjects.empty()) {
				// オブジェクト一覧を表示するリストボックス
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

				// --- Delete ボタン ---
				if (selectedObjectIndex >= 0 && selectedObjectIndex < g_sceneObjects.size()) {
					if (ImGui::Button("Delete Selected")) {
						// unique_ptr なので erase された瞬間に自動でメモリ解放されます
						g_sceneObjects.erase(g_sceneObjects.begin() + selectedObjectIndex);

						if (selectedObjectIndex >= g_sceneObjects.size()) {
							selectedObjectIndex = static_cast<int>(g_sceneObjects.size()) - 1;
						}
					}
				}

				// 全部一括消去したい場合用
				ImGui::SameLine();
				if (ImGui::Button("Delete All")) {
					g_sceneObjects.clear();
					selectedObjectIndex = -1;
				}

				// --- 選択中のオブジェクトの Transform や Color 調整 ---
				if (selectedObjectIndex >= 0 && selectedObjectIndex < g_sceneObjects.size()) {
					auto& target = g_sceneObjects[selectedObjectIndex];
					ImGui::Separator();
					ImGui::Text("Edit Target: Object %d", selectedObjectIndex + 1);

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

			//triangle->Draw(3);
			//triangle2->Draw(2);
			if (visibleUI) {
				sprite->Draw(1);
			}

			//// 既存の描画処理 (切り替え表示)
			//switch (objectType) {
			//case ObjectType::plane:
			//	modelPlane->Draw(1);
			//	break;
			//case ObjectType::sphere:
			//	sphere->Draw(useMonsterBall ? 3 : 2);
			//	break;
			//case ObjectType::teapot:
			//	modelTeapot->Draw(6);
			//	break;
			//case ObjectType::bunny:
			//	modelBunny->Draw(4);
			//	break;
			//case ObjectType::multiMesh:
			//	modelMultiMesh->Draw(useMonsterBall ? 3 : 2);
			//	break;
			//case ObjectType::suzanne:
			//	break;
			//}

			// ----------------------------------------------------
			// 【追加】動的に生成された全オブジェクトの描画
			// ----------------------------------------------------
			for (auto& obj : g_sceneObjects) {
				// 描画したいテクスチャ番号（例: 1 や 2 など）を渡して描画
				obj->Draw(1);
			}

			DebugRenderer::Flush(activeCamera);

			dxCommon->PostDraw();

		}
	}

	DebugRenderer::Finalize();
	ebichaEngine->Finalize();

	return 0;
}