#pragma once
#include "EbichaEngine.h"

class Player
{
public:
	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

private:
	Model* modelTeapot_ = nullptr;

	static inline const float kAcceleration = 0.2f;
};

