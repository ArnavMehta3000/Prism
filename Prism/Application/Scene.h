#pragma once
#include "Math/Math.h"

namespace Prism
{
	struct WVP
	{
		Matrix World;
		Matrix View;
		Matrix Projection;
	};

}