#pragma once

#include "Util.h"

class GameTimer {
public:
	GameTimer() :
		curTime(0), 
		prevTime(0),
		pauseTimeSpan(0),
		baseTime(0),
		pauseTime(0),
		paused(false),
		secondPerCount(0.0),
		deltaTime(0.0){

		__int64 countPerSecond;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countPerSecond);
		secondPerCount = 1.0 / (double)countPerSecond;

	};

	~GameTimer() {}

	void Reset() {
		QueryPerformanceCounter((LARGE_INTEGER*)&baseTime);
		curTime = baseTime;
		prevTime = baseTime;
		pauseTimeSpan = 0;
		pauseTime = 0;

		paused = false;
	}

	void Start() {
		if (paused) {
			paused = 1;
			QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
			pauseTimeSpan += curTime - pauseTime;
		}
	}
	
	void Tick() {
		if (paused) deltaTime = 0.0;
		else {
			QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
			deltaTime = (double)(curTime - prevTime) * secondPerCount;
			prevTime = curTime;
		}

	}
	
	void Pause() {
		if (!paused) {
			paused = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&pauseTime);
		}
	}

	float GetDeltaTime() {
		return (float)deltaTime; 
	}

private:
	__int64 curTime;
	__int64 prevTime; 
	__int64 pauseTimeSpan;
	__int64 baseTime;
	__int64 pauseTime;

	bool paused;
	
	double secondPerCount;
	double deltaTime;
};