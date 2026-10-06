// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#pragma once

#include "BMFontTypes.h"
#include "Containers/Ticker.h"
#include "UObject/Object.h"
#include "BMFontAsset.generated.h"

class UAssetImportData;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class FBMFontLayout;

enum class EBMFontAssetChange : uint8
{
	Metrics,
	RenderResources
};

DECLARE_MULTICAST_DELEGATE_OneParam(FBMFontAssetChanged, EBMFontAssetChange);

/** Imported BMFont descriptor data and hard references to every atlas page. */
UCLASS(BlueprintType)
class UNREALBMFONT_API UBMFontAsset : public UObject
{
	GENERATED_BODY()

public:
	UBMFontAsset();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BMFont")
	FBMFontData FontData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Instanced, Category = "Import Settings")
	TObjectPtr<UAssetImportData> AssetImportData;
#endif

	/**
	 * Material that extracts glyph coverage from packed-channel atlas pages.
	 * Defaults to the plugin's M_BMFontPacked; ignored for non-packed descriptors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BMFont", AdvancedDisplay, NoClear)
	TSoftObjectPtr<UMaterialInterface> PackedRenderMaterial;

	UFUNCTION(BlueprintPure, Category = "BMFont")
	bool HasGlyph(int32 Codepoint) const;

	UFUNCTION(BlueprintPure, Category = "BMFont")
	int32 GetLineHeight() const;

	UFUNCTION(BlueprintPure, Category = "BMFont")
	int32 GetPageCount() const;

	/** Copies glyph metrics for Blueprint callers without exposing an internal map pointer. */
	UFUNCTION(BlueprintPure, Category = "BMFont")
	bool GetGlyph(int32 Codepoint, FBMFontGlyph& OutGlyph) const;

	UFUNCTION(BlueprintPure, Category = "BMFont")
	UTexture2D* GetPageTexture(int32 PageId) const;

	/**
	 * Returns the Slate brush resource for a page and glyph channel: the page texture
	 * itself, or a cached channel-extraction material instance when the descriptor uses
	 * packed channels.
	 */
	UObject* GetPageRenderResource(int32 PageId, int32 GlyphChannel = 15);

	const FBMFontGlyph* FindGlyph(int32 Codepoint) const;
	const FBMFontPage* FindPage(int32 PageId) const;
	UFUNCTION(BlueprintPure, Category = "BMFont")
	int32 GetKerning(int32 FirstCodepoint, int32 SecondCodepoint) const;
	uint32 GetDataRevision() const;
	uint32 GetRenderResourceRevision() const { return RenderResourceRevision; }
	/** Game-thread changes notify live Slate/UMG consumers, including invalidation panels. */
	FBMFontAssetChanged& OnChanged() { return ChangedEvent; }
	void SetFontData(FBMFontData InFontData);
	UFUNCTION(BlueprintCallable, Category = "BMFont")
	void SetPackedRenderMaterial(TSoftObjectPtr<UMaterialInterface> InMaterial);
	/** Adopt an already loaded material without unbounded synchronous retries. */
	void RefreshRenderResources();

	virtual void Serialize(FArchive& Ar) override;
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif

private:
	friend class FBMFontLayout;

	void RebuildLookup();
	void ClearRenderResourceCache();
	void EnsurePackedRenderMaterialReference();
	void EnsurePackedRenderMaterialLoaded(bool bAllowRenderRetry = false);
	void AdoptPackedRenderMaterial(UMaterialInterface* Material);

	TMap<uint64, int32> KerningLookup;
	uint32 DataRevision = 1;
	uint32 RenderResourceRevision = 1;
	FBMFontAssetChanged ChangedEvent;
	FTSTicker::FDelegateHandle MaterialRecoveryHandle;
	FTSTicker::FDelegateHandle ResourceNotificationHandle;

	UPROPERTY(Transient)
	TMap<uint64, TObjectPtr<UMaterialInstanceDynamic>> PageMaterialCache;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> LoadedPackedRenderMaterial = nullptr;

	TMap<uint64, TWeakObjectPtr<UTexture2D>> PageMaterialSources;
	bool bPackedMaterialLoadAttempted = false;
	bool bPackedMaterialRenderRetryAttempted = false;
	bool bWarnedMissingPackedMaterial = false;
};
