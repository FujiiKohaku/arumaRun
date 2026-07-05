#include "TitleScene.h"
#include "Math.h"
#include <numbers>

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
TitleScene::~TitleScene() { delete fade_; }

//==================================================
// 初期化
//==================================================
void TitleScene::Initialize() {

	//--------------------------------------------------
	// モデル生成
	//--------------------------------------------------
	modelTitle_ = Model::CreateFromOBJ("aru", true);
	modelPlayer_ = Model::CreateFromOBJ("playermax");
	sun_ = Model::CreateFromOBJ("isLand", true);
	backGround_ = Model::CreateFromOBJ("sora", true);
	pushSpace_ = Model::CreateFromOBJ("pushSpace", true);

	BackGroundTexture_ = TextureManager::Load("mario.jpg");

	//--------------------------------------------------
	// カメラ初期化
	//--------------------------------------------------
	camera_.Initialize();

	//--------------------------------------------------
	// タイトル文字 Transform
	//--------------------------------------------------
	const float kPlayerTitle = 2.0f;
	worldTransformTitle_.Initialize();
	worldTransformTitle_.scale_ = {kPlayerTitle, kPlayerTitle, kPlayerTitle};

	//--------------------------------------------------
	// プレイヤー Transform
	//--------------------------------------------------
	const float kPlayerScale = 3.0f;
	worldTransformPlayer_.Initialize();
	worldTransformPlayer_.scale_ = {kPlayerScale, kPlayerScale, kPlayerScale};
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	//--------------------------------------------------
	// Sun Transform
	//--------------------------------------------------
	worldTransformSun_.Initialize();
	worldTransformSun_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransformSun_.scale_ = {9.0f, 9.0f, 9.0f};

	//--------------------------------------------------
	// 背景 Transform
	//--------------------------------------------------
	worldTransformBack_.Initialize();
	const float bgScale = 30.0f;
	worldTransformBack_.scale_ = {bgScale, bgScale, bgScale};
	worldTransformBack_.translation_.z = 60.0f;
	worldTransformBack_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	//--------------------------------------------------
	// PushSpace Transform
	//--------------------------------------------------
	worldTransformPushSpace_.Initialize();
	worldTransformPushSpace_.scale_ = {3.0f, 3.0f, 3.0f};
	worldTransformPushSpace_.translation_.z = -10.0f;
	worldTransformPushSpace_.translation_.y = -10.0f;
	worldTransformPushSpace_.rotation_.x = std::numbers::pi_v<float> / 2.0f;
	worldTransformPushSpace_.rotation_.y = std::numbers::pi_v<float>;

	//-------------------------------------------------
	// タイトル
	//-------------------------------------------------
	worldTransformTitle_.rotation_.x = std::numbers::pi_v<float> / 2.0f;
	worldTransformTitle_.rotation_.y = std::numbers::pi_v<float>;
	worldTransformTitle_.translation_.z = -5.0f;

	//--------------------------------------------------
	// フェード生成
	//--------------------------------------------------
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	//--------------------------------------------------
	// フェーズ初期化
	//--------------------------------------------------
	phase_ = Phase::kFadeIn;

	soundHandleSelecct = Audio::GetInstance()->LoadWave("serect.wav");
}

//==================================================
// 更新
//==================================================
void TitleScene::Update() {

	//--------------------------------------------------
	// フェーズ管理
	//--------------------------------------------------
	switch (phase_) {
	case Phase::kFadeIn: {
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
	} break;

	case Phase::kMain: {
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			Audio::GetInstance()->PlayWave(soundHandleSelecct);
			phase_ = Phase::kFadeOut;
		}
	} break;

	case Phase::kFadeOut: {
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
	} break;
	}

	//--------------------------------------------------
	// タイトル文字の上下動アニメーション
	//--------------------------------------------------
	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_, kTimeTitleMove);

	float angleTitle = counter_ / kTimeTitleMove * 2.0f * std::numbers::pi_v<float>;
	worldTransformTitle_.translation_.y = std::sin(angleTitle) + 10.0f;

	//--------------------------------------------------
	// プレイヤーを円運動＋ふわふわさせる
	//--------------------------------------------------
	angle_ += 0.01f;            // 回転速度
	const float radius = 30.0f; // 回転半径

	// 中心座標
	Vector3 center = {0.0f, -10.0f, 0.0f};

	// 円運動（XZ平面）
	worldTransformPlayer_.translation_.x = center.x + radius * std::cos(angle_);
	worldTransformPlayer_.translation_.z = center.z + radius * std::sin(angle_);

	// ふわふわ（Y座標をsin波で変化）
	const float baseHeight = 5.0f;     // 基準高さ
	const float floatAmplitude = 2.0f; // 振幅
	const float floatSpeed = 0.05f;    // 速度
	worldTransformPlayer_.translation_.y = baseHeight + floatAmplitude * std::sin(angle_ * (1.0f / floatSpeed));

	// プレイヤーの向き
	worldTransformPlayer_.rotation_.y = angle_ + std::numbers::pi_v<float> / 2.0f;

	//--------------------------------------------------
	// カメラ更新
	//--------------------------------------------------
	camera_.TransferMatrix();

	//--------------------------------------------------
	// ワールド行列更新
	//--------------------------------------------------
	WorldTransformUpdate(worldTransformTitle_);
	WorldTransformUpdate(worldTransformPlayer_);
	WorldTransformUpdate(worldTransformSun_);
	WorldTransformUpdate(worldTransformBack_);
	WorldTransformUpdate(worldTransformPushSpace_);
}

//==================================================
// 描画
//==================================================
void TitleScene::Draw() {
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	// モデル描画
	modelTitle_->Draw(worldTransformTitle_, camera_);
	modelPlayer_->Draw(worldTransformPlayer_, camera_);
	sun_->Draw(worldTransformSun_, camera_);
	backGround_->Draw(worldTransformBack_, camera_);
	pushSpace_->Draw(worldTransformPushSpace_, camera_);

	// フェード描画
	fade_->Draw();

	Model::PostDraw();
}
