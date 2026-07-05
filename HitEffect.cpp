#include "HitEffect.h"
#include <cmath>
#include <cstdlib> // rand(), RAND_MAX

// 静的メンバ
KamataEngine::Model* HitEffect::model_ = nullptr;
KamataEngine::Camera* HitEffect::camera_ = nullptr;

//--------------------------------------------------
// インスタンス生成
//--------------------------------------------------
HitEffect* HitEffect::Create(const KamataEngine::Vector3& position) {
	HitEffect* effect = new HitEffect();
	effect->Initialize(position);
	return effect;
}

//--------------------------------------------------
// 初期化
//--------------------------------------------------
void HitEffect::Initialize(const KamataEngine::Vector3& position) {
	position_ = position;
	finished_ = false;
	lifeTimer_ = 0;

	for (int i = 0; i < kNumPieces; i++) {
		pieces_[i].transform.Initialize();
		pieces_[i].transform.translation_ = position_;
		pieces_[i].transform.scale_ = {0.3f, 0.3f, 0.3f};

		// ランダム方向に飛ばす
		float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
		float speed = 0.05f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f;
		pieces_[i].velocity = {cosf(angle) * speed, 0.1f + (rand() % 100) / 500.0f, sinf(angle) * speed};
	}
}

//--------------------------------------------------
// 更新
//--------------------------------------------------
void HitEffect::Update() {
	if (finished_)
		return;

	lifeTimer_++;

	for (int i = 0; i < kNumPieces; i++) {
		// 移動 + 重力
		pieces_[i].velocity.y -= 0.01f;
		pieces_[i].transform.translation_ += pieces_[i].velocity;

		// 縮小
		pieces_[i].transform.scale_ *= 0.9f;

		// 行列更新
		WorldTransformUpdate(pieces_[i].transform);
	}

	if (lifeTimer_ > lifeTime_) {
		finished_ = true;
	}
}

//--------------------------------------------------
// 描画
//--------------------------------------------------
void HitEffect::Draw() {
	if (finished_)
		return;

	for (int i = 0; i < kNumPieces; i++) {
		model_->Draw(pieces_[i].transform, *camera_);
	}
}
