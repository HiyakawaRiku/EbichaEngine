#include "EbichaEngine.h"


//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	EbichaEngine* ebichaEngine = EbichaEngine::GetInstance();
	ebichaEngine->Initialize();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize();
	DebugRenderer::Initialize();

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

			normalCamera->Update();

			if (activeCamera != normalCamera.get()) {
				activeCamera->Update();
			}

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
			ImGui::DragFloat3("CameraTranslate", &normalCamera->transform_.translate.x, 0.1f);

			// CameraRotate (deg)
			ImGui::SliderFloat3("CameraRotate", &normalCamera->transform_.rotate.x, -3.14f, 3.14f);
			

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
			if (useDebugCamera) {
				// 通常カメラの現在の座標を取得
				Vector3 camPos = normalCamera->transform_.translate;
				Vector3 camRot = normalCamera->transform_.rotate;

				// --- 1. 最もシンプルな「カメラ位置に赤・緑・青の十字線を引く」 ---
				float axisLength = 5.0f; // 線の長さ

				// カメラのローカル方向（前・上・右）のベクトルを計算
				Matrix4x4 camRotMat = MakeRotateMatrix(camRot);
				Vector3 forward = Transforms({ 0.0f, 0.0f, 1.0f }, camRotMat);
				Vector3 up = Transforms({ 0.0f, 1.0f, 0.0f }, camRotMat);
				Vector3 right = Transforms({ 1.0f, 0.0f, 0.0f }, camRotMat);

				// X軸（右方向）：赤
				DebugRenderer::AddLine(camPos, { camPos.x + right.x * axisLength, camPos.y + right.y * axisLength, camPos.z + right.z * axisLength }, { 1.0f, 0.0f, 0.0f, 1.0f });
				// Y軸（上方向）：緑
				DebugRenderer::AddLine(camPos, { camPos.x + up.x * axisLength,    camPos.y + up.y * axisLength,    camPos.z + up.z * axisLength }, { 0.0f, 1.0f, 0.0f, 1.0f });
				// Z軸（前方向・視線）：青
				DebugRenderer::AddLine(camPos, { camPos.x + forward.x * axisLength, camPos.y + forward.y * axisLength, camPos.z + forward.z * axisLength }, { 0.0f, 0.0f, 1.0f, 1.0f });


				// --- 2. ステップアップ：「カメラが見ている視界の箱（簡易四角錐）」を描画する ---
				// カメラの目の前にある「画面の4隅」のローカル座標
				float w = 4.0f; // 横幅
				float h = 3.0f; // 縦幅
				float d = 6.0f; // 前方の深さ

				Vector3 p0 = Transforms({ -w,  h, d }, camRotMat);
				Vector3 p1 = Transforms({ w,  h, d }, camRotMat);
				Vector3 p2 = Transforms({ w, -h, d }, camRotMat);
				Vector3 p3 = Transforms({ -w, -h, d }, camRotMat);

				// 世界の座標（ワールド座標）に変換
				Vector3 w0 = { camPos.x + p0.x, camPos.y + p0.y, camPos.z + p0.z };
				Vector3 w1 = { camPos.x + p1.x, camPos.y + p1.y, camPos.z + p1.z };
				Vector3 w2 = { camPos.x + p2.x, camPos.y + p2.y, camPos.z + p2.z };
				Vector3 w3 = { camPos.x + p3.x, camPos.y + p3.y, camPos.z + p3.z };

				// カメラ位置から4隅へ黄色い線を伸ばす
				Vector4 yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
				DebugRenderer::AddLine(camPos, w0, yellow);
				DebugRenderer::AddLine(camPos, w1, yellow);
				DebugRenderer::AddLine(camPos, w2, yellow);
				DebugRenderer::AddLine(camPos, w3, yellow);

				// 4隅を繋いで四角い枠を作る
				DebugRenderer::AddLine(w0, w1, yellow);
				DebugRenderer::AddLine(w1, w2, yellow);
				DebugRenderer::AddLine(w2, w3, yellow);
				DebugRenderer::AddLine(w3, w0, yellow);
			}

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