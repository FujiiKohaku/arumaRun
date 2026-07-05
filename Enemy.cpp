#define NOMINMAX
#define _USE_MATH_DEFINES
#include "Enemy.h"
#include "HitEffect.h"
#include "MapChipField.h"
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <cmath>

//--------------------------------------------------
// 初期化
//--------------------------------------------------
void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, MapChipField* mapChipField) {
	assert(model);
	model_ = model;
	camera_ = camera;
	mapChipField_ = mapChipField;

	// ワールド変換初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期速度（左移動）
	velocity_ = {-kWalkSpeed, 0, 0};

	// 左向きに回転
	worldTransform_.rotation_.y = -float(M_PI) / 2.0f;

	// 状態リセット
	behavior_ = Behavior::kNormal;
	isDead_ = false;
	deathTimer_ = 0;
	isCollisionDisabled_ = false;
}

//--------------------------------------------------
// 更新
//--------------------------------------------------
void Enemy::UpDate() {
	if (behavior_ == Behavior::kNormal) {
		static float time = 0.0f;
		time += 0.05f;

		// ====== ブロック衝突チェック ======
		KamataEngine::Vector3 nextPos = worldTransform_.translation_ + velocity_;
		KamataEngine::Vector3 checkPos = nextPos;

		if (velocity_.x > 0) {
			checkPos.x += kWidth / 2.0f; // 右移動中は右前方
		} else {
			checkPos.x -= kWidth / 2.0f; // 左移動中は左前方
		}

		auto index = mapChipField_->GetMapChipIndexSetByposition(checkPos);
		auto mapChip = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		if (mapChip == MapChipType::kBlock) {
			// ブロックに当たる → 反転
			velocity_.x *= -1.0f;
			if (velocity_.x > 0) {
				worldTransform_.rotation_.y = float(M_PI) / 2.0f; // 右向き
			} else {
				worldTransform_.rotation_.y = -float(M_PI) / 2.0f; // 左向き
			}
		} else if (mapChip == MapChipType::kSpike) {
			// トゲに触れたら即死
			behavior_ = Behavior::kDefeated;
			isCollisionDisabled_ = true;
			deathTimer_ = 0;
		}

		// 移動
		worldTransform_.translation_.x += velocity_.x;

		// 呼吸スケール
		float sleepScaleY = 1.0f + 0.3f * std::sin(time);
		worldTransform_.scale_ = {1.0f, sleepScaleY, 1.0f};

		// 首振り
		float swing = 0.2f * std::sin(time * 2.0f);
		worldTransform_.rotation_.y += swing;

	} else if (behavior_ == Behavior::kDefeated) {
		// ====== デス演出 ======
		deathTimer_++;

		worldTransform_.translation_.y += 0.05f; // 浮く
		worldTransform_.rotation_.z += 0.2f;     // 回転

		float t = deathTimer_ / 60.0f;
		float scale = std::max(0.0f, 1.0f - t);
		worldTransform_.scale_ = {scale, scale, scale};

		if (deathTimer_ > 60) {
			isDead_ = true;
		}
	}

	WorldTransformUpdate(worldTransform_);
}

//--------------------------------------------------
// 描画
//--------------------------------------------------
void Enemy::Draw() {
	if (!isDead_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

//--------------------------------------------------
// AABB取得
//--------------------------------------------------
AABB Enemy::GetAABB() {
	KamataEngine::Vector3 worldPos = GetWorldPosition();
	return {
	    {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
        {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
    };
}

//--------------------------------------------------
// ワールド座標取得
//--------------------------------------------------
KamataEngine::Vector3 Enemy::GetWorldPosition() { return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]}; }

//--------------------------------------------------
// プレイヤー衝突時
//--------------------------------------------------
void Enemy::OnCollision(const Player* player) {
	(void)player;
}
bool Enemy::IsDefeated() const { return (behavior_ == Behavior::kDefeated || isDead_); }