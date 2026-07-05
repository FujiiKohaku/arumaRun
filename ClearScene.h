#pragma once
#define NOMINMAX
#include "Fade.h"
#include "KamataEngine.h"
#include "Math.h"

class ClearScene {
public:
	void Initialize();
	void Update();
	void Draw();

	bool IsFinished() const { return finished_; }

	enum class Result { None, Retry, Title };
	Result GetResult() const { return result_; }

	enum class State { WaitInput, FadeOut, Finish };
	State state_;

private:
	enum class Phase { kFadeIn, kMain, kFadeOut };

	Phase phase_ = Phase::kFadeIn;
	bool finished_ = false;
	Result result_ = Result::None;

	Fade* fade_ = nullptr;

	KamataEngine::Model* clearFont_ = nullptr; // "GAME CLEAR"文字
	KamataEngine::Model* retryFont_ = nullptr; // "Press Enter"文字

	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* modelSkyDome_ = nullptr;

	KamataEngine::WorldTransform worldTransformSky_;
	KamataEngine::WorldTransform worldTransformClear_;
	KamataEngine::WorldTransform worldTransformPushTo_;

	uint32_t soundHandleClear_;

	uint32_t soundHandleSelect_;
};
