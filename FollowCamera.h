#pragma once
#include "EbichaEngine.h"

class FollowCamera
{
public:
	void Initialize();
	void Update();
	void SetTarget(const Transform* target) { target_ = target; }
	Camera& GetCamera(){ return viewProjection_; }
private:
	Camera viewProjection_;
	const Transform* target_ = nullptr;

	static const inline float kRotateSpeed=0.2f;
};

