#include "ClearScene.h" 
#include "GameOverScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "base/WinApp.h"
#include "TitleScene.h"
#include <Windows.h>

// シーンの種類
enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame,
	kGameOver,
	kClear, 
};

// シーンのポインタ
TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;
GameOverScene* gameOverScene = nullptr;
ClearScene* clearScene = nullptr; // ★

// 現在のシーン
Scene scene = Scene::kUnknown;

//--------------------------------------------------
// シーン切り替え
//--------------------------------------------------
void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// === タイトルからゲームへ ===
			scene = Scene::kGame;
			delete titleScene;
			titleScene = nullptr;

			gameScene = new GameScene;
			gameScene->Initialize();
		}
		break;

	case Scene::kGame:
		if (gameScene->IsFinished()) {

			// === Pause → Enter でタイトルに戻る ===
			if (gameScene->IsReturnToTitle()) {
				scene = Scene::kTitle;
				delete gameScene;
				gameScene = nullptr;

				titleScene = new TitleScene;
				titleScene->Initialize();
			}
			// === ゴールしてクリア ===
			else if (gameScene->IsClear()) {
				scene = Scene::kClear;
				delete gameScene;
				gameScene = nullptr;

				clearScene = new ClearScene;
				clearScene->Initialize();
			}
			// === 死亡したらゲームオーバー ===
			else {
				scene = Scene::kGameOver;
				delete gameScene;
				gameScene = nullptr;

				gameOverScene = new GameOverScene;
				gameOverScene->Initialize();
			}
		}
		break;

	case Scene::kGameOver:
		if (gameOverScene->IsFinished()) {
			// === ゲームオーバーからタイトルへ ===
			scene = Scene::kTitle;
			delete gameOverScene;
			gameOverScene = nullptr;

			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;

	case Scene::kClear:
		if (clearScene->IsFinished()) {
			// === クリア後はタイトルへ ===
			scene = Scene::kTitle;
			delete clearScene;
			clearScene = nullptr;

			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;

	default:
		break;
	}
}


//--------------------------------------------------
// シーン更新
//--------------------------------------------------
void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	case Scene::kGameOver:
		gameOverScene->Update();
		break;
	case Scene::kClear:
		clearScene->Update();
		break;
	default:
		break;
	}
}

//--------------------------------------------------
// シーン描画
//--------------------------------------------------
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	case Scene::kGameOver:
		gameOverScene->Draw();
		break;
	case Scene::kClear:
		clearScene->Draw();
		break;
	default:
		break;
	}
}

//--------------------------------------------------
// エントリーポイント
//--------------------------------------------------
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジン初期化
	KamataEngine::Initialize();

	// デバッグテキストの初期化
	DebugText::GetInstance()->Initialize();

	// DirectX共通インスタンス
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// 最初はタイトル
	scene = Scene::kTitle;
	titleScene = new TitleScene;
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジン更新（×ボタンで閉じたら終了）
		if (KamataEngine::Update())
			break;

		// ===== アプリケーションウィンドウのアイコンを強制適用 =====
		HWND hwnd = KamataEngine::WinApp::GetInstance()->GetHwnd();
		if (hwnd) {
			// 自プロセス(nullptr)のリソースID: 1 からアイコンをロード
			HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
			if (hIcon) {
				SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			}
		}

		// ImGui受付開始
		imguiManager->Begin();

		// シーン遷移チェック
		ChangeScene();

		// シーン更新
		UpdateScene();

		// ImGui受付終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		AxisIndicator::GetInstance()->Draw();
		PrimitiveDrawer::GetInstance()->Reset();

		// シーン描画
		DrawScene();

		// ImGui描画
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	// 解放処理（残ってるシーンだけ delete）
	delete gameScene;
	delete titleScene;
	delete gameOverScene;
	delete clearScene;

	gameScene = nullptr;
	titleScene = nullptr;
	gameOverScene = nullptr;
	clearScene = nullptr;

	// エンジン終了
	KamataEngine::Finalize();
	return 0;
}
