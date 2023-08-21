#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(wdGizmoConstants, 2)
{
	MAT4(ObjectToWorldMatrix);
	MAT4(WorldToObjectMatrix);
	COLOR4F(GizmoColor);
	FLOAT1(GizmoScale);
	INT1(GameObjectID);
};
