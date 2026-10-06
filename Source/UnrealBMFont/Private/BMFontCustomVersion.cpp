// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#include "BMFontCustomVersion.h"
#include "Serialization/CustomVersion.h"

const FGuid FBMFontCustomVersion::GUID(0xFE7A101E, 0xD0EC4B30, 0xB853F86D, 0xA7F3D597);
static FCustomVersionRegistration GRegisterBMFontVersion(
	FBMFontCustomVersion::GUID, FBMFontCustomVersion::LatestVersion, TEXT("BMFontAssetVersion"));
