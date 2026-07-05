#define NOMINMAX
#include "Player.h"
#include "Enemy.h"
#include "imgui.h"
#include <cassert>
using namespace KamataEngine;

//==================================================
// コンストラクタ・デストラクタ
//==================================================
Player::Player() {}
Player::~Player() {}

//==================================================
// 初期化処理
//==================================================
void Player::Initialize(KamataEngine::Model* model, KamataEngine::Model* modelRollling, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	assert(model);

	model_ = model;

	camera_ = camera;

	modelRollling_ = modelRollling;

	hp_ = kMaxHp;
	isInvincible_ = false;
	invincibleTimer_ = 0.0f;
	isDead_ = false;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期向き：右向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	dustModel_ = Model::CreateFromOBJ("particle", true);

	soundHandleDeath = Audio::GetInstance()->LoadWave("desu.wav");
	soundHandleSpring = Audio::GetInstance()->LoadWave("jumpbane.wav");
	soundHandleChange = Audio::GetInstance()->LoadWave("changeMode.wav");
	dust_.Initialize(dustModel_, camera_);

	// ===== 新規演出初期化 =====
	landDust_.Initialize(dustModel_, camera_);
	spark_.Initialize(dustModel_, camera_);

	trailColors_.clear();
	for (int i = 0; i < kMaxTrails; i++) {
		auto color = std::make_unique<ObjectColor>();
		color->Initialize();
		trailColors_.push_back(std::move(color));
	}
	for (auto& transform : trailTransforms_) {
		transform.Initialize();
	}
	trails_.clear();
	trailEmitTimer_ = 0.0f;
	sparkEmitTimer_ = 0.0f;
}

#pragma region 入力処理ジャンプ関数化
//==================================================
// 入力処理（移動・ジャンプ）
//==================================================
void Player::InputMove() {
	// 自動で右へ走り続ける
	velocity_.x = kLimitRunSpeed;

	// ===== ジャンプ処理（二段ジャンプ対応、上矢印キー） =====
	if (Input::GetInstance()->TriggerKey(DIK_UP)) {
		if (onGround_ || jumpCount_ < kMaxJumpCount) {
			velocity_.y = kJumpInitialVelocity; // 初速を与える
			onGround_ = false;
			jumpCount_++;
		}
	}

	//  ジャンプ中の長押し補助（上昇中だけ、上矢印キー）
	if (Input::GetInstance()->PushKey(DIK_UP) && velocity_.y > 0) {
		velocity_.y += kJumpBoost / 60.0f;
	}

	//  重力を加える
	velocity_.y -= kGravityAcceleration / 60.0f;
	velocity_.y = std::max(velocity_.y, -kLimitFallSpeed); // 落下速度制限

	// ===== 地面に着いたらジャンプ回数リセット =====
	if (onGround_) {
		jumpCount_ = 0;
	}
}

#pragma endregion

//==================================================
// 四隅の座標を返す
//==================================================
Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, // 右下
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, // 左下
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, // 右上
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}  // 左上
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

#pragma region プレイヤーの当たり判定まとめ
//==================================================
// マップ衝突判定の統合呼び出し
//==================================================
void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}
#pragma endregion

#pragma region 天井
//--------------------------------------------------
// 天井との衝突判定
//--------------------------------------------------
void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	if (info.move.y <= 0)
		return; // 上昇してなければ不要

	// 四隅の座標
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	bool hit = false;
	MapChipType mapChipType, mapChipTypeNext;
	MapChipField::IndexSet indexSet;

	// 左上
	indexSet = mapChipField_->GetMapChipIndexSetByposition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock)
		hit = true;

	// 右上
	indexSet = mapChipField_->GetMapChipIndexSetByposition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock)
		hit = true;

	if (hit) {
		Vector3 playerTopPos = worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0);
		indexSet = mapChipField_->GetMapChipIndexSetByposition(playerTopPos);
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

		info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
		info.isHitCeiling = true;
	}
}
#pragma endregion

#pragma region 床との衝突判定
//--------------------------------------------------
// 床との衝突判定
//--------------------------------------------------
void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	if (info.move.y >= 0)
		return; // 下降してなければ不要

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	bool hit = false;
	float highestFloorTop = -1000.0f;
	MapChipField::IndexSet bestIndexSet = {};

	// 左下と右下をチェック
	for (Corner corner : {kLeftBottom, kRightBottom}) {
		auto indexSetTemp = mapChipField_->GetMapChipIndexSetByposition(positionsNew[corner]);

		// そのブロックの天面高さをチェック
		Vector3 blockPos = mapChipField_->GetMapChipPositionByIndex(indexSetTemp.xIndex, indexSetTemp.yIndex);
		float blockTopY = blockPos.y + MapChipField::kBlockHeight / 2.0f;
		float playerBottomY = worldTransform_.translation_.y - kHeight / 2.0f;

		// ブロックの天面が、プレイヤーの足元よりも明らかに高いなら、それは床ではなく「壁」なので無視する
		if (blockTopY > playerBottomY + 0.2f) {
			continue;
		}

		MapChipType mapChip = mapChipField_->GetMapChipTypeByIndex(indexSetTemp.xIndex, indexSetTemp.yIndex);

		// トゲならダメージ（無敵中はスルー）
		if (mapChip == MapChipType::kSpike) {
			if (!isInvincible_) {
				ApplyDamage(30.0f);
			}
			return;
		}

		// ブロック判定
		if (mapChip == MapChipType::kBlock) {
			hit = true;

			// 衝突したブロックの中から、最も高い床を探す
			int xIndex = indexSetTemp.xIndex;
			int yIndex = indexSetTemp.yIndex;

			while (yIndex < 20) {
				Vector3 tempPos = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
				float tempTopY = tempPos.y + MapChipField::kBlockHeight / 2.0f;
				MapChipType type = mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex);

				if (type == MapChipType::kBlock && tempTopY <= playerBottomY + 0.2f) {
					if (tempTopY > highestFloorTop) {
						highestFloorTop = tempTopY;
						bestIndexSet = { (uint32_t)xIndex, (uint32_t)yIndex };
					}
					break;
				}
				yIndex++;
			}
		}
	}

	if (hit && highestFloorTop > -900.0f) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(bestIndexSet.xIndex, bestIndexSet.yIndex);
		info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
		info.isHitLanding = true;
	}
}
#pragma endregion

#pragma region 右側の衝突判定
//--------------------------------------------------
// 右側との衝突判定
//--------------------------------------------------
void Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	if (info.move.x <= 0)
		return;

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	bool hit = false;
	MapChipType mapChip = MapChipType::kBlank;
	for (Corner corner : {kRightTop, kRightBottom}) {
		auto index = mapChipField_->GetMapChipIndexSetByposition(positionsNew[corner]);
		mapChip = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		// ===== ブロックなら通常の壁衝突 =====
		if (mapChip == MapChipType::kBlock) {
			if (!isInvincible_) {
				ApplyDamage(20.0f); // 壁激突ダメージ
			}

			if (isInvincible_) {
				// 無敵中ならすり抜ける（当たりにしない）
			} else {
				hit = true;
				break;
			}
		}

		// ===== トゲならダメージ =====
		if (mapChip == MapChipType::kSpike) {
			if (!isInvincible_) {
				ApplyDamage(30.0f);
			}
			return;
		}
	}

	if (hit) {
		Vector3 playerRightPosNext = worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f, 0, 0);
		auto index = mapChipField_->GetMapChipIndexSetByposition(playerRightPosNext);
		auto rect = mapChipField_->GetRectByIndex(index.xIndex, index.yIndex);

		info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
		info.isHitWall = true;
	}
}

#pragma endregion

#pragma region 左側の衝突判定
//--------------------------------------------------
// 左側との衝突判定
//--------------------------------------------------

//--------------------------------------------------
// 左側との衝突判定
//--------------------------------------------------
void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {
	if (info.move.x >= 0)
		return;

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	bool hit = false;
	for (Corner corner : {kLeftTop, kLeftBottom}) {
		auto index = mapChipField_->GetMapChipIndexSetByposition(positionsNew[corner]);
		auto mapChip = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		// ===== ブロックなら衝突扱い =====
		if (mapChip == MapChipType::kBlock) {
			if (!isInvincible_) {
				ApplyDamage(20.0f);
			}

			if (isInvincible_) {
				// 無敵中ならすり抜ける
			} else {
				hit = true;
			}
		}

		// ===== トゲならダメージ =====
		if (mapChip == MapChipType::kSpike) {
			if (!isInvincible_) {
				ApplyDamage(30.0f);
			}
			return;
		}
	}

	if (hit) {
		Vector3 leftPosAfterMove = worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f, 0, 0);
		auto index = mapChipField_->GetMapChipIndexSetByposition(leftPosAfterMove);
		auto rect = mapChipField_->GetRectByIndex(index.xIndex, index.yIndex);

		float overlap = leftPosAfterMove.x - rect.right;
		if (overlap < 0.0f) {
			info.move.x -= overlap + kBlank; // 微小な隙間を空ける
		}
		info.isHitWall = true;
	}
}

#pragma endregion

#pragma region 接地状態の更新
//==================================================
// 接地状態の更新
//==================================================
void Player::UpdateOnGround(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false; // ジャンプ開始
		} else {
			// 落下判定
			std::array<Vector3, kNumCorner> positionNew;
			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
			}

			bool hit = false;
			for (Corner corner : {kLeftBottom, kRightBottom}) {
				auto index = mapChipField_->GetMapChipIndexSetByposition(positionNew[corner] + Vector3(0, -kGroundSearchHeight, 0));
				auto mapChip = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);
				if (mapChip == MapChipType::kBlock)
					hit = true;
			}
			if (!hit)
				onGround_ = false; // 落下開始
		}
	} else {
		if (info.isHitLanding) {
			onGround_ = true; // 着地
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;

			// 着地煙を発生させる
			Vector3 footPos = worldTransform_.translation_ + Vector3(0.0f, -kHeight / 2.0f, 0.0f);
			landDust_.EmitLand(footPos);
		}
	}
}
#pragma endregion

#pragma region 壁衝突時
//==================================================
// 壁衝突時の処理
// 壁にぶつかったら横方向の速度をリセットする
//==================================================
void Player::UpdateOnWall(const CollisionMapInfo& info) {
	if (info.isHitWall) {
		// 無敵中なら壁すり抜けが有効なので速度リセットしない
		if (!isInvincible_) {
			velocity_.x = 0.0f;
		}
	}
}
#pragma endregion

//==================================================
// 更新処理
//==================================================
void Player::Update() {
	//==================================================
	// 無敵タイマーの更新と安全措置
	//==================================================
	if (isInvincible_) {
		invincibleTimer_ -= 1.0f / 60.0f;
		if (invincibleTimer_ <= 0.0f) {
			// 安全措置：壁の中にめり込んでいる場合は、壁の外に脱出するまで無敵終了を保留
			bool inBlock = false;
			auto index = mapChipField_->GetMapChipIndexSetByposition(worldTransform_.translation_);
			if (mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex) == MapChipType::kBlock) {
				inBlock = true;
			}

			if (!inBlock) {
				isInvincible_ = false;
			} else {
				invincibleTimer_ = 0.1f; // 0.1秒維持して次のフレームで再確認
			}
		}
	}

	//==================================================
	// 状態ごとの更新
	//==================================================
	InputMove();   // 移動・ジャンプ入力
	UpdateState(); // 状態遷移チェック（Normal⇔Rolling）

	//==================================================
	// パーティクル（砂ぼこり）
	//==================================================
	if (onGround_ && fabs(velocity_.x) > 0.1f) {
		dustEmitTimer_ += 1.0f / 60.0f; // 経過時間を加算
		if (dustEmitTimer_ >= 0.1f) {   // 0.1秒ごとに発生
			Vector3 footPos = worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0);
			dust_.Emit(footPos);
			dustEmitTimer_ = 0.0f;
		}
	} else {
		dustEmitTimer_ = 0.0f; // 停止時はリセット
	}
	dust_.Update();

	// ===== 着地煙の更新 =====
	landDust_.Update();

	// ===== スライディング摩擦火花の発生と更新 =====
	if (state_ == PlayerState::Rolling && onGround_ && fabs(velocity_.x) > 0.1f) {
		sparkEmitTimer_ += 1.0f / 60.0f;
		if (sparkEmitTimer_ >= 0.05f) { // 0.05秒ごとに発生
			Vector3 footPos = worldTransform_.translation_ + Vector3(0.0f, -kHeight / 2.0f, 0.0f);
			spark_.Emit(footPos);
			sparkEmitTimer_ = 0.0f;
		}
	} else {
		sparkEmitTimer_ = 0.0f;
	}
	spark_.Update();

	// ===== 残像の記録 =====
	bool shouldEmitTrail = false;
	if (state_ == PlayerState::Rolling) {
		shouldEmitTrail = true;
	} else if (!onGround_ && velocity_.y > 0.05f) {
		shouldEmitTrail = true;
	}

	trailEmitTimer_ += 1.0f / 60.0f;
	if (shouldEmitTrail && trailEmitTimer_ >= 0.06f) { // 約4フレームごと
		Trail newTrail;
		newTrail.translation = worldTransform_.translation_;
		newTrail.rotation = worldTransform_.rotation_;
		newTrail.scale = worldTransform_.scale_;

		newTrail.state = state_;
		newTrail.alpha = 0.6f; // 初期アルファ

		trails_.push_front(newTrail);
		if (trails_.size() > kMaxTrails) {
			trails_.pop_back();
		}
		trailEmitTimer_ = 0.0f;
	}

	// 残像のフェードアウトと削除
	for (auto it = trails_.begin(); it != trails_.end(); ) {
		it->alpha -= 0.03f; // 更新速度に合わせて減衰
		if (it->alpha <= 0.0f) {
			it = trails_.erase(it);
		} else {
			++it;
		}
	}

	//==================================================
	// 移動・衝突判定
	//==================================================
	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.move = velocity_;

	CheckMapCollision(collisionMapInfo);                   // マップ衝突判定
	worldTransform_.translation_ += collisionMapInfo.move; // 移動反映

	if (collisionMapInfo.isHitCeiling) {
		velocity_.y = 0; // 天井に当たったらY速度リセット
	}

	UpdateOnGround(collisionMapInfo); // 接地判定
	UpdateOnWall(collisionMapInfo);   // 壁判定

	//==================================================
	// 旋回処理
	//==================================================
	if (turnTimer_ > 0.0f) {
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);

		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, 1.0f - turnTimer_ / kTimeTrun);
	}
	//==================================================
	// 穴落下死亡判定
	//==================================================
	if (worldTransform_.translation_.y < -5.0f) { //
		uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleDeath);
		Audio::GetInstance()->SetVolume(handle, 0.1f);
		isDead_ = true;
	}
	//==================================================
	// 行列更新
	//==================================================
	WorldTransformUpdate(worldTransform_);
}

//==================================================
// 描画処理
//==================================================
void Player::Draw() {
	// 無敵時間中の点滅（0.06秒ごとに表示/非表示をトグル）
	if (isInvincible_) {
		if (static_cast<int>(invincibleTimer_ * 15.0f) % 2 == 0) {
			return; // 描画をスキップ
		}
	}

	if (state_ == PlayerState::Rolling) {
		modelRollling_->Draw(worldTransform_, *camera_);
	} else {
		model_->Draw(worldTransform_, *camera_);
	}

	// ===== 残像の描画 =====
	int colorIndex = 0;
	for (auto& trail : trails_) {
		if (colorIndex >= trailColors_.size())
			break;

		// 事前確保された WorldTransform にパラメータをコピーして更新
		auto& wt = trailTransforms_[colorIndex];
		wt.translation_ = trail.translation;
		wt.rotation_ = trail.rotation;
		wt.scale_ = trail.scale;
		WorldTransformUpdate(wt);

		// シアン調の半透明カラーを設定
		Vector4 trailColor = {0.3f, 0.7f, 1.0f, trail.alpha};
		trailColors_[colorIndex]->SetColor(trailColor);

		if (trail.state == PlayerState::Rolling) {
			modelRollling_->Draw(wt, *camera_, trailColors_[colorIndex].get());
		} else {
			model_->Draw(wt, *camera_, trailColors_[colorIndex].get());
		}
		colorIndex++;
	}

	// 各パーティクルの描画
	dust_.Draw();
	landDust_.Draw();
	spark_.Draw();
}

//==================================================
// ユーティリティ
//==================================================

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	return {
	    {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
        {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
    };
}

void Player::OnCollision(const Enemy* enemy) {
	// --- 敵がすでに倒れているなら何もしない ---
	if (enemy->IsDefeated()) {
		return;
	}

	// --- 接触したら死亡 ---
	isDead_ = true;
	uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleDeath);
	Audio::GetInstance()->SetVolume(handle, 0.1f);
}

void Player::OnCollision(const Enemy2* enemy) {
	// --- 敵がすでに倒れているなら何もしない ---
	if (enemy->IsDefeated()) {
		return;
	}

	// --- 接触したら死亡 ---
	isDead_ = true;
	uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleDeath);
	Audio::GetInstance()->SetVolume(handle, 0.1f);
}

KamataEngine::Vector3 Player::GetWorldPosition() {
	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

void Player::UpdateState() {
	// --- 下矢印キー（DIK_DOWN）によるホールド式しゃがみ ---
	if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		if (state_ == PlayerState::Normal) {
			// しゃがむ（通常 → ローリング）
			uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleChange);
			Audio::GetInstance()->SetVolume(handle, 0.1f);
			state_ = PlayerState::Rolling;

			// 【足元維持オフセット】：立ち（高さ1.5f, 半分0.75f）からしゃがみ（高さ0.8f, 半分0.4f）に変わるため、
			// 中心Y座標を 0.35f 下げて足元の位置を完全にキープする
			worldTransform_.translation_.y -= 0.35f;
		}
	} else {
		if (state_ == PlayerState::Rolling) {
			// 立ち上がる（ローリング → 通常）前に、立ち上がった後の頭上（Y + 0.35f + 0.75f = Y + 1.1f）にブロックがないかチェック
			Vector3 topPos = worldTransform_.translation_ + Vector3(0, 1.1f, 0);

			auto index = mapChipField_->GetMapChipIndexSetByposition(topPos);
			auto mapChip = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

			if (mapChip != MapChipType::kBlock) {
				uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleChange);
				Audio::GetInstance()->SetVolume(handle, 0.1f);
				state_ = PlayerState::Normal; // 立ち上がる

				// 【足元維持オフセット】：しゃがみから立ちに戻るため、
				// 中心Y座標を 0.35f 上げて足元の位置を完全にキープする
				worldTransform_.translation_.y += 0.35f;
			}
		}
	}

	// --- 高さを設定 ---
	if (state_ == PlayerState::Rolling) {
		kHeight = 0.8f;
	} else {
		kHeight = 1.5f;
	}

	// --- 下の判定（地面に埋まらないように補正） ---
	if (state_ == PlayerState::Normal) {
		float highestFloorTop = -1000.0f;
		bool foundFloor = false;
		MapChipField::IndexSet bestIndexSet = {};

		// 左右の足元の座標をチェック
		for (float offsetX : { -kWidth / 2.0f, +kWidth / 2.0f }) {
			Vector3 checkPos = worldTransform_.translation_ + Vector3(offsetX, -kHeight / 2.0f, 0);
			auto index = mapChipField_->GetMapChipIndexSetByposition(checkPos);

			int xIndex = index.xIndex;
			int yIndex = index.yIndex;
			float playerBottomY = worldTransform_.translation_.y - kHeight / 2.0f;

			// 足元より明らかに高い（0.2f超）ブロックは「床」ではなく「壁」なので無視し、下を探す
			while (yIndex < 20) {
				Vector3 tempPos = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
				float tempTopY = tempPos.y + MapChipField::kBlockHeight / 2.0f;
				MapChipType type = mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex);

				if (type == MapChipType::kBlock && tempTopY <= playerBottomY + 0.2f) {
					if (tempTopY > highestFloorTop) {
						highestFloorTop = tempTopY;
						bestIndexSet = { (uint32_t)xIndex, (uint32_t)yIndex };
						foundFloor = true;
					}
					break;
				}
				yIndex++;
			}
		}

		if (foundFloor) {
			auto rect = mapChipField_->GetRectByIndex(bestIndexSet.xIndex, bestIndexSet.yIndex);
			float targetY = rect.top + kHeight / 2.0f + kBlank;
			float diffY = targetY - worldTransform_.translation_.y;
			// わずかな埋まり（0.4f未満）のときのみ補正する（壁の天面へのワープを防ぐ）
			if (diffY > 0.0f && diffY < 0.4f) {
				worldTransform_.translation_.y = targetY;
			}
		}
	}

	// --- 上の判定（天井にぶつかったら補正） ---
	if (state_ == PlayerState::Normal) {
		Vector3 topPos = worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0);
		auto index = mapChipField_->GetMapChipIndexSetByposition(topPos);
		auto mapChip = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		if (mapChip == MapChipType::kBlock) {
			auto rect = mapChipField_->GetRectByIndex(index.xIndex, index.yIndex);
			float targetY = rect.bottom - kHeight / 2.0f - kBlank;
			float diffY = worldTransform_.translation_.y - targetY;
			// わずかなめり込み（0.4f未満）のときのみ補正する
			if (diffY > 0.0f && diffY < 0.4f) {
				worldTransform_.translation_.y = targetY;
				velocity_.y = 0.0f; // 天井にぶつかったのでリセット
			}
		}
	}
}


void Player::CheckSpringCollision(const std::vector<std::vector<WorldTransform*>>& springs) {
	// すでに上昇中ならバネ判定をスキップし、衝突SEの重複（チャララランと連続で鳴る現象）を防ぐ
	if (velocity_.y > 0.0f)
		return;

	AABB playerAABB = GetAABB();
	float playerX = worldTransform_.translation_.x;

	for (auto& line : springs) {
		for (auto& spring : line) {
			if (!spring)
				continue;

			// 画面外なら判定をスキップ
			if (std::abs(playerX - spring->translation_.x) > 13.0f)
				continue;

			// バネの AABB を計算
			AABB springAABB = {
			    {spring->translation_.x - MapChipField::kBlockWidth / 2.0f, spring->translation_.y - MapChipField::kBlockHeight / 2.0f, -0.5f},
			    {spring->translation_.x + MapChipField::kBlockWidth / 2.0f, spring->translation_.y + MapChipField::kBlockHeight / 2.0f, +0.5f}
            };

			// 当たり判定
			if (IsCollision(playerAABB, springAABB)) {
				//  バネに触れたら強制大ジャンプ (4マスの高壁を確実に超えられるよう、通常ジャンプの2.2倍に強化)
				velocity_.y = kJumpInitialVelocity * 2.2f; 
				onGround_ = false;
				uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleSpring);
				Audio::GetInstance()->SetVolume(handle, 0.1f);
				return; // 1つヒットしたら終わり
			}
		}
	}
}
void Player::CheckFireCollision(const std::vector<std::vector<WorldTransform*>>& fires) {
	if (isInvincible_)
		return; // 無敵中なら判定をスキップ

	AABB playerAABB = GetAABB();
	float playerX = worldTransform_.translation_.x;

	for (auto& line : fires) {
		for (auto& fire : line) {
			if (!fire)
				continue;

			// 画面外なら判定をスキップ
			if (std::abs(playerX - fire->translation_.x) > 13.0f)
				continue;

			// 火の AABB
			AABB fireAABB = {
			    {fire->translation_.x - MapChipField::kBlockWidth / 2.0f, fire->translation_.y - MapChipField::kBlockHeight / 2.0f, -0.5f},
			    {fire->translation_.x + MapChipField::kBlockWidth / 2.0f, fire->translation_.y + MapChipField::kBlockHeight / 2.0f, +0.5f}
            };

			if (IsCollision(playerAABB, fireAABB)) {
				// 火に触れたらダメージ
				ApplyDamage(30.0f);
				return;
			}
		}
	}
}

void Player::ApplyDamage(float damage) {
	if (isInvincible_ || isDead_)
		return;

	hp_ -= damage;
	if (hp_ <= 0.0f) {
		hp_ = 0.0f;
		isDead_ = true;
		uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleDeath);
		Audio::GetInstance()->SetVolume(handle, 0.1f);
	} else {
		// 被弾無敵の開始
		isInvincible_ = true;
		invincibleTimer_ = kInvincibleDuration;

		// 被弾時の変身SE
		uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleChange);
		Audio::GetInstance()->SetVolume(handle, 0.1f);
	}
}


