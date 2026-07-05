#pragma once
#include "KamataEngine.h"
class Skydome {
public:
	// 初期化
	void initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	// 更新
	void UpDate(const KamataEngine::Vector3& position);

	// 描画

	void Draw();

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr; // カメラへのポインタ
};
