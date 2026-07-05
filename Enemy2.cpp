#include "Enemy2.h"
#include "HitEffect.h"
#include "MapChipField.h"
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <cmath>

void Enemy2::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, MapChipField* mapChipField) {
	assert(model);
	model_ = model;
	camera_ = camera;
	mapChipField_ = mapChipField;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期は上に移動
	velocity_ = {0, kMoveSpeed, 0};

	behavior_ = Behavior::kNormal;
	isDead_ = false;
	deathTimer_ = 0;
	isCollisionDisabled_ = false;
}

void Enemy2::Update() {
	if (behavior_ == Behavior::kNormal) {
		static float time = 0.0f;
		time += 0.05f;

		// 上下に移動
		worldTransform_.translation_ += velocity_;

		if (worldTransform_.translation_.y > 5.0f || worldTransform_.translation_.y < 1.0f) {
			velocity_.y *= -1.0f; // 端で反転
		}

		// 見た目アニメーション（ぷよぷよ）
		float scaleX = 1.0f + 0.2f * std::sin(time * 2.0f);
		float scaleY = 1.0f + 0.2f * std::cos(time * 2.0f);
		worldTransform_.scale_ = {scaleX, scaleY, 1.0f};

	} else if (behavior_ == Behavior::kDefeated) {
		// デス演出
		deathTimer_++;
		worldTransform_.translation_.y += 0.05f;
		worldTransform_.rotation_.z += 0.2f;

		float t = deathTimer_ / 60.0f;
		float scale = std::max(0.0f, 1.0f - t);
		worldTransform_.scale_ = {scale, scale, scale};

		if (deathTimer_ > 60) {
			isDead_ = true;
		}
	}

	WorldTransformUpdate(worldTransform_);
}

void Enemy2::Draw() {
	if (!isDead_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

AABB Enemy2::GetAABB() {
	KamataEngine::Vector3 worldPos = GetWorldPosition();
	return {
	    {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
        {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
    };
}

KamataEngine::Vector3 Enemy2::GetWorldPosition() { return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]}; }

void Enemy2::OnCollision(const Player* player) {
	(void)player;
}

bool Enemy2::IsDefeated() const { return (behavior_ == Behavior::kDefeated || isDead_); }
