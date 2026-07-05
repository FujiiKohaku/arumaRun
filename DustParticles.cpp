#include "DustParticles.h"
#include <algorithm>
#include <random>

using namespace KamataEngine;

// 初期化
void DustParticles::Initialize(Model* model, Camera* camera) {
	model_ = model;
	camera_ = camera;

	for (auto& wt : worldTransforms_) {
		wt.Initialize();
	}

	objectColor_.Initialize();
	color_ = {0.7f, 0.7f, 0.7f, 1.0f}; // 灰色っぽい色
	isFinished_ = true;
}

// 発生
void DustParticles::Emit(const Vector3& position) {
	counter_ = 0.0f;
	isFinished_ = false;

	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> speedDist(kMinSpeed_, kMaxSpeed_);

	for (uint32_t i = 0; i < kNumParticles; i++) {
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = position + Vector3(0, 0.05f, 0); // 少し浮かす
		worldTransforms_[i].scale_ = {0.25f, 0.25f, 0.25f};

		float angle = angleDist(rng);
		float speed = speedDist(rng);

		velocities_[i] = {
		    cosf(angle) * speed,
		    0.015f + (speed * 0.3f), // 上に舞い上がる成分
		    sinf(angle) * speed};
	}
}

// 更新
void DustParticles::Update() {
	if (isFinished_)
		return;

	counter_ += 1.0f / 60.0f;
	if (counter_ >= kDuration_) {
		counter_ = kDuration_;
		isFinished_ = true;
	}

	for (uint32_t i = 0; i < kNumParticles; i++) {
		// 移動
		worldTransforms_[i].translation_ += velocities_[i];

		// 減速 + 重力
		velocities_[i].x *= 0.94f;
		velocities_[i].z *= 0.94f;
		velocities_[i].y -= 0.001f;

		// スケールを少し広げる
		worldTransforms_[i].scale_ *= 1.01f;

		WorldTransformUpdate(worldTransforms_[i]);
	}

	// アルファフェード
	color_.w = std::clamp(1.0f - counter_ / kDuration_, 0.0f, 1.0f);
	objectColor_.SetColor(color_);
}

// 描画
void DustParticles::Draw() {
	if (isFinished_)
		return;

	for (auto& wt : worldTransforms_) {
		model_->Draw(wt, *camera_, &objectColor_);
	}
}
