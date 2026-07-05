#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include <array>
#include <numbers>

class DustParticles {
public:
    // 初期化
    void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

    // 発生させる
    void Emit(const KamataEngine::Vector3& position);

    // 更新
    void Update();

    // 描画
    void Draw();

    // 終了判定
    bool IsFinished() const { return isFinished_; }

private:
    static inline const uint32_t kNumParticles = 12; // パーティクルの数
    static inline const float kDuration_ = 0.6f;     // 寿命（秒）
    static inline const float kMinSpeed_ = 0.01f;    // 最小速度
    static inline const float kMaxSpeed_ = 0.05f;    // 最大速度

    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Camera* camera_ = nullptr;

    std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;
    std::array<KamataEngine::Vector3, kNumParticles> velocities_;

    float counter_ = 0.0f;
    bool isFinished_ = true;

    KamataEngine::ObjectColor objectColor_;
    KamataEngine::Vector4 color_;
};
