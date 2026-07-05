#include "CameraController.h"
#include "Player.h"

#include <algorithm> // std::max, std::min を使用するために必要

void CameraController::Initialize(KamataEngine::Camera* camera) { camera_ = camera; }

void CameraController::Update() {
	// プレイヤーの現在位置を取得
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// プレイヤーの現在速度を取得
	const KamataEngine::Vector3& targetvelocity = target_->GetVelocity();

	// カメラの目的地 = プレイヤー位置 + オフセット + (速度 × バイアス)
	destination_ = targetWorldTransform.translation_ + targetOffset_ + targetvelocity * kVelocityBias;

	// 補間でなめらかに目的地へ近づける（ゆったり追従）
	camera_->translation_ = Lerp(camera_->translation_, destination_, kinterpolationRate);

	// プレイヤーとのマージン制御（画面からはみ出さないように）
	camera_->translation_.x = std::max(camera_->translation_.x, destination_.x + targetMargin.left);
	camera_->translation_.x = std::min(camera_->translation_.x, destination_.x + targetMargin.right);
	camera_->translation_.y = std::max(camera_->translation_.y, destination_.y + targetMargin.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, destination_.y + targetMargin.top);

	// ステージ全体の移動範囲制限（外に出ないように）
	camera_->translation_.x = std::max(camera_->translation_.x, movableArea_.left);
	camera_->translation_.x = std::min(camera_->translation_.x, movableArea_.right);
	camera_->translation_.y = std::max(camera_->translation_.y, movableArea_.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, movableArea_.top);

	// 最終的に行列を更新
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
	// 追従対象の和０ルドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの位置を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}