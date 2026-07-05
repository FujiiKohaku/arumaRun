#pragma once
#define NOMINMAX
#include "KamataEngine.h"
#include "Math.h"

class Player;
class MapChipField;

class Enemy2 {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, MapChipField* mapChipField);

	void Update();
	void Draw();

	AABB GetAABB();
	KamataEngine::Vector3 GetWorldPosition();

	void OnCollision(const Player* player);

	bool IsDead() const { return isDead_; }
	bool IsDefeated() const;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	static inline const float kMoveSpeed = 0.02f;
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	KamataEngine::Vector3 velocity_ = {};

	enum class Behavior { kNormal, kDefeated };
	Behavior behavior_ = Behavior::kNormal;

	bool isDead_ = false;
	bool isCollisionDisabled_ = false;
	int deathTimer_ = 0;
};
