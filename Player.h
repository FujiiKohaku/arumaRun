#pragma once
#include "DustParticles.h"
#include "SparkParticles.h"
#include "Enemy2.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Math.h"
#include <algorithm>
#include <array>
#include <numbers>
#include <list>
#include <memory>
// 前方宣言
class MapChipField;
class Enemy; // 02_10 21枚目

//==================================================
// プレイヤークラス
//==================================================
class Player {
public:
	// コンストラクタ / デストラクタ
	Player();
	~Player();

	// 初期化
	void Initialize(KamataEngine::Model* modelNormal, KamataEngine::Model* modelRolling, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	void CheckSpringCollision(const std::vector<std::vector<WorldTransform*>>& springs);
	// 火の当たり判定
	void CheckFireCollision(const std::vector<std::vector<WorldTransform*>>& fires);

	//==================================================
	// Getter / Setter
	//==================================================
	// ワールド変換を返す（描画などに利用）
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	// 現在の速度を返す
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	// マップチップフィールドを外部から設定
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// ワールド座標を取得（02_10 10枚目）
	Vector3 GetWorldPosition();

	// AABBを取得（02_10 13枚目）
	AABB GetAABB();

	// 敵との衝突応答（02_10 21枚目）

	void OnCollision(const Enemy* enemy);
	void OnCollision(const Enemy2* enemy2);
	Vector3 GetPosition() const { return worldTransform_.translation_; }
	//==================================================
	// 入力処理
	//==================================================
	// 移動入力処理（AL3_02_07 スライド10枚目）
	void InputMove();

	// キャラクター矩形の角を示す列挙型（02_07 スライド16枚目）
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上
		kNumCorner    // 要素数
	};
	struct CollisionMapInfo {
		bool isHitCeiling = false;
		bool isHitLanding = false;
		bool isHitWall = false;
		Vector3 move;
	};
	enum class PlayerState { Normal, Rolling };
	PlayerState state_ = PlayerState::Normal;
	bool IsDead() const { return isDead_; }

	// HPと無敵関連
	float hp_ = 100.0f;
	const float kMaxHp = 100.0f;
	bool isInvincible_ = false;
	float invincibleTimer_ = 0.0f;
	const float kInvincibleDuration = 1.5f;

	void ApplyDamage(float damage);
	float GetHp() const { return hp_; }
	float GetMaxHp() const { return kMaxHp; }
	bool IsInvincible() const { return isInvincible_; }

private:
	//==================================================
	// 内部関数
	//==================================================
	// マップとの衝突判定処理（AL3_02_07 p13）
	void CheckMapCollision(CollisionMapInfo& info);
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	// キャラクター矩形の角座標を算出（AL3_02_07 page17）
	Vector3 CornerPosition(const Vector3& center, Corner corner);

	// 接地状態の更新（02_08 page14）
	void UpdateOnGround(const CollisionMapInfo& info);

	// 壁衝突時の処理（02_08 page27）
	void UpdateOnWall(const CollisionMapInfo& info);

	void UpdateState();

	//==================================================
	// メンバ変数
	//==================================================
	// Transform / Rendering
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelRollling_ = nullptr;
	uint32_t textureHandle_ = 0u;
	KamataEngine::Camera* camera_ = nullptr;

	// 物理
	KamataEngine::Vector3 velocity_ = {};  // 現在の速度
	bool onGround_ = true;                 // 接地フラグ
	MapChipField* mapChipField_ = nullptr; // マップフィールド

	//==================================================
	// キャラ挙動用パラメータ
	//==================================================
	// 移動パラメータ
	static inline float kAccelaration_ = 0.01f; // 加速度
	static inline float kAttenuation = 0.05f;   // 減衰率
	static inline float kLimitRunSpeed = 0.2f;  // 最大走行速度

	// ジャンプ / 重力
	static inline const float kGravityAcceleration = 0.98f; // 重力加速度
	static inline const float kLimitFallSpeed = 0.5f;       // 最大落下速度
	static inline const float kJumpAcceleration = 20.0f;    // ジャンプ初速

	// プレイヤー
	float kWidth = 0.8f;                      // 横幅
	float kHeight = 1.5f;                     // 高さ
	static inline const float kBlank = 0.04f; // 判定の余白

	// 接地 / 壁パラメータ
	static inline const float kAttenuationLanding = 0.0f;  // 着地時減衰
	static inline const float kAttenuationWall = 0.2f;     // 壁接触時減衰
	static inline const float kGroundSearchHeight = 0.06f; // 微小高さ

	// 旋回関連
	enum class LRDorection { kRight, kLeft };
	LRDorection lrDirection_ = LRDorection::kRight;
	float turnFirstRotationY_ = 0.0f;           // 旋回開始角度
	float turnTimer_ = 0.0f;                    // 旋回タイマー
	static inline const float kTimeTrun = 0.3f; // 旋回にかける時間（秒）

	bool isDead_ = false;

	DustParticles dust_;

	Model* dustModel_;

	float dustEmitTimer_;
	const float kJumpInitialVelocity = 0.25f; // 最低ジャンプ高さ
	const float kJumpBoost = 0.35f;           // 長押し補助の強さ

	int jumpCount_ = 0;          // 今何回ジャンプしたか
	const int kMaxJumpCount = 2; // 最大ジャンプ回数（二段ジャンプなら2）

	uint32_t soundHandleDeath = 0;

	uint32_t soundHandleSpring = 0;

	uint32_t soundHandleChange = 0;

	// ===== 演出追加要素 =====
	// 着地煙
	DustParticles landDust_;

	// 摩擦火花
	SparkParticles spark_;
	float sparkEmitTimer_ = 0.0f;

	// 残像エフェクト
	static inline const int kMaxTrails = 5;
	struct Trail {
		KamataEngine::Vector3 translation;
		KamataEngine::Vector3 rotation;
		KamataEngine::Vector3 scale;
		PlayerState state;
		float alpha;
	};
	std::list<Trail> trails_;
	std::array<KamataEngine::WorldTransform, kMaxTrails> trailTransforms_;
	std::vector<std::unique_ptr<KamataEngine::ObjectColor>> trailColors_;
	float trailEmitTimer_ = 0.0f;
};
