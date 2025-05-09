#pragma once
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>

namespace Prism::Gfx
{
	template <typename T>
	inline void SetDebugObjectName(MAYBE_UNUSED _In_ T resource, MAYBE_UNUSED _In_z_ Elos::StringView name)
	{
#if PRISM_BUILD_DEBUG
		if (resource)
		{
			resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(name.length()), name.data());
		}
#endif
	}
}
