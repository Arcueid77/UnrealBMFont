// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

/** Serialized asset schema. Unversioned 0.x assets retain their reflected-property layout. */
struct UNREALBMFONT_API FBMFontCustomVersion
{
	static const FGuid GUID;
	enum Type : int32
	{
		Initial = 0,
		LatestVersion = Initial
	};
};
