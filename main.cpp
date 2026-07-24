#include "EbichaEngine.h"

enum ObjectType {
	plane,
	sphere,
	teapot,
	bunny,
	multiMesh,
	suzanne
};

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの一括初期化
	EbichaEngine* ebichaEngine = EbichaEngine::GetInstance();
	ebichaEngine->Initialize();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Input* input = Input::GetInstance(); // シングルトンインスタンス

	Triangle* triangle = new Triangle;
	triangle->Initialize({ -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f });

	Triangle* triangle2 = new Triangle;
	triangle2->Initialize({ -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, -0.5f, -0.5f, 1.0f });

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
	dxCommon->InitializeTexture("resources/fence.png", 4);

	ObjectType objectType = ObjectType::plane;
	const char* object_names = "plane\0sphere\0teapot\0bunny\0multiMesh\0suzanne\0\0";

	float colorModel4[4] = { modelPlane->color.x, modelPlane->color.y, modelPlane->color.z, modelPlane->color.w };
	float colorModelBunny4[4] = { modelBunny->color.x, modelBunny->color.y, modelBunny->color.z, modelBunny->color.w };
	float colorModelTeapot4[4] = { modelTeapot->color.x, modelTeapot->color.y, modelTeapot->color.z, modelTeapot->color.w };
	float color4[4] = { sphere->color.x, sphere->color.y, sphere->color.z, sphere->color.w };

	// enableLighting
	bool enableLighting = sphere->enableLighting != 0;

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
			ImGui::DragFloat3("CameraRotate", &normalCamera->transform_.rotate.x, 0.1f);
			ImGui::DragFloat3("CameraTranslate", &normalCamera->transform_.translate.x, 0.1f);
			//ImGui::SliderFloat3("CameraRotate", &normalCamera->transform_.rotate.x, -3.14f, 3.14f);

			int currentIndex = static_cast<int>(dxCommon->blendMode_);
			if (ImGui::Combo("Blend Mode", &currentIndex, dxCommon->blendMode_names)) {
				dxCommon->blendMode_ = static_cast<BlendMode>(currentIndex);
				dxCommon->SetBlendMode(dxCommon->blendMode_);
			};

			ImGui::Checkbox("Show UI", &visibleUI);

			ImGui::End();

			//// ビュー行列・射影行列を取得
			//Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
			//Matrix4x4 projectionMatrix = debugCamera.GetProjectionMatrix();
			//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

			// Transformを更新（例：Y軸回転）

			// GPU上のリソース（定数バッファ）の中身を書き換える
			triangle->Update(activeCamera);
			triangle2->Update(activeCamera);
			sprite->Update(activeCamera);

			sphere->Update(activeCamera);
			modelPlane->Update(activeCamera);
			modelTeapot->Update(activeCamera);
			modelBunny->Update(activeCamera);
			modelMultiMesh->Update(activeCamera);

			// グレーのグリッドを床に配置
			DebugRenderer::AddGrid(20.0f, 20, { 0.5f, 0.5f, 0.5f, 1.0f });

			//// 原点に緑色のワイヤーフレーム球体を表示 (半径2.0f, 分割数16)
			//DebugRenderer::AddWireSphere({ 0.0f, 0.0f, 0.0f }, 1.0f, 16, { 0.0f, 1.0f, 0.0f, 1.0f });

			if (input->PushKey(DIK_0)) {
				activeCamera->transform_.rotate.y += 0.03f;
				DebugRenderer::AddLine({ -5.0f, 2.0f, 0.0f }, { 5.0f, 2.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
			}

#ifdef _DEBUG

			ImGui::Begin("Settings");

			//ImGui::ShowDemoWindow();

			int objectIndex = static_cast<int>(objectType);
			if (ImGui::Combo("object", &objectIndex, object_names)) {
				objectType = static_cast<ObjectType>(objectIndex);
			}
			switch (objectType) {
			case ObjectType::plane:

				// color (3Dオブジェクト用マテリアルカラー)
				ImGui::ColorEdit4("color", colorModel4);
				modelPlane->color = { colorModel4[0], colorModel4[1], colorModel4[2], colorModel4[3] };

				ImGui::DragFloat3("Scale",&modelPlane->transform.scale.x, 0.1f);
				ImGui::DragFloat3("Rotate",&modelPlane->transform.rotate.x, 0.1f);
				ImGui::DragFloat3("Translate",&modelPlane->transform.translate.x, 0.1f);

				break;
			case ObjectType::sphere:
				// color (3Dオブジェクト用マテリアルカラー)
				ImGui::ColorEdit4("color", color4);
				sphere->color = { color4[0], color4[1], color4[2], color4[3] };

				ImGui::Checkbox("enableLighting", &enableLighting);
				sphere->enableLighting = enableLighting ? 1 : 0;

				ImGui::DragFloat3("Scale", &sphere->transform.scale.x, 0.1f);
				ImGui::DragFloat3("Rotate", &sphere->transform.rotate.x, 0.1f);
				ImGui::DragFloat3("Translate", &sphere->transform.translate.x, 0.1f);


				// useMonsterBall
				ImGui::Checkbox("useMonsterBall", &useMonsterBall);

				break;
			case ObjectType::teapot:

				// color (3Dオブジェクト用マテリアルカラー)
				ImGui::ColorEdit4("color", colorModelTeapot4);
				modelTeapot->color = { colorModelTeapot4[0], colorModelTeapot4[1], colorModelTeapot4[2], colorModelTeapot4[3] };

				ImGui::DragFloat3("Scale", &modelTeapot->transform.scale.x, 0.1f);
				ImGui::DragFloat3("Rotate", &modelTeapot->transform.rotate.x, 0.1f);
				ImGui::DragFloat3("Translate", &modelTeapot->transform.translate.x, 0.1f);

				break;
			case ObjectType::bunny:

				// color (3Dオブジェクト用マテリアルカラー)
				ImGui::ColorEdit4("color", colorModel4);
				modelBunny->color = { colorModel4[0], colorModel4[1], colorModel4[2], colorModel4[3] };

				ImGui::DragFloat3("Scale", &modelBunny->transform.scale.x, 0.1f);
				ImGui::DragFloat3("Rotate", &modelBunny->transform.rotate.x, 0.1f);
				ImGui::DragFloat3("Translate", &modelBunny->transform.translate.x, 0.1f);

				break;
			case ObjectType::multiMesh:

				//// color (3Dオブジェクト用マテリアルカラー)
				//ImGui::ColorEdit4("color", colorModel4);
				//modelMultiMesh->color = { colorModel4[0], colorModel4[1], colorModel4[2], colorModel4[3] };

				//ImGui::DragFloat3("Scale", &modelMultiMesh->transform.scale.x, 0.1f);
				//ImGui::DragFloat3("Rotate", &modelMultiMesh->transform.rotate.x, 0.1f);
				//ImGui::DragFloat3("Translate", &modelMultiMesh->transform.translate.x, 0.1f);

				break;
			case ObjectType::suzanne:

				break;
			}

				//// LightColor
				//float lightColor4[4] = { sphere->color.x, sphere->color.y, sphere->color.z, sphere->color.w };
				//ImGui::ColorEdit4("LightColor", lightColor4);
				//sphere->color = { lightColor4[0], lightColor4[1], lightColor4[2], lightColor4[3] };

				//// LightDirection
				//ImGui::SliderFloat3("LightDirection", &sphere->directionalLightData->direction.x, -1.0f, 1.0f, "%.2f");

				//// Intensity
				//ImGui::DragFloat("Intensity", &sphere->directionalLightData->intensity, 0.01f);




			//// colorSprite (Sprite用マテリアルカラー)
			//float colorSprite4[4] = { sprite->color.x, sprite->color.y, sprite->color.z, sprite->color.w };
			//ImGui::ColorEdit4("colorSprite", colorSprite4);
			//sprite->color = { colorSprite4[0], colorSprite4[1], colorSprite4[2], colorSprite4[3] };

			//// translateSprite
			//ImGui::DragFloat3("translateSprite", &sprite->transform.translate.x, 0.5f);

			//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

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
			//sphere->Draw(useMonsterBall ? 3 : 2);
			//model->Draw(4);

			switch (objectType) {
			case ObjectType::plane:
				modelPlane->Draw(1);
				break;
			case ObjectType::sphere:
				sphere->Draw(useMonsterBall ? 3 : 2);
				break;
			case ObjectType::teapot:
				modelTeapot->Draw(useMonsterBall ? 3 : 2);
				break;
			case ObjectType::bunny:
				modelBunny->Draw(useMonsterBall ? 3 : 2);
				break;
			case ObjectType::multiMesh:
				//modelMultiMesh->Draw(useMonsterBall ? 3 : 2);
				break;
			case ObjectType::suzanne:
				break;
			}

			DebugRenderer::Flush(activeCamera);

			dxCommon->PostDraw();

		}
	}

	DebugRenderer::Finalize();
	ebichaEngine->Finalize();

	return 0;
}