#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include "MapChipField.h"

class FallingSpike {
public:
	enum class State {
		kWaiting,
		kShaking,
		kFalling,
		kLanded
	};

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, MapChipField* mapChipField);
	void Update(float playerX);
	void Draw();

	// 状態と座標
	State GetState() const { return state_; }
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	
	// 衝突用の AABB (落下中の被弾判定用)
	AABB GetAABB() const;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	State state_ = State::kWaiting;
	KamataEngine::Vector3 startPosition_;
	float targetY_ = 0.0f;
	float shakeTimer_ = 0.0f;
	float fallVelocity_ = 0.0f;
	uint32_t soundHandleCrash_ = 0;
	bool playedCrashSound_ = false;
};
