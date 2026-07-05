#include "FallingSpike.h"
#include <cmath>

using namespace KamataEngine;

void FallingSpike::Initialize(Model* model, Camera* camera, const Vector3& position, MapChipField* mapChipField) {
	model_ = model;
	camera_ = camera;
	mapChipField_ = mapChipField;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	startPosition_ = position;
	
	// 着地目標Y座標 (Y=16のブロックの中心高さ。床Y=18の上の上の段で止めることで、下1マスのスライディング隙間を空ける)
	auto index = mapChipField_->GetMapChipIndexSetByposition(position);
	Vector3 targetBlockPos = mapChipField_->GetMapChipPositionByIndex(index.xIndex, 16);
	targetY_ = targetBlockPos.y;

	state_ = State::kWaiting;
	shakeTimer_ = 0.0f;
	fallVelocity_ = 0.0f;
	
	soundHandleCrash_ = Audio::GetInstance()->LoadWave("jumpbane.wav"); // 着地音としてバネ音を再利用
	playedCrashSound_ = false;
}

void FallingSpike::Update(float playerX) {
	switch (state_) {
	case State::kWaiting:
		// プレイヤーが画面内（手前 13.0 ユニット以内）に入ったら揺れ開始
		if (playerX >= worldTransform_.translation_.x - 13.0f) {
			state_ = State::kShaking;
			shakeTimer_ = 999.0f; // 落下トリガーまで揺れ続けるためのダミータイマー
		}
		break;

	case State::kShaking:
		shakeTimer_ -= 1.0f / 60.0f;
		// 左右にプルプル揺らす
		worldTransform_.translation_.x = startPosition_.x + sinf(shakeTimer_ * 120.0f) * 0.06f;
		
		// プレイヤーが手前 6.5 ユニットまで近づいたら落下開始
		if (playerX >= worldTransform_.translation_.x - 6.5f) {
			worldTransform_.translation_.x = startPosition_.x; // 座標を戻す
			state_ = State::kFalling;
			fallVelocity_ = 0.0f;
		}
		break;

	case State::kFalling:
		// 急降下
		fallVelocity_ += 0.04f; // 重力加速度 (少し早く落下させるため0.03から0.04へ調整)
		worldTransform_.translation_.y -= fallVelocity_;

		// 着地判定 (Y=16の位置で停止)
		if (worldTransform_.translation_.y <= targetY_) {
			worldTransform_.translation_.y = targetY_;
			state_ = State::kLanded;

			// 着地音
			if (!playedCrashSound_) {
				uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleCrash_);
				Audio::GetInstance()->SetVolume(handle, 0.1f);
				playedCrashSound_ = true;
			}

			// 【マップ書き換え】着地したマスのマップチップ（Y=16）を Block に変更し、
			// プレイヤーが上に乗れるようにする (下1マスのY=17は空白のまま空く)
			auto index = mapChipField_->GetMapChipIndexSetByposition(worldTransform_.translation_);
			mapChipField_->SetMapChipType(index.xIndex, 16, MapChipType::kBlock);
		}
		break;

	case State::kLanded:
		// 着地後は固定
		break;
	}

	WorldTransformUpdate(worldTransform_);
}

void FallingSpike::Draw() {
	if (model_ && camera_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

AABB FallingSpike::GetAABB() const {
	// 通常ブロックと同じサイズ
	AABB aabb = {
		{ worldTransform_.translation_.x - MapChipField::kBlockWidth / 2.0f, worldTransform_.translation_.y - MapChipField::kBlockHeight / 2.0f, -0.5f },
		{ worldTransform_.translation_.x + MapChipField::kBlockWidth / 2.0f, worldTransform_.translation_.y + MapChipField::kBlockHeight / 2.0f, 0.5f }
	};
	return aabb;
}
