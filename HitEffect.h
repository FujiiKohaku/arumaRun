#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include <array>

//==================================================
// シンプルなヒットエフェクト（破片がはじけ散る）
//==================================================
class HitEffect {
public:
	// 初期化
	void Initialize(const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// インスタンス生成と初期化
	static HitEffect* Create(const KamataEngine::Vector3& position);

	// モデル・カメラを共有設定
	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	// 終了判定
	bool IsFinished() const { return finished_; }

private:
	// ==== 静的共有 ====
	static KamataEngine::Model* model_;
	static KamataEngine::Camera* camera_;

	// ==== エフェクト用 ====
	static const int kNumPieces = 8; // 破片の数
	struct Piece {
		KamataEngine::WorldTransform transform;
		KamataEngine::Vector3 velocity;
	};
	std::array<Piece, kNumPieces> pieces_;

	// 発生位置
	KamataEngine::Vector3 position_;

	// 寿命
	int lifeTimer_ = 0;
	int lifeTime_ = 30;
	bool finished_ = false;
};
