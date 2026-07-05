#pragma once
#define NOMINMAX
#include "Fade.h"
#include "KamataEngine.h"
#include "Math.h"
class GameOverScene {
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

	KamataEngine::Model* gameOverFont_ = nullptr;
	KamataEngine::Model* retryFont_ = nullptr;

	KamataEngine::Camera* camera_;

	KamataEngine::Model* modelSkyDome_;

	KamataEngine::WorldTransform worldTransformSky_;
	KamataEngine::WorldTransform worldTransformGameover_;
	KamataEngine::WorldTransform worldTransformPushTo_;

	uint32_t soundHandleGameOver_ = 0;
	uint32_t soundHandleSelect_ = 0;
};
