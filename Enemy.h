#pragma once
#define NOMINMAX
#include "KamataEngine.h"
#include "Math.h"

class Player;
class MapChipField; // 前方宣言

class Enemy {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, MapChipField* mapChipField);

	// 更新
	void UpDate();

	// 描画
	void Draw();

	// 当たり判定
	AABB GetAABB();
	KamataEngine::Vector3 GetWorldPosition();

	// プレイヤー衝突時
	void OnCollision(const Player* player);

	// 生存判定
	bool IsDead() const { return isDead_; }
	bool IsDefeated() const;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	MapChipField* mapChipField_ = nullptr; 

	// 歩行速度
	static inline const float kWalkSpeed = 0.01f;

	// サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 行動状態
	enum class Behavior { kNormal, kDefeated };
	Behavior behavior_ = Behavior::kNormal;

	// 管理フラグ
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;

	// デス演出用
	int deathTimer_ = 0;
};
