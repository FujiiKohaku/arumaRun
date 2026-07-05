#pragma once
#include "KamataEngine.h"
#include "Math.h"
#include <vector>

// マップチップの種類
enum class MapChipType {
	kBlank,  // 空白
	kBlock,  // 普通のブロック
	kSpike,  // トゲブロックを追加
	kEnemy,  // 敵を追加
	kSpring, // ばね追加
	kMovingFire,//火の玉
	kEnemy2,
	kGoal,
	kCoin,
	kFallingSpike,
	kRisingSpike,
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
public:
	void Initialize();
	void UpDate();
	void Draw();
	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filepath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	void SetMapChipType(uint32_t xIndex, uint32_t yIndex, MapChipType type);
	void GenerateNextColumn();
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndedx);

	uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	struct Rect {
		float left;
		float right;
		float top;
		float bottom;
	};

	IndexSet GetMapChipIndexSetByposition(const Vector3& position);
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

private:
	MapChipData mapChipData_;
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 200;

	// 自動生成用パラメータ
	uint32_t safeColumns_ = 10;
};
