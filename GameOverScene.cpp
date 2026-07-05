#include "GameOverScene.h"
#include <algorithm>
#include <numbers>

//==================================================
// 初期化
//==================================================
void GameOverScene::Initialize() {
	// フェード
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// カメラ
	camera_ = new Camera();
	camera_->Initialize();

	// スカイドーム
	modelSkyDome_ = Model::CreateFromOBJ("skydome", true);
	worldTransformSky_.Initialize();

	// 「GAME OVER」文字
	gameOverFont_ = Model::CreateFromOBJ("gameover", true);
	worldTransformGameover_.Initialize();
	worldTransformGameover_.rotation_.x = std::numbers::pi_v<float> / 2.0f;
	worldTransformGameover_.rotation_.y = std::numbers::pi_v<float>;
	worldTransformGameover_.translation_.z = -40.0f;

	// 「Press Enter」文字
	retryFont_ = Model::CreateFromOBJ("pushToEnter", true);
	worldTransformPushTo_.Initialize();
	worldTransformPushTo_.rotation_.x = std::numbers::pi_v<float> / 2.0f;
	worldTransformPushTo_.rotation_.y = std::numbers::pi_v<float>;
	worldTransformPushTo_.translation_.z = -40.0f;
	worldTransformPushTo_.translation_.y = -2.0f;

	// 状態初期化
	state_ = State::WaitInput;

	soundHandleGameOver_ = Audio::GetInstance()->LoadWave("gameover.wav");
	Audio::GetInstance()->PlayWave(soundHandleGameOver_);

	soundHandleSelect_ = Audio::GetInstance()->LoadWave("serect.wav");
}

//==================================================
// 更新
//==================================================
void GameOverScene::Update() {
	// フェード更新
	if (fade_) {
		fade_->Update();
	}

	//==========================================
	// ゲームオーバー文字のアニメーション（ぴょんぴょん）
	//==========================================
	static float timer = 0.0f;
	timer += 1.0f / 60.0f;

	float baseY = -1.0f;
	float jump = 0.5f * fabsf(sinf(timer * 3.0f)); // ぴょんぴょん動く
	worldTransformGameover_.translation_.y = baseY + jump;

	//==========================================
	// 状態管理
	//==========================================
	switch (state_) {
	case State::WaitInput:
		// Enter押されたらフェードアウト開始
		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			Audio::GetInstance()->PlayWave(soundHandleSelect_);
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			state_ = State::FadeOut;
		}
		break;

	case State::FadeOut:
		// フェード完了でシーン終了
		if (fade_->IsFinished()) {
			result_ = Result::Title;
			finished_ = true;
			state_ = State::Finish;
		}
		break;

	case State::Finish:
		// もう何もしない
		break;
	}

	// ワールド・カメラ更新
	WorldTransformUpdate(worldTransformSky_);
	WorldTransformUpdate(worldTransformGameover_);
	WorldTransformUpdate(worldTransformPushTo_);
	camera_->TransferMatrix();
}

//==================================================
// 描画
//==================================================
void GameOverScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// 3Dモデル描画開始
	Model::PreDraw(commandList);

	// 背景
	if (modelSkyDome_) {
		modelSkyDome_->Draw(worldTransformSky_, *camera_);
	}

	// 「GAME OVER」文字
	if (gameOverFont_) {
		gameOverFont_->Draw(worldTransformGameover_, *camera_);
	}

	// 「Press Enter」文字
	if (retryFont_) {
		retryFont_->Draw(worldTransformPushTo_, *camera_);
	}

	// 3Dモデル描画終了
	Model::PostDraw();

	// フェード描画（最後に）
	if (fade_) {
		fade_->Draw();
	}
}
