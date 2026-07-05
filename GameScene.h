#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Enemy2.h"
#include "Fade.h"
#include "HitEffect.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Math.h"
#include "Player.h"
#include "Skydome.h"
#include "FallingSpike.h"
#include "RisingSpike.h"
#include <list>
#include <vector>

//--------------------------------------------------
// ゲームシーン
//--------------------------------------------------
class GameScene {
public:
	~GameScene();

	// ===== ライフサイクル =====
	void Initialize(); // 初期化
	void Update();     // 更新
	void Draw();       // 描画

	// ===== 補助処理 =====
	void GenerateBlocks();     // マップチップからブロック生成
	void UpdateDynamicBlocks(); // 動的にブロックを生成・破棄する
	void CheckAllCollisions(); // 全衝突判定

	// ===== 状態確認 =====
	bool IsFinished() const { return finished_; }
	bool IsClear() const { return isClear_; }
	bool IsReturnToTitle() const { return returnToTitle_; }

private:
	//--------------------------------------------------
	// リソース（モデル / テクスチャ）
	//--------------------------------------------------
	// プレイヤー・敵・背景
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelRolling = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Model* skydomeModel_ = nullptr;
	KamataEngine::Model* enemyModel_ = nullptr;
	KamataEngine::Model* enemyModel2_ = nullptr;
	KamataEngine::Model* dethParticleModel = nullptr;

	// オブジェクト
	Model* spikeModel_ = nullptr;
	Model* springModel_ = nullptr;
	Model* fireModel_ = nullptr;
	KamataEngine::Model* goalModel_ = nullptr;

	// READY / GO!! 演出モデル
	KamataEngine::Model* readyModel_ = nullptr;
	KamataEngine::Model* goModel_ = nullptr;

	// テクスチャ
	uint32_t textureHandle_ = 0;
	uint32_t textureHandleMove_ = 0;
	uint32_t textureHandleJump_ = 0;
	uint32_t textureHandleAttack = 0;
	uint32_t textureHandleDown = 0;
	uint32_t textureHandlePose_ = 0;
	uint32_t soundhandleGo_ = 0;
	//--------------------------------------------------
	// 変換行列 / カメラ
	//--------------------------------------------------
	KamataEngine::WorldTransform worldtransform_;        // 汎用ワールド変換
	KamataEngine::WorldTransform worldTransformReady_;   // Ready用
	KamataEngine::WorldTransform worldTransformGo_;      // Go用
	KamataEngine::WorldTransform worldTransformSkydome_; // スカイドーム用

	KamataEngine::Camera* camera_ = nullptr;           // メインカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr; // デバッグカメラ

	//--------------------------------------------------
	// ゲームオブジェクト
	//--------------------------------------------------
	Player* player_ = nullptr;
	Skydome* skydome_ = nullptr;

	std::list<Enemy*> enemies_;   // 敵1リスト
	std::list<Enemy2*> enemies2_; // 敵2リスト

	// マップチップ由来の配置オブジェクト
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;  // ブロック
	std::vector<std::vector<WorldTransform*>> worldTransformSpikes_;  // スパイク
	std::vector<std::vector<WorldTransform*>> worldTransformSprings_; // バネ
	std::vector<std::vector<WorldTransform*>> worldTransformFires_;   // 火

	WorldTransform* worldTransformGoal_ = nullptr; // ゴール

	//--------------------------------------------------
	// マップ / カメラ
	//--------------------------------------------------
	MapChipField* mapChipField_ = nullptr;
	CameraController* cController_ = nullptr;
	bool isDebugCameraActive_ = false; // デバッグカメラON/OFF

	//--------------------------------------------------
	// 演出系
	//--------------------------------------------------
	DeathParticles* deathParticles_ = nullptr; // デス演出
	std::list<HitEffect*> hitEffects_;         // ヒットエフェクト

	// READY-GO!! 管理
	float readyTimer_ = 0.0f;

	// フェーズ管理
	enum class Phase {
		kFadeIn,
		kReady,   // READY-GO!! 演出
		kPlay,    // プレイ中
		kDeath,   // 死亡演出
		kFadeOut, // シーン終了
		kPause,
	};
	Phase phase_ = Phase::kFadeIn;
	void ChangePhese();

	// フェード
	Fade* fade_ = nullptr;

	//--------------------------------------------------
	// 状態フラグ
	//--------------------------------------------------
	bool finished_ = false; // シーン終了フラグ
	bool isClear_ = false;  // ゴールクリアフラグ
	bool returnToTitle_ = false; 
	//--------------------------------------------------
	// UI（2Dスプライト）
	//--------------------------------------------------
	KamataEngine::Sprite* spriteMove_ = nullptr;

	KamataEngine::Sprite* spriteJump_ = nullptr;

	KamataEngine::Sprite* spriteAttack = nullptr;

	KamataEngine::Sprite* spriteDown = nullptr;

	KamataEngine::Sprite* spritePose_ = nullptr;
	bool playedReadySound_ = false;

	// ===== コイン追加要素 =====
	std::vector<std::vector<WorldTransform*>> worldTransformCoins_; // コインの座標
	uint32_t soundHandleCoin_ = 0; // コイン回収SE
	int coinCount_ = 0;            // 獲得コイン数

	// ===== 落下プレス機追加要素 =====
	std::list<FallingSpike*> fallingSpikes_; // 落下プレス機のリスト

	// ===== 飛び出すトゲ追加要素 =====
	std::list<RisingSpike*> risingSpikes_; // 飛び出すトゲのリスト
};
