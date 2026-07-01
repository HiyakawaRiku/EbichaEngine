#include "DirectXCommon.h"
#include "Matrix.h"
#include "Triangle.h"
#include "Sprite.h"
#include "Camera.h"
#include "Model.h"
#include "Sphere.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	//COMの初期化
	(void)CoInitializeEx(0, COINIT_MULTITHREADED);

	// 誰も捕捉しなかった場合に(Unhandled)、補足する関数を登録
	// main関数はじまってすぐに登録すると良い
	SetUnhandledExceptionFilter(ExportDump);

	WinApp* app = WinApp::GetInstance();
	app->CreateGameWindow();

	CreateLogFile();

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

	Camera* camera = new Camera;

	// Transform変数を作る
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
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

			// Transformを更新（例：Y軸回転）
			transform.rotate.y += 0.03f;

			// GPU上のリソース（定数バッファ）の中身を書き換える
			*triangle->wvpData = camera->DrawObject3d(transform);
			*triangle2->wvpData = camera->DrawObject3d(transform);

			*sprite->wvpData = camera->DrawObject2d(transformSprite);
			*sphere->wvpData = camera->DrawObject3d(transform);
			*model->wvpData = camera->DrawObject3d(transform);

#ifdef _DEBUG

			//ImGui::ShowDemoWindow();

			// === Settings パネル ===
			ImGui::Begin("Settings");

			// CameraTranslate
			ImGui::DragFloat3("CameraTranslate", &camera->transform_.translate.x, 0.01f);

			// CameraRotate (deg)
			ImGui::DragFloat("CameraRotateX", &camera->transform_.rotate.x, 0.1f, -360.0f, 360.0f);
			ImGui::SameLine(); ImGui::Text("deg");
			ImGui::DragFloat("CameraRotateY", &camera->transform_.rotate.y, 0.1f, -360.0f, 360.0f);
			ImGui::SameLine(); ImGui::Text("deg");
			ImGui::DragFloat("CameraRotateZ", &camera->transform_.rotate.z, 0.1f, -360.0f, 360.0f);
			ImGui::SameLine(); ImGui::Text("deg");

			// color (3Dオブジェクト用マテリアルカラー)
			float color4[4] = { sphere->materialData->color.x, sphere->materialData->color.y, sphere->materialData->color.z, sphere->materialData->color.w };
			ImGui::ColorEdit4("color", color4);
			sphere->materialData->color = { color4[0], color4[1], color4[2], color4[3] };

			// enableLighting
			bool enableLighting = sphere->materialData->enableLighting != 0;
			ImGui::Checkbox("enableLighting", &enableLighting);
			sphere->materialData->enableLighting = enableLighting ? 1 : 0;

			// colorSprite (Sprite用マテリアルカラー)
			float colorSprite4[4] = { sprite->materialData->color.x, sprite->materialData->color.y, sprite->materialData->color.z, sprite->materialData->color.w };
			ImGui::ColorEdit4("colorSprite", colorSprite4);
			sprite->materialData->color = { colorSprite4[0], colorSprite4[1], colorSprite4[2], colorSprite4[3] };

			// translateSprite
			ImGui::DragFloat3("translateSprite", &transformSprite.translate.x, 0.5f);

			// useMonsterBall
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);

			// LightColor
			float lightColor4[4] = { sphere->materialData->color.x, sphere->materialData->color.y, sphere->materialData->color.z, sphere->materialData->color.w };
			ImGui::ColorEdit4("LightColor", lightColor4);
			sphere->materialData->color = { lightColor4[0], lightColor4[1], lightColor4[2], lightColor4[3] };

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

			triangle->Draw(6, 3);
			triangle2->Draw(6, 2);
			sprite->Draw(6, 1);
			sphere->Draw(sphere->kSubdivision, useMonsterBall ? 3 : 2);
			model->Draw(4);

			dxCommon->PostDraw();

		}
	}

#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	CoUninitialize();

	return 0;
}