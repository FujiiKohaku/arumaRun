#include "GameScene.h"
using namespace KamataEngine;

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
GameScene::~GameScene() {
	// ====== オブジェクト解放 ======
	delete player_;
	delete skydome_;

	delete model_;
	delete modelBlock_;
	delete skydomeModel_;
	delete enemyModel_;
	delete dethParticleModel;

	delete debugCamera_;
	delete camera_;
	delete mapChipField_;
	delete cController_;

	delete deathParticles_;

	// Ready-Go モデル解放
	delete readyModel_;
	delete goModel_;

	// ====== 敵の解放 ======
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();
	// ====== 敵の解放 ======
	for (Enemy2* enemy : enemies2_) {
		delete enemy;
	}
	enemies_.clear();
	// ====== ヒットエフェクト解放 ======
	for (HitEffect* effect : hitEffects_) {
		delete effect;
	}
	hitEffects_.clear();

	// ====== ブロックの解放 ======
	for (auto& worldTransformBlockLine : worldTransformBlocks_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}

	delete spriteMove_;
	worldTransformBlocks_.clear();

	// ====== コインの解放 ======
	for (auto& worldTransformCoinLine : worldTransformCoins_) {
		for (auto& worldTransformCoin : worldTransformCoinLine) {
			delete worldTransformCoin;
		}
	}
	worldTransformCoins_.clear();

	// ====== 落下プレス機の解放 ======
	for (FallingSpike* fs : fallingSpikes_) {
		delete fs;
	}
	fallingSpikes_.clear();

	// ====== 飛び出すトゲの解放 ======
	for (RisingSpike* rs : risingSpikes_) {
		delete rs;
	}
	risingSpikes_.clear();
}

//==================================================
// 初期化
//==================================================
void GameScene::Initialize() {
	fallingSpikes_.clear();
	risingSpikes_.clear();

	// モデル生成
	model_ = Model::CreateFromOBJ("playermax", true);
	modelRolling = Model::CreateFromOBJ("roll", true);
	modelBlock_ = Model::CreateFromOBJ("tileBlock", true);
	skydomeModel_ = Model::CreateFromOBJ("skydome", true);
	enemyModel_ = Model::CreateFromOBJ("enemyBody", true);
	dethParticleModel = Model::CreateFromOBJ("deathParticle");
	spikeModel_ = Model::CreateFromOBJ("spike", true);
	// Ready/Go モデル
	readyModel_ = Model::CreateFromOBJ("rReady", true);
	goModel_ = Model::CreateFromOBJ("Go!", true);
	enemyModel2_ = Model::CreateFromOBJ("enemyBody", true);
	// カメラ
	camera_ = new Camera();
	camera_->Initialize();
	debugCamera_ = new DebugCamera(1080, 720);

	// ===== ヒットエフェクトにモデルとカメラを渡す =====
	HitEffect::SetModel(Model::CreateFromOBJ("AttackEffect", true));
	HitEffect::SetCamera(camera_);

	springModel_ = Model::CreateFromOBJ("spring", true);

	fireModel_ = Model::CreateFromOBJ("fire", true);
	goalModel_ = Model::CreateFromOBJ("doa", true);
	// Ready モデル
	worldTransformReady_.Initialize();
	worldTransformReady_.rotation_.x = std::numbers::pi_v<float> / 2.0f;
	worldTransformReady_.rotation_.y = std::numbers::pi_v<float>;
	worldTransformReady_.translation_.z = 30.0f; // カメラの正面
	worldTransformReady_.translation_.x = 2.5f;
	worldTransformReady_.translation_.y = 0.0f;
	worldTransformReady_.scale_ = {9.0f, 9.0f, 9.0f};

	// Go モデル
	worldTransformGo_.Initialize();
	worldTransformGo_.rotation_.x = std::numbers::pi_v<float> / 2.0f;
	worldTransformGo_.rotation_.y = std::numbers::pi_v<float>;
	worldTransformGo_.translation_.z = 30.0f;
	worldTransformGo_.translation_.x = 4.0f;
	worldTransformGo_.translation_.y = 0.0f;
	worldTransformGo_.scale_ = {9.0f, 9.0f, 9.0f};

	// ワールドトランスフォーム
	worldtransform_.Initialize();

	// テクスチャ
	textureHandle_ = TextureManager::Load("mario.jpg");

	// マップ
	mapChipField_ = new MapChipField();
	mapChipField_->ResetMapChipData();

	// プレイヤー生成
	Vector3 playerposition = mapChipField_->GetMapChipPositionByIndex(5, 10);
	player_ = new Player();
	player_->Initialize(model_, modelRolling, camera_, playerposition);
	player_->SetMapChipField(mapChipField_);

	// 初期動的生成
	UpdateDynamicBlocks();

	// スカイドーム生成
	skydome_ = new Skydome();
	skydome_->initialize(skydomeModel_, camera_);

	// カメラコントローラー
	cController_ = new CameraController();
	cController_->Initialize(camera_);
	cController_->SetTarget(player_);
	cController_->Reset();

	CameraController::Rect cameraArea = {12.0f, 1000000.0f, 6.0f, 6.0f};
	cController_->SetMovableArea(cameraArea);

	// デスパーティクル
	deathParticles_ = new DeathParticles;
	deathParticles_->Initialize(dethParticleModel, camera_, playerposition);

	// フェーズ初期化
	phase_ = Phase::kFadeIn;
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// Ready-Go タイマー
	readyTimer_ = 0.0f;

	textureHandleMove_ = TextureManager::Load("setumei.jpg");
	textureHandleJump_ = TextureManager::Load("junp.jpg");
	textureHandleAttack = TextureManager::Load("Attack.jpg");
	textureHandleDown = TextureManager::Load("down.jpg");
	textureHandlePose_ = TextureManager::Load("po-zu.jpg");
	spriteMove_ = KamataEngine::Sprite::Create(textureHandleMove_, {50, 50});
	spriteJump_ = KamataEngine::Sprite::Create(textureHandleJump_, {50, 150});
	spriteAttack = KamataEngine::Sprite::Create(textureHandleAttack, {50, 250});
	spriteDown = KamataEngine::Sprite::Create(textureHandleDown, {50, 350});
	spritePose_ = KamataEngine::Sprite::Create(textureHandlePose_, {90, 0});
	soundhandleGo_ = Audio::GetInstance()->LoadWave("GO.wav");
	soundHandleCoin_ = Audio::GetInstance()->LoadWave("serect.wav");
	coinCount_ = 0;
}

//==================================================
// 更新
//==================================================
//==================================================
// 更新
//==================================================
void GameScene::Update() {
	ChangePhese();
	WorldTransformUpdate(worldTransformReady_);
	WorldTransformUpdate(worldTransformGo_);

	switch (phase_) {
	case Phase::kFadeIn: {
		if (fade_) {
			fade_->Update();
			if (fade_->IsFinished()) {
				phase_ = Phase::kReady;
				readyTimer_ = 0.0f;
			}
		}
		skydome_->UpDate(player_->GetPosition());
		cController_->Update();
	} break;
	case Phase::kReady: {
		readyTimer_ += 1.0f / 60.0f;

		if (readyTimer_ < 1.0f) {
			float t = readyTimer_;
			float scale = 12.0f - 3.0f * t;
			worldTransformReady_.scale_ = {scale, scale, scale};

		} else {
			float t = (readyTimer_ - 1.0f) * 2.0f * 3.14159f;
			float scale = 9.0f + 1.0f * sinf(t * 4.0f);
			worldTransformGo_.scale_ = {scale, scale, scale};
			// 一度だけ鳴らす
			if (!playedReadySound_) {
				uint32_t handle = Audio::GetInstance()->PlayWave(soundhandleGo_);
				Audio::GetInstance()->SetVolume(handle, 0.1f);
				playedReadySound_ = true;
			}
		}

		if (readyTimer_ > 1.5f) {
			phase_ = Phase::kPlay;
		}
		skydome_->UpDate(player_->GetPosition());
		cController_->Update();
	} break;

	case Phase::kPlay: {
		UpdateDynamicBlocks(); // 動的にブロックを生成・破棄

		skydome_->UpDate(player_->GetPosition());
		cController_->Update();
		player_->Update();
		for (Enemy* enemy : enemies_) {
			enemy->UpDate();
		}
		for (Enemy2* enemy : enemies2_) {
			enemy->Update();
		}
		// ===== ESCキーでポーズに切り替え =====
		if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
			phase_ = Phase::kPause;
			break;
		}

		// ===== デバッグカメラ切り替え =====
#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_G)) {
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_->matView = debugCamera_->GetCamera().matView;
			camera_->matProjection = debugCamera_->GetCamera().matProjection;
			camera_->TransferMatrix();
		} else {
			camera_->UpdateMatrix();
		}

		// ===== ブロック更新 =====
		for (auto& line : worldTransformBlocks_) {
			for (auto& block : line) {
				if (!block)
					continue;
				WorldTransformUpdate(*block);
			}
		}

		// ===== スパイク更新 =====
		for (auto& line : worldTransformSpikes_) {
			for (auto& spike : line) {
				if (!spike)
					continue;
				WorldTransformUpdate(*spike);
			}
		}

		// ===== バネ更新 =====
		for (auto& line : worldTransformSprings_) {
			for (auto& spring : line) {
				if (!spring)
					continue;
				WorldTransformUpdate(*spring);
			}
		}

		// ===== コイン更新 =====
		for (auto& line : worldTransformCoins_) {
			for (auto& coin : line) {
				if (!coin)
					continue;
				coin->rotation_.y += 0.05f; // コルクル回転させる
				WorldTransformUpdate(*coin);
			}
		}
		// ===== 火の更新 =====
		static float t = 0.0f;
		t += 1.0f / 60.0f; // フレームごとに進める

		for (uint32_t i = 0; i < worldTransformFires_.size(); ++i) {
			for (uint32_t j = 0; j < worldTransformFires_[i].size(); ++j) {
				auto* fire = worldTransformFires_[i][j];
				if (!fire)
					continue;

				float amplitude = 0.4f; // 上下の振れ幅を小さく（1.0fから0.4fに変更）
				float speed = 2.0f;     // 上下の速さ

				// 座標蓄積バグを防ぐため、マップチップから元々の初期Y座標を取得
				float baseY = mapChipField_->GetMapChipPositionByIndex(j, i).y;
				fire->translation_.y = baseY + amplitude * sinf(t * speed);

				WorldTransformUpdate(*fire);
			}
		}

		if (worldTransformGoal_) {
			WorldTransformUpdate(*worldTransformGoal_);
		}

		// ===== 落下プレス機更新 =====
		float playerX = player_->GetPosition().x;
		for (FallingSpike* fs : fallingSpikes_) {
			fs->Update(playerX);
		}

		// ===== 飛び出すトゲ更新 =====
		for (RisingSpike* rs : risingSpikes_) {
			rs->Update(playerX);
		}

		player_->CheckSpringCollision(worldTransformSprings_);
		player_->CheckFireCollision(worldTransformFires_);
		CheckAllCollisions();

	} break;

	case Phase::kDeath: {
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			if (fade_)
				fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		skydome_->UpDate(player_->GetPosition());
		cController_->Update();
		for (Enemy* enemy : enemies_)
			enemy->UpDate();
		if (deathParticles_)
			deathParticles_->Update();
	} break;

	case Phase::kFadeOut: {
		if (fade_) {
			fade_->Update();
			if (fade_->IsFinished())
				finished_ = true;
		}
		skydome_->UpDate(player_->GetPosition());
		cController_->Update();
		for (Enemy* enemy : enemies_)
			enemy->UpDate();
	} break;
	case Phase::kPause:
		if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
			phase_ = Phase::kPlay;
		}

		// ===== Enterでタイトルに戻る =====
		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
			returnToTitle_ = true;
		}
		break;
	}

	// ====== フェーズ共通で敵削除処理 ======
	enemies_.remove_if([this](Enemy* enemy) {
		if (enemy->IsDead()) {
			// ヒットエフェクト生成
			hitEffects_.push_back(HitEffect::Create(enemy->GetWorldPosition()));
			delete enemy;
			return true;
		}
		return false;
	});

	// ====== ヒットエフェクト更新 ======
	for (auto it = hitEffects_.begin(); it != hitEffects_.end();) {
		(*it)->Update();
		if ((*it)->IsFinished()) {
			delete *it;
			it = hitEffects_.erase(it);
		} else {
			++it;
		}
	}
}

//==================================================
// 描画
//==================================================
void GameScene::Draw() {
	KamataEngine::DirectXCommon* dxcommon = KamataEngine::DirectXCommon::GetInstance();

	// ==============================
	// 3D描画開始
	// ==============================
	Model::PreDraw(dxcommon->GetCommandList());

	// --- プレイヤー ---
	if (!player_->IsDead()) {
		player_->Draw();
	}

	// --- 敵描画 ---
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}
	for (Enemy2* enemy2 : enemies2_) {
		enemy2->Draw();
	}

	// --- コイン描画 ---
	for (auto& line : worldTransformCoins_) {
		for (auto& coin : line) {
			if (!coin)
				continue;
			// modelRolling（丸い形状）を薄い黄色いコインに見立てて描画
			modelRolling->Draw(*coin, *camera_);
		}
	}

	// --- 落下プレス機描画 ---
	for (FallingSpike* fs : fallingSpikes_) {
		fs->Draw();
	}

	// --- 飛び出すトゲ描画 ---
	for (RisingSpike* rs : risingSpikes_) {
		rs->Draw();
	}

	// --- スカイドーム ---
	skydome_->Draw();

	// --- ブロック ---
	for (auto& line : worldTransformBlocks_) {
		for (auto& block : line) {
			if (!block)
				continue;
			modelBlock_->Draw(*block, *camera_, nullptr);
		}
	}

	// --- スパイク ---
	for (auto& line : worldTransformSpikes_) {
		for (auto& spike : line) {
			if (!spike)
				continue;
			spikeModel_->Draw(*spike, *camera_, nullptr);
		}
	}

	// --- バネ ---
	for (auto& line : worldTransformSprings_) {
		for (auto& spring : line) {
			if (!spring)
				continue;
			springModel_->Draw(*spring, *camera_, nullptr);
		}
	}

	// --- 動く火 ---
	for (auto& line : worldTransformFires_) {
		for (auto& fire : line) {
			if (!fire)
				continue;
			fireModel_->Draw(*fire, *camera_, nullptr);
		}
	}

	// --- ゴール ---
	if (worldTransformGoal_) {
		goalModel_->Draw(*worldTransformGoal_, *camera_);
	}

	// --- デスパーティクル ---
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	// --- ヒットエフェクト ---
	for (auto* effect : hitEffects_) {
		effect->Draw();
	}

	// --- Ready/Go 演出 ---
	if (phase_ == Phase::kReady) {
		if (readyTimer_ < 1.0f) {
			readyModel_->Draw(worldTransformReady_, *camera_);
		} else {
			goModel_->Draw(worldTransformGo_, *camera_);
		}
	}

	Model::PostDraw();
	// ==============================
	// 3D描画終了
	// ==============================

	// ==============================
	// 2D描画開始 (UI, フェード, スプライト)
	// ==============================
	Sprite::PreDraw(dxcommon->GetCommandList());


	if (phase_ == Phase::kPause) {

		spritePose_->Draw();
	}

	// --- HPテキストの描画 (DebugText) ---
	char hpString[64];
	snprintf(hpString, sizeof(hpString), "HP: %d / %d", static_cast<int>(player_->GetHp()), static_cast<int>(player_->GetMaxHp()));
	DebugText::GetInstance()->Print(hpString, 1000, 50, 2.0f);

	// --- 獲得コイン数の描画 (DebugText) ---
	char coinString[64];
	snprintf(coinString, sizeof(coinString), "Coins: %d", coinCount_);
	DebugText::GetInstance()->Print(coinString, 1000, 90, 2.0f);

	// デバッグテキスト描画
	DebugText::GetInstance()->DrawAll();

	Sprite::PostDraw();
	// ==============================
	// 2D描画終了
	// ==============================

	// --- フェードは最前面に描画 ---
	if (fade_) {
		fade_->Draw();
	}

#ifdef USE_IMGUI
	// --- HPバーの描画 ---
	ImGui::Begin("HP Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
	ImGui::SetWindowPos(ImVec2(10, 10));
	ImGui::SetWindowSize(ImVec2(400, 100));
	float hpPercent = player_->GetHp() / player_->GetMaxHp();

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "HP");
	ImGui::SameLine();

	ImVec4 barColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 緑
	if (hpPercent < 0.3f) {
		barColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // 赤
	} else if (hpPercent < 0.6f) {
		barColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // 黄
	}

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
	ImGui::ProgressBar(hpPercent, ImVec2(250, 25), "");
	ImGui::PopStyleColor();

	ImGui::SameLine();
	ImGui::Text("%d / %d", static_cast<int>(player_->GetHp()), static_cast<int>(player_->GetMaxHp()));
	ImGui::End();
#endif
}

//==================================================
// ブロック生成
//==================================================
void GameScene::GenerateBlocks() {
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	worldTransformSpikes_.resize(numBlockVirtical);

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
		worldTransformSpikes_[i].resize(numBlockHorizontal);
	}

	worldTransformFires_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformFires_[i].resize(numBlockHorizontal);
	}
	worldTransformSprings_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformSprings_[i].resize(numBlockHorizontal);
	}
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);

			if (type == MapChipType::kBlock) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = wt;

			} else if (type == MapChipType::kSpike) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);

				//  サイズを0.5倍
				wt->scale_ = {0.5f, 0.5f, 0.5f};

				//  左に90°回転（Y軸方向を想定）
				wt->rotation_.y = -std::numbers::pi_v<float> / 2.0f;

				worldTransformSpikes_[i][j] = wt;

			} else if (type == MapChipType::kEnemy) {
				Vector3 enemyPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				Enemy* newEnemy = new Enemy();
				newEnemy->Initialize(enemyModel_, camera_, enemyPos, mapChipField_);
				enemies_.push_back(newEnemy);
			}

			else if (type == MapChipType::kSpring) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->scale_ = {0.5f, 0.5f, 0.5f};
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				// 配列に追加
				worldTransformSprings_[i][j] = wt;
			} else if (type == MapChipType::kMovingFire) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);

				// 火の大きさは少し小さめに
				wt->scale_ = {0.5f, 0.5f, 0.5f};

				// 配列に追加（worldTransformFires_ を用意しておく）
				worldTransformFires_[i][j] = wt;
			} else if (type == MapChipType::kEnemy2) {
				Vector3 enemyPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				Enemy2* newEnemy2 = new Enemy2();
				newEnemy2->Initialize(enemyModel2_, camera_, enemyPos, mapChipField_);
				enemies2_.push_back(newEnemy2);
			} else if (type == MapChipType::kGoal) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				wt->scale_ = {1.0f, 1.0f, 1.0f};
				wt->rotation_.y = -std::numbers::pi_v<float> / 2.0f;
				worldTransformGoal_ = wt; // 1個だけなら単体で持つ
			}
		}
	}
}

//==================================================
// 衝突判定
//==================================================
void GameScene::CheckAllCollisions() {
	AABB aabb1 = player_->GetAABB();
	AABB aabb2;
	float playerX = player_->GetPosition().x;

	// プレイヤーが無敵中でない場合のみ敵との衝突をチェックする
	if (!player_->IsInvincible()) {
		// Enemy1 との当たり判定
		for (Enemy* enemy : enemies_) {
			// 画面外なら衝突判定をスキップ
			if (std::abs(playerX - enemy->GetWorldPosition().x) > 13.0f)
				continue;

			aabb2 = enemy->GetAABB();
			if (IsCollision(aabb1, aabb2)) {
				player_->ApplyDamage(25.0f); // 敵衝突で25ダメージ
			}
		}

		// Enemy2 との当たり判定
		for (Enemy2* enemy2 : enemies2_) {
			// 画面外なら衝突判定をスキップ
			if (std::abs(playerX - enemy2->GetWorldPosition().x) > 13.0f)
				continue;

			aabb2 = enemy2->GetAABB();
			if (IsCollision(aabb1, aabb2)) {
				player_->ApplyDamage(25.0f); // 敵衝突で25ダメージ
			}
		}
	}

	// ===== コインとの衝突判定 =====
	for (size_t i = 0; i < worldTransformCoins_.size(); ++i) {
		for (size_t j = 0; j < worldTransformCoins_[i].size(); ++j) {
			auto* coin = worldTransformCoins_[i][j];
			if (!coin)
				continue;

			// 画面外なら判定をスキップ
			if (std::abs(playerX - coin->translation_.x) > 13.0f)
				continue;

			// コインの AABB (スケールアップに合わせて当たり判定も拡大)
			AABB coinAABB = {
				{ coin->translation_.x - 0.4f, coin->translation_.y - 0.4f, -0.5f },
				{ coin->translation_.x + 0.4f, coin->translation_.y + 0.4f, 0.5f }
			};

			if (IsCollision(aabb1, coinAABB)) {
				// コイン獲得！
				coinCount_++;

				// SE再生 (音量 0.1)
				uint32_t handle = Audio::GetInstance()->PlayWave(soundHandleCoin_);
				Audio::GetInstance()->SetVolume(handle, 0.1f);

				// メモリ解放とマップデータ消去
				delete worldTransformCoins_[i][j];
				worldTransformCoins_[i][j] = nullptr;
				mapChipField_->SetMapChipType(static_cast<int>(j), static_cast<int>(i), MapChipType::kBlank);
			}
		}
	}

	// ===== 落下プレス機との当たり判定 =====
	for (FallingSpike* fs : fallingSpikes_) {
		// 落下中または揺れ中のものだけ、プレイヤーと直撃（潰される）判定を行う
		// (着地後はマップチップが Block に書き換わるため、通常の壁・床衝突システムで処理される)
		if (fs->GetState() == FallingSpike::State::kFalling || fs->GetState() == FallingSpike::State::kShaking) {
			AABB fsAABB = fs->GetAABB();
			if (IsCollision(aabb1, fsAABB)) {
				// 落下中のプレス機に直撃したら 30 ダメージ
				player_->ApplyDamage(30.0f);
			}
		}
	}

	// ===== 飛び出すトゲとの当たり判定 =====
	for (RisingSpike* rs : risingSpikes_) {
		if (rs->IsDamageActive()) {
			AABB rsAABB = rs->GetAABB();
			if (IsCollision(aabb1, rsAABB)) {
				// 飛び出すトゲに被弾したら 20 ダメージ
				player_->ApplyDamage(20.0f);
			}
		}
	}

	if (worldTransformGoal_) {
		AABB goalAABB = {
		    {worldTransformGoal_->translation_.x - 0.5f, worldTransformGoal_->translation_.y - 0.5f, -0.5f},
		    {worldTransformGoal_->translation_.x + 0.5f, worldTransformGoal_->translation_.y + 0.5f, 0.5f }
        };

		if (IsCollision(player_->GetAABB(), goalAABB)) {
			if (phase_ != Phase::kFadeOut) {
				// フェードアウト開始（1秒くらい）
				fade_->Start(Fade::Status::FadeOut, 1.0f);
				phase_ = Phase::kFadeOut;
				isClear_ = true;
			}
		}
	}
}

//==================================================
// フェーズ切替
//==================================================
void GameScene::ChangePhese() {
	switch (phase_) {
	case Phase::kPlay:
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(dethParticleModel, camera_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:
		break;
	}
}

void GameScene::UpdateDynamicBlocks() {
	uint32_t playerXIndex = mapChipField_->GetMapChipIndexSetByposition(player_->GetPosition()).xIndex;
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();

	// 必要な列数まで vector を拡張
	uint32_t neededColumns = playerXIndex + 50;
	if (worldTransformBlocks_.empty() || worldTransformBlocks_[0].size() < neededColumns) {
		worldTransformBlocks_.resize(numBlockVirtical);
		worldTransformSpikes_.resize(numBlockVirtical);
		worldTransformSprings_.resize(numBlockVirtical);
		worldTransformFires_.resize(numBlockVirtical);
		worldTransformCoins_.resize(numBlockVirtical);
		for (uint32_t i = 0; i < numBlockVirtical; ++i) {
			worldTransformBlocks_[i].resize(neededColumns, nullptr);
			worldTransformSpikes_[i].resize(neededColumns, nullptr);
			worldTransformSprings_[i].resize(neededColumns, nullptr);
			worldTransformFires_[i].resize(neededColumns, nullptr);
			worldTransformCoins_[i].resize(neededColumns, nullptr);
		}
	}

	// 生成範囲: [playerXIndex - 15, playerXIndex + 35]
	int startX = std::max(0, (int)playerXIndex - 15);
	int endX = (int)playerXIndex + 35;

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (int j = startX; j <= endX; ++j) {
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);

			if (type == MapChipType::kBlock && !worldTransformBlocks_[i][j]) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = wt;
			}
			else if (type == MapChipType::kSpike && !worldTransformSpikes_[i][j]) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				wt->scale_ = {0.5f, 0.5f, 0.5f};
				wt->rotation_.y = -std::numbers::pi_v<float> / 2.0f;
				worldTransformSpikes_[i][j] = wt;
			}
			else if (type == MapChipType::kSpring && !worldTransformSprings_[i][j]) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->scale_ = {0.5f, 0.5f, 0.5f};
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformSprings_[i][j] = wt;
			}
			else if (type == MapChipType::kMovingFire && !worldTransformFires_[i][j]) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				wt->scale_ = {0.5f, 0.5f, 0.5f};
				worldTransformFires_[i][j] = wt;
			}
			else if (type == MapChipType::kCoin && !worldTransformCoins_[i][j]) {
				WorldTransform* wt = new WorldTransform();
				wt->Initialize();
				wt->scale_ = {0.45f, 0.45f, 0.12f};
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				wt->rotation_.y = static_cast<float>(rand()) / RAND_MAX * 3.14159265f * 2.0f;
				worldTransformCoins_[i][j] = wt;
			}
			else if (type == MapChipType::kEnemy) {
				Vector3 enemyPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				Enemy* newEnemy = new Enemy();
				newEnemy->Initialize(enemyModel_, camera_, enemyPos, mapChipField_);
				enemies_.push_back(newEnemy);
				mapChipField_->SetMapChipType(j, i, MapChipType::kBlank); // 重複生成防止
			}
			else if (type == MapChipType::kEnemy2) {
				Vector3 enemyPos = mapChipField_->GetMapChipPositionByIndex(j, i);
				Enemy2* newEnemy2 = new Enemy2();
				newEnemy2->Initialize(enemyModel2_, camera_, enemyPos, mapChipField_);
				enemies2_.push_back(newEnemy2);
				mapChipField_->SetMapChipType(j, i, MapChipType::kBlank); // 重複生成防止
			}
			else if (type == MapChipType::kFallingSpike) {
				Vector3 spikePos = mapChipField_->GetMapChipPositionByIndex(j, i);
				FallingSpike* fs = new FallingSpike();
				// プレス機モデルには modelBlock_ を流用する
				fs->Initialize(modelBlock_, camera_, spikePos, mapChipField_);
				fallingSpikes_.push_back(fs);
				mapChipField_->SetMapChipType(j, i, MapChipType::kBlank); // 重複生成防止
			}
			else if (type == MapChipType::kRisingSpike) {
				Vector3 spikePos = mapChipField_->GetMapChipPositionByIndex(j, i);
				RisingSpike* rs = new RisingSpike();
				rs->Initialize(spikeModel_, camera_, spikePos, mapChipField_);
				risingSpikes_.push_back(rs);
				mapChipField_->SetMapChipType(j, i, MapChipType::kBlank); // 重複生成防止
			}
		}
	}

	// 範囲外のオブジェクトを破棄してメモリ解放
	int purgeEndX = (int)playerXIndex - 16;
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (int j = 0; j <= purgeEndX; ++j) {
			if (j < (int)worldTransformBlocks_[i].size()) {
				if (worldTransformBlocks_[i][j]) {
					delete worldTransformBlocks_[i][j];
					worldTransformBlocks_[i][j] = nullptr;
				}
				if (worldTransformSpikes_[i][j]) {
					delete worldTransformSpikes_[i][j];
					worldTransformSpikes_[i][j] = nullptr;
				}
				if (worldTransformSprings_[i][j]) {
					delete worldTransformSprings_[i][j];
					worldTransformSprings_[i][j] = nullptr;
				}
				if (worldTransformFires_[i][j]) {
					delete worldTransformFires_[i][j];
					worldTransformFires_[i][j] = nullptr;
				}
				if (worldTransformCoins_[i][j]) {
					delete worldTransformCoins_[i][j];
					worldTransformCoins_[i][j] = nullptr;
				}
			}
		}
	}

	// 画面外の落下プレス機を破棄
	fallingSpikes_.remove_if([purgeEndX, this](FallingSpike* fs) {
		auto index = mapChipField_->GetMapChipIndexSetByposition(fs->GetPosition());
		if ((int)index.xIndex <= purgeEndX) {
			delete fs;
			return true;
		}
		return false;
	});

	// 画面外の飛び出すトゲを破棄
	risingSpikes_.remove_if([purgeEndX, this](RisingSpike* rs) {
		auto index = mapChipField_->GetMapChipIndexSetByposition(rs->GetPosition());
		if ((int)index.xIndex <= purgeEndX) {
			delete rs;
			return true;
		}
		return false;
	});
}
