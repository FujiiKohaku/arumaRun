#pragma once
#define NOMINMAX
#include "KamataEngine.h"
#include "Math.h"
#include "Player.h"
#include <Windows.h>

class Player; // 前方宣言

class CameraController {
public:
	// 初期化
	void Initialize(KamataEngine::Camera* camera);

	// 毎フレーム更新（追従処理）
	void Update();

	// ターゲット（プレイヤー）を登録
	void SetTarget(Player* target) { target_ = target; }

	// 位置をリセット（ターゲットに即追従）
	void Reset();

	// 矩形（カメラの移動できる範囲）
	struct Rect {
		float left = 0.0f;   // Xの最小
		float right = 1.0f;  // Xの最大
		float bottom = 0.0f; // Yの最小
		float top = 1.0f;    // Yの最大
	};

	// 移動可能範囲を設定
	void SetMovableArea(const Rect& area) { movableArea_ = area; }

private:
	// カメラ本体
	KamataEngine::Camera* camera_ = nullptr;

	// 追従対象（プレイヤー）
	Player* target_ = nullptr;

	// プレイヤーからの相対位置（後ろに15.0f下がった位置がデフォルト）
	KamataEngine::Vector3 targetOffset_ = {0.0f, 3.0f, -15.0f};

	// 移動可能範囲（デフォルト: 0〜100）
	// → 実際は GameScene 側で設定するので初期値は保険
	Rect movableArea_ = {0, 100, 0, 100};

	// 目的地（補間でゆったり追従するために使う）
	KamataEngine::Vector3 destination_;

	// 補間率（0.1f = 毎フレーム10%だけ目的地に近づく）
	static inline const float kinterpolationRate = 0.1f;

	// 速度のバイアス（プレイヤーの進行方向に先行してカメラを動かす量）
	static inline const float kVelocityBias = 20.0f;

	// プレイヤーを画面内に収めるためのマージン
	// → 左右: ±9.0f, 上下: ±5.0f
	static inline const Rect targetMargin = {-9.0f, 9.0f, -5.0f, 5.0f};
};
