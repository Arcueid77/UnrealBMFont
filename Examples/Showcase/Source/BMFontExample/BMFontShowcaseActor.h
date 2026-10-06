// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BMFontShowcaseActor.generated.h"

class UBMFontAsset;
class UWidget;
class SWidget;

/** Map-owned hard references exercise the same dependency cook as a user project. */
UCLASS()
class ABMFontShowcaseActor : public AActor
{
	GENERATED_BODY()

public:
	ABMFontShowcaseActor();
	UPROPERTY(EditAnywhere, Category = "BMFont") TObjectPtr<UBMFontAsset> PlainFont;
	UPROPERTY(EditAnywhere, Category = "BMFont") TObjectPtr<UBMFontAsset> PackedFont;
	UPROPERTY(EditAnywhere, Category = "BMFont") TObjectPtr<UBMFontAsset> MultiPageFont;
	UPROPERTY(EditAnywhere, Category = "BMFont") TObjectPtr<UBMFontAsset> ShowcaseFont;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ValidateRendering();
	UPROPERTY(Transient) TArray<TObjectPtr<UWidget>> OwnedWidgets;
	TArray<TSharedRef<SWidget>> Probes;
	TSharedPtr<SWidget> ViewportWidget;
	FString EvidenceDirectory;
	int32 ValidationFrame = 0;
	bool bValidate = false;
};
