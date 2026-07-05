#include "MapChipField.h"
#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <cstdlib> // rand() のため追加

// 内部リンケージ
namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank     },
    {"1", MapChipType::kBlock     },
    {"2", MapChipType::kSpike     },
    {"3", MapChipType::kEnemy     },
    {"4", MapChipType::kSpring    },
    {"5", MapChipType::kMovingFire},
    {"6", MapChipType::kEnemy2    },
    {"7", MapChipType::kGoal      },
};
}

// マップチップデータをリセット（初期平地の生成）
void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	safeColumns_ = 10;

	// 最初は40列分安全な平地をあらかじめ生成しておく
	for (uint32_t i = 0; i < 40; ++i) {
		GenerateNextColumn();
	}
}

// CSV読み込み（自動生成化に伴い不要になったが、呼び出し元の互換性のため残す）
void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	(void)filePath;
	ResetMapChipData();
}

// インデックスから座標取得
Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0); }

// 動的拡張付きマップチップ取得
MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (yIndex >= kNumBlockVirtical) {
		return MapChipType::kBlank;
	}

	// データが足りなければ自動生成で拡張
	while (mapChipData_.data.empty() || mapChipData_.data[0].size() <= xIndex) {
		GenerateNextColumn();
	}

	return mapChipData_.data[yIndex][xIndex];
}

// 動的拡張付きマップチップ設定
void MapChipField::SetMapChipType(uint32_t xIndex, uint32_t yIndex, MapChipType type) {
	if (yIndex >= kNumBlockVirtical) return;

	// データが足りなければ自動生成で拡張
	while (mapChipData_.data.empty() || mapChipData_.data[0].size() <= xIndex) {
		GenerateNextColumn();
	}

	mapChipData_.data[yIndex][xIndex] = type;
}

// ルールベースの自動生成（クリア不可能な配置を回避する）
void MapChipField::GenerateNextColumn() {
	uint32_t currentX = mapChipData_.data.empty() ? 0 : static_cast<uint32_t>(mapChipData_.data[0].size());

	// デフォルト地面（最下部とその上をブロックにする）
	std::vector<MapChipType> column(kNumBlockVirtical, MapChipType::kBlank);
	column[19] = MapChipType::kBlock; // 最下部
	column[18] = MapChipType::kBlock; // 床

	// スタート位置周辺（最初の40列）は安全な平地
	if (currentX < 40) {
		for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
			mapChipData_.data[y].push_back(column[y]);
		}
		return;
	}

	// 最初のギミック（X=40）として、確実に「飛び出すトゲ」を出現させる
	if (currentX == 40) {
		for (int w = 0; w < 2; ++w) {
			std::vector<MapChipType> spikeColumn = column;
			spikeColumn[17] = MapChipType::kRisingSpike;
			spikeColumn[14] = MapChipType::kCoin; // 飛び越しコイン
			for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
				mapChipData_.data[y].push_back(spikeColumn[y]);
			}
		}
		safeColumns_ = 7;
		return;
	}

	// 安全カウンタが残っていれば平地を生成
	if (safeColumns_ > 0) {
		safeColumns_--;
	} else {
		// ギミックの生成
		int r = rand() % 100;

		if (r < 30) {
			// 平地を延長
			safeColumns_ = rand() % 6 + 4; // 4~9列の安全地帯 (元の2~6列から増加)
		}
		else if (r < 45) {
			// 穴（奈落）を生成 (幅2〜3マス) + 【誘導コイン配置】
			uint32_t pitWidth = rand() % 2 + 2;

			// ① 穴の手前1列に、ジャンプ踏み切り誘導コイン(y=16)を配置するため、直前列の上書き
			size_t lastIndex = mapChipData_.data[16].size() - 1;
			if (mapChipData_.data[16][lastIndex] == MapChipType::kBlank) {
				mapChipData_.data[16][lastIndex] = MapChipType::kCoin;
			}

			// ② 穴の空中上空に放物線を描いてコインを配置
			for (uint32_t w = 0; w < pitWidth; ++w) {
				std::vector<MapChipType> pitColumn(kNumBlockVirtical, MapChipType::kBlank);
				// 穴の上空にサインカーブの放物線
				int coinY = 15;
				if (pitWidth == 2) {
					coinY = (w == 0 || w == 1) ? 14 : 15;
				} else { // pitWidth == 3
					coinY = (w == 1) ? 13 : 14;
				}
				pitColumn[coinY] = MapChipType::kCoin;

				for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
					mapChipData_.data[y].push_back(pitColumn[y]);
				}
			}

			// ③ 穴の直後に着地誘導コイン(y=16)を配置
			std::vector<MapChipType> postColumn = column;
			postColumn[16] = MapChipType::kCoin;
			for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
				mapChipData_.data[y].push_back(postColumn[y]);
			}

			safeColumns_ = 8; // 穴の後の安全地帯 (5から8に増加)
			return; // 既に列を挿入したので終了
		}
		else if (r < 62) {
			// トゲを配置 (床の上 y=17) + 【飛び越し誘導コイン配置】
			int spikeWidth = rand() % 2 + 1; // 1~2マスのトゲ
			for (int w = 0; w < spikeWidth; ++w) {
				std::vector<MapChipType> spikeColumn = column;
				spikeColumn[17] = MapChipType::kSpike;
				// トゲをジャンプで超えるための放物線コイン
				int coinY = (spikeWidth == 1) ? 15 : 14;
				spikeColumn[coinY] = MapChipType::kCoin;

				for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
					mapChipData_.data[y].push_back(spikeColumn[y]);
				}
			}
			safeColumns_ = 7; // トゲの後の安全地帯 (5から7に増加)
			return; // 既に列を挿入したので終了
		}
		else if (r < 72) {
			// 壁（高さ2マス。ジャンプで超えられる）
			column[17] = MapChipType::kBlock;
			column[16] = MapChipType::kBlock;
			safeColumns_ = 7; // 壁の後の安全地帯 (4から7に増加)
		}
		else if (r < 80) {
			// バネ + 高壁 + 【大ジャンプ軌道の放物線コイン配置】
			// 1列目: バネ (バネの上にコイン y=15)
			std::vector<MapChipType> col1 = column;
			col1[17] = MapChipType::kSpring;
			col1[15] = MapChipType::kCoin;
			for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
				mapChipData_.data[y].push_back(col1[y]);
			}
			// 2列目: 上昇頂点 (上空高くにコイン y=11)
			std::vector<MapChipType> col2 = column;
			col2[11] = MapChipType::kCoin;
			for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
				mapChipData_.data[y].push_back(col2[y]);
			}
			// 3列目: 高壁 (壁の上空 y=11 にコイン)
			std::vector<MapChipType> col3 = column;
			col3[17] = MapChipType::kBlock;
			col3[16] = MapChipType::kBlock;
			col3[15] = MapChipType::kBlock;
			col3[14] = MapChipType::kBlock;
			col3[11] = MapChipType::kCoin;
			for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
				mapChipData_.data[y].push_back(col3[y]);
			}
			// 4列目: 下降・着地側 (少し降りてきた位置にコイン y=13)
			std::vector<MapChipType> col4 = column;
			col4[13] = MapChipType::kCoin;
			for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
				mapChipData_.data[y].push_back(col4[y]);
			}
			safeColumns_ = 10; // 大ジャンプ着地後の安全地帯 (6から10に増加)
			return; // 既に列を挿入したので終了
		}
		else if (r < 85) {
			// 空中に動く火 (y=13)
			column[13] = MapChipType::kMovingFire;
			safeColumns_ = 7; // 火の後の安全地帯 (4から7に増加)
		}
		else if (r < 90) {
			// しゃがみ必須の低天井（トンネル）を生成 + 【トンネル内直線コイン配置】

			// 1. 手前の余裕 (4マスの平地。3から4に増加)
			for (uint32_t w = 0; w < 4; ++w) {
				for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
					mapChipData_.data[y].push_back(column[y]);
				}
			}

			// 2. トンネル本体 (幅2〜3マス、トンネルの床直上 y=17 にコインを並べる)
			uint32_t tunnelWidth = rand() % 2 + 2; // 2~3マス
			for (uint32_t w = 0; w < tunnelWidth; ++w) {
				std::vector<MapChipType> tunnelColumn = column;
				tunnelColumn[16] = MapChipType::kBlock;
				tunnelColumn[15] = MapChipType::kBlock;
				tunnelColumn[14] = MapChipType::kBlock;
				tunnelColumn[17] = MapChipType::kCoin; // しゃがみスライディングで綺麗に取れる！

				for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
					mapChipData_.data[y].push_back(tunnelColumn[y]);
				}
			}

			// 3. 奥の余裕 (10マスの安全地帯。8から10に増加)
			safeColumns_ = 10; 
			return; // 既に列を挿入したので終了
		}
		else if (r < 94) {
			// 落下するプレス機（落下ツララ）を生成 (幅3マス連続で生成、前後に十分な余裕)
			uint32_t pressWidth = 3;
			
			// ① 手前の余裕 (3マスの平地)
			for (uint32_t w = 0; w < 3; ++w) {
				for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
					mapChipData_.data[y].push_back(column[y]);
				}
			}

			// ② プレス機本体の列 (3列分、空中 y=13 に配置)
			// 下のスライディング隙間にコインを直線上に3枚並べる！
			for (uint32_t w = 0; w < pressWidth; ++w) {
				std::vector<MapChipType> pressColumn = column;
				pressColumn[13] = MapChipType::kFallingSpike;
				pressColumn[17] = MapChipType::kCoin; // スライディング用コイン！

				for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
					mapChipData_.data[y].push_back(pressColumn[y]);
				}
			}

			// ③ 奥の余裕の1列目 (落下後に上に乗って取る用の着地誘導コインを y=13 に配置)
			std::vector<MapChipType> postColumn = column;
			postColumn[13] = MapChipType::kCoin;
			for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
				mapChipData_.data[y].push_back(postColumn[y]);
			}

			// ④ 奥の残りの安全地帯 (8マスの平地)
			safeColumns_ = 8;
			return; // 既に列を挿入したので終了
		}
		else if (r < 98) {
			// 飛び出すトゲ（下からせり出すトゲ）を生成 (幅1〜2マス)
			int spikeWidth = rand() % 2 + 1; // 1~2マス
			for (int w = 0; w < spikeWidth; ++w) {
				std::vector<MapChipType> spikeColumn = column;
				spikeColumn[17] = MapChipType::kRisingSpike;
				// 飛び越し誘導コイン (トゲの上空)
				int coinY = (spikeWidth == 1) ? 15 : 14;
				spikeColumn[coinY] = MapChipType::kCoin;

				for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
					mapChipData_.data[y].push_back(spikeColumn[y]);
				}
			}
			safeColumns_ = 7; // トゲの後の安全地帯
			return; // 既に列を挿入したので終了
		}
		else {
			// 敵
			column[17] = (rand() % 2 == 0) ? MapChipType::kEnemy : MapChipType::kEnemy2;
			safeColumns_ = 7; // 敵の後の安全地帯 (5から7に増加)
		}
	}

	// ===== 平地（安全地帯）を走っている際の簡易直線コイン配置 =====
	if (safeColumns_ > 2 && column[17] == MapChipType::kBlank && column[16] == MapChipType::kBlank) {
		// 30%の確率で、走りながらそのまま回収できる高さ(y=16)にコインを配置
		if (rand() % 10 < 3) {
			column[16] = MapChipType::kCoin;
		}
	}

	// 1列追加
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		mapChipData_.data[y].push_back(column[y]);
	}
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByposition(const Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	indexSet.yIndex = kNumBlockVirtical - 1 - static_cast<uint32_t>((position.y + kBlockHeight / 2.0f) / kBlockHeight);
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
	Rect rect = {};
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	return rect;
}
