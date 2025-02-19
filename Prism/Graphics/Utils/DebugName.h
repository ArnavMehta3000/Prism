#pragma once
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>

namespace Px::Gfx
{
	template <typename T>
	inline void SetDebugObjectName(MAYBE_UNUSED _In_ T resource, MAYBE_UNUSED _In_z_ Elos::StringView name)
	{
#if PRISM_BUILD_DEBUG
		if (resource)
		{
			// Buffer to store the current name
			char currentName[256];
			unsigned int nameLength = sizeof(currentName);
			resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(name.length()), name.data());
		}
#endif
	}
}
