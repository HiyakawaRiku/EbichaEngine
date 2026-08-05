#pragma once
#include "EbichaEngine.h"

class BaseCharacter
{
public:
	virtual void Initialize(const std::vector<Model*>& models);
	virtual void Update(const Camera& activeCamera_);
	virtual void Draw();

protected:
	std::vector<Model*>models_;
	Transform transform_;
};

