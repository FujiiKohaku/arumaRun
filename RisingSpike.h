#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include "MapChipField.h"

class RisingSpike {
public:
	enum class State {
		kHidden,
		kRising,
		kActive
	};

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, MapChipField* mapChipField);
	void Update(float playerX);
	void Draw();

	// 状態と座標
	State GetState() const { return state_; }
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	
	// 衝突用の AABB (伸長段階に合わせて高さが変化)
	AABB GetAABB() const;
	
	// 被弾判定が有効かどうか (ある程度せり出したら有効にする)
	bool IsDamageActive() const { return riseProgress_ >= 0.3f; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	State state_ = State::kHidden;
	KamataEngine::Vector3 startPosition_; // 飛び出した後の通常中心位置 (Y=17)
	float baseY_ = 0.0f;
	float riseProgress_ = 0.0f;
	uint32_t soundHandleRise_ = 0;
	bool playedSound_ = false;
};
