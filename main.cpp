#include "DirectXCommon.h"
#include "Matrix.h"
#include "Mesh.h"
#include "Sprite.h"
#include "Camera.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int){
	
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

	Mesh* mesh=new Mesh;
	mesh->Initialize();

	Sprite* sprite = new Sprite;
	sprite->Initialize();

	Mesh* sphere = new Mesh;
	uint32_t kSubdivision = 16;
	sphere->InitializeSphere(kSubdivision);

	Camera* camera = new Camera;


	// Transform変数を作る
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	// CPUで動かす用のTransformを作る
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };


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
			*mesh->wvpData = camera->DrawObject3d(transform);
			*mesh->materialData = { 0.0f,0.0f,0.0f,1.0f };
			
			*sprite->transformationMatrixData = camera->DrawObject2d(transformSprite);
			*sprite->materialData = { 1.0f,0.0f,1.0f,0.0f };

			*sphere->wvpData = camera->DrawObject3d(transform);

#ifdef USE_IMGUI
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
			//ImGui::ShowDemoWindow();

			ImGui::Checkbox("useMonsterBall", &mesh->useMonsterBall);

			// ImGuiの内部コマンドを生成する
			ImGui::Render();
#endif

			mesh->Draw(6);
			sprite->Draw(6);
			sphere->DrawSphere(kSubdivision);

			// 実際のcommandListのImGuiの描画コマンドを積む
#ifdef USE_IMGUI
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());
#endif

			dxCommon->PostDraw();
		
		}
	}

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	// こういうもんである。初期化と逆順に行う
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	CoUninitialize();

	return 0;
}