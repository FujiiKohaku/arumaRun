#include "RisingSpike.h"
#include <cmath>

using namespace KamataEngine;

void RisingSpike::Initialize(Model* model, Camera* camera, const Vector3& position, MapChipField* mapChipField) {
	model_ = model;
	camera_ = camera;
	mapChipField_ = mapChipField;

	worldTransform_.Initialize();
	
	// position は通常床の上 Y=17 (床が Y=18)
	// 初期状態では、底面が床の天面に固定されたまま Yスケール 0 に縮んでいるように見せる
	worldTransform_.translation_ = position;
	baseY_ = position.y;
	
	// 初期値は非表示
	worldTransform_.scale_ = { 1.0f, 0.0f, 1.0f };
	// 底面を接地させたままスケール変更するため、中心位置を床の中に下げる
	worldTransform_.translation_.y = baseY_ - 0.5f;

	state_ = State::kHidden;
	riseProgress_ = 0.0f;
	
	soundHandleRise_ = Audio::GetInstance()->LoadWave("changeMode.wav"); // 飛び出し音として changeMode 音を再利用
	playedSound_ = false;
}

void RisingSpike::Update(float playerX) {
	switch (state_) {
	case State::kHidden:
		// プレイヤーが手前 9.0 ユニット以内に近づいたら飛び出し開始
		if (playerX >= worldTransform_.translation_.x - 9.0f) {
			state_ = State::kRising;
			
			// 飛び出し音
			if (!playedSound_) {
				uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleRise_);
				Audio::GetInstance()->SetVolume(handle, 0.08f); // 飛び出し音はやや控えめに
				playedSound_ = true;
			}
		}
		break;

	case State::kRising:
		// 高速でせり出す (約10フレームで完了)
		riseProgress_ += 0.1f;
		if (riseProgress_ >= 1.0f) {
			riseProgress_ = 1.0f;
			state_ = State::kActive;
		}

		// スケールと座標を同期して、底面を固定したままニョキッと縦に伸ばす
		worldTransform_.scale_.y = riseProgress_;
		worldTransform_.translation_.y = baseY_ - (1.0f - riseProgress_) * 0.5f;
		break;

	case State::kActive:
		// 出きった後は通常位置で固定
		worldTransform_.scale_.y = 1.0f;
		worldTransform_.translation_.y = baseY_;
		break;
	}

	WorldTransformUpdate(worldTransform_);
}

void RisingSpike::Draw() {
	if (model_ && camera_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

AABB RisingSpike::GetAABB() const {
	// 現在のせり出し高さ (riseProgress_ * 1.0f) に応じて当たり判定の箱も伸縮
	float currentHalfHeight = riseProgress_ * 0.5f;
	AABB aabb = {
		{ worldTransform_.translation_.x - 0.4f, worldTransform_.translation_.y - currentHalfHeight, -0.5f },
		{ worldTransform_.translation_.x + 0.4f, worldTransform_.translation_.y + currentHalfHeight, 0.5f }
	};
	return aabb;
}
