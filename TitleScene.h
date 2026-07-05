#pragma once
#include "Fade.h"
#include "KamataEngine.h"
using namespace KamataEngine;

//--------------------------------------------------
// タイトルシーン
// 02_12 スライド 19枚目
//--------------------------------------------------
class TitleScene {
public:
	~TitleScene();

	// ライフサイクル
	void Initialize();
	void Update();
	void Draw();

	// シーンが終了したか
	bool IsFinished() const { return finished_; }

	// シーンのフェーズ
	// 02_12 スライド 27枚目
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン
		kFadeOut, // フェードアウト
	};

private:
	//--------------------------------------------------
	// 定数
	//--------------------------------------------------
	static inline const float kTimeTitleMove = 2.0f;

	//--------------------------------------------------
	// カメラ・ワールド変換
	//--------------------------------------------------
	Camera camera_;
	WorldTransform worldTransformTitle_;     // タイトルロゴ
	WorldTransform worldTransformPlayer_;    // プレイヤー
	WorldTransform worldTransformSun_;       // 太陽
	WorldTransform worldTransformBack_;      // 背景
	WorldTransform worldTransformPushSpace_; // 「Push Space」

	//--------------------------------------------------
	// モデル・テクスチャ
	//--------------------------------------------------
	Model* modelPlayer_ = nullptr;
	Model* modelTitle_ = nullptr;
	Model* sun_ = nullptr;
	Model* backGround_ = nullptr;
	Model* pushSpace_ = nullptr;
	uint32_t BackGroundTexture_ = 0;

	//--------------------------------------------------
	// フェード・演出
	//--------------------------------------------------
	Fade* fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
	float counter_ = 0.0f; // 汎用タイマー
	float angle_ = 0.0f;   // 演出回転用

	//--------------------------------------------------
	// 状態管理
	//--------------------------------------------------
	bool finished_ = false;

	//--------------------------------------------------
	// サウンド
	//--------------------------------------------------
	uint32_t soundHandleSelecct = 0;
};
