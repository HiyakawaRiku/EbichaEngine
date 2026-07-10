#include "EbichaEngine.h"


//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	EbichaEngine* ebichaEngine = EbichaEngine::GetInstance();
	ebichaEngine->Initialize();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize();

	Triangle* triangle = new Triangle;
	triangle->Initialize({ -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f });

	Triangle* triangle2 = new Triangle;
	triangle2->Initialize({ -0.5f, -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, -0.5f, -0.5f, 1.0f });

	Sprite* sprite = new Sprite;
	sprite->Initialize();

	Sphere* sphere = new Sphere;
	sphere->Initialize();


	Model* model = new Model;
	model->Initialize("resources", "axis.obj");

	auto normalCamera = std::make_unique<Camera>();
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize();
	Camera* activeCamera = normalCamera.get();
	bool useDebugCamera = false;

	//// 初期化
	//DebugCamera debugCamera;
	//debugCamera.Initialize();

	DebugRenderer::Initialize();

	// CPUで動かす用のTransformを作る
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	bool useMonsterBall = true;

	dxCommon->InitializeTexture("resources/uvChecker.png", 1);
	dxCommon->InitializeTexture("resources/sky_sphere.png", 2);
	dxCommon->InitializeTexture("resources/monsterBall.png", 3);
	dxCommon->InitializeTexture("resources/ground_leaf.png", 4);

	MSG msg{};
	//ウィンドウのxボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {

			dxCommon->PreDraw();

			BYTE key[256] = {};
			UpdateKeyState(dxCommon->keyboard);
			dxCommon->keyboard->GetDeviceState(sizeof(key), key);

			activeCamera->Update();

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
			model->Update(activeCamera);

			// グレーのグリッドを床に配置
			DebugRenderer::AddGrid(20.0f, 20, { 0.5f, 0.5f, 0.5f, 1.0f });

			// 原点に緑色のワイヤーフレーム球体を表示 (半径2.0f, 分割数16)
			DebugRenderer::AddWireSphere({ 0.0f, 0.0f, 0.0f }, 1.0f, 16, { 0.0f, 1.0f, 0.0f, 1.0f });

			if (PushKey(DIK_0)) {
				activeCamera->transform_.rotate.y += 0.03f;
				DebugRenderer::AddLine({ -5.0f, 2.0f, 0.0f }, { 5.0f, 2.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
			}

#ifdef _DEBUG

			ImGui::Begin("Settings");

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

			ImGui::End();

			//ImGui::ShowDemoWindow();

			// === Settings パネル ===
			ImGui::Begin("Settings");

			// CameraTranslate
			ImGui::DragFloat3("CameraTranslate", &activeCamera->transform_.translate.x, 0.01f);

			// CameraRotate (deg)
			ImGui::DragFloat("CameraRotateX", &activeCamera->transform_.rotate.x, 0.1f, -360.0f, 360.0f);
			ImGui::SameLine(); ImGui::Text("deg");
			ImGui::DragFloat("CameraRotateY", &activeCamera->transform_.rotate.y, 0.1f, -360.0f, 360.0f);
			ImGui::SameLine(); ImGui::Text("deg");
			ImGui::DragFloat("CameraRotateZ", &activeCamera->transform_.rotate.z, 0.1f, -360.0f, 360.0f);
			ImGui::SameLine(); ImGui::Text("deg");

			// color (3Dオブジェクト用マテリアルカラー)
			float color4[4] = { sphere->color.x, sphere->color.y, sphere->color.z, sphere->color.w };
			ImGui::ColorEdit4("color", color4);
			sphere->color = { color4[0], color4[1], color4[2], color4[3] };

			// enableLighting
			bool enableLighting = sphere->materialData->enableLighting != 0;
			ImGui::Checkbox("enableLighting", &enableLighting);
			sphere->materialData->enableLighting = enableLighting ? 1 : 0;

			// colorSprite (Sprite用マテリアルカラー)
			float colorSprite4[4] = { sprite->color.x, sprite->color.y, sprite->color.z, sprite->color.w };
			ImGui::ColorEdit4("colorSprite", colorSprite4);
			sprite->color = { colorSprite4[0], colorSprite4[1], colorSprite4[2], colorSprite4[3] };

			// translateSprite
			ImGui::DragFloat3("translateSprite", &sprite->transform.translate.x, 0.5f);

			// useMonsterBall
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);

			// LightColor
			float lightColor4[4] = { sphere->color.x, sphere->color.y, sphere->color.z, sphere->color.w };
			ImGui::ColorEdit4("LightColor", lightColor4);
			sphere->color = { lightColor4[0], lightColor4[1], lightColor4[2], lightColor4[3] };

			// LightDirection
			ImGui::SliderFloat3("LightDirection", &sphere->directionalLightData->direction.x, -1.0f, 1.0f, "%.2f");

			// Intensity
			ImGui::DragFloat("Intensity", &sphere->directionalLightData->intensity, 0.01f);

			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

			ImGui::End();

#endif


			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			sprite->materialData->uvTransform = uvTransformMatrix;

			triangle->Draw(3);
			triangle2->Draw(2);
			sprite->Draw(1);
			sphere->Draw(useMonsterBall ? 3 : 2);
			model->Draw(4);

			DebugRenderer::Flush(activeCamera);

			dxCommon->PostDraw();

		}
	}

	DebugRenderer::Finalize();
	ebichaEngine->Finalize();

	return 0;
}