// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#include "BMFontShowcaseActor.h"
#include "BMFontAsset.h"
#include "BMFontText.h"
#include "Widgets/BMFontRichTextBlock.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "ImageUtils.h"
#include "Json.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "RHI.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 ProbeWidth = 32;
	constexpr int32 ProbeHeight = 24;

	bool ExpectedCoverage(int32 Case, int32 X, int32 Y)
	{
		if (Case == 1) // Packed.png stores solid coverage in green, with zero alpha.
		{
			return X < 8 && Y < 10;
		}
		if (Case == 2 && X >= 11) // B follows A's 12px advance with -1px kerning.
		{
			X -= 11;
			return X < 10 && Y < 14 && ((Y < 2 || (Y >= 6 && Y < 8) || Y >= 12) ? X < 8 : X < 2 || X >= 8);
		}
		if (X >= 10 || Y >= 14)
		{
			return false;
		}
		// A in Minimal.png, the first multi-page glyph, and the Rich Text probe.
		return Y < 2 ? X >= 2 && X < 8 : (Y >= 6 && Y < 8) || X < 2 || X >= 8;
	}

	bool CheckPixels(const TArray<FColor>& Pixels, int32 Case, int32& OutMismatches)
	{
		OutMismatches = 0;
		if (Pixels.Num() != ProbeWidth * ProbeHeight)
		{
			return false;
		}
		for (int32 Y = 0; Y < ProbeHeight; ++Y)
		{
			for (int32 X = 0; X < ProbeWidth; ++X)
			{
				const FColor& Pixel = Pixels[Y * ProbeWidth + X];
				const bool Expected = ExpectedCoverage(Case, X, Y);
				if (Expected ? Pixel.A < 240 || Pixel.R < 240 || Pixel.G < 240 || Pixel.B < 240 : Pixel.A > 8)
				{
					++OutMismatches;
				}
			}
		}
		return OutMismatches == 0;
	}
}

ABMFontShowcaseActor::ABMFontShowcaseActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABMFontShowcaseActor::BeginPlay()
{
	Super::BeginPlay();
	bValidate = FParse::Param(FCommandLine::Get(), TEXT("BMFontValidate"));
	FParse::Value(FCommandLine::Get(), TEXT("BMFontEvidenceDir="), EvidenceDirectory);
	if (EvidenceDirectory.IsEmpty())
	{
		EvidenceDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("BMFontEvidence"));
	}
	const auto MakePlain = [this](UBMFontAsset* Font, const TCHAR* Text, float Scale = 1.0f)
	{
		UBMFontText* Widget = NewObject<UBMFontText>(this);
		OwnedWidgets.Add(Widget);
		Widget->SetFontAsset(Font);
		Widget->SetText(FText::FromString(Text));
		Widget->SetFontScale(Scale);
		return Widget->TakeWidget();
	};
	Probes.Add(MakePlain(PlainFont, TEXT("A")));
	Probes.Add(MakePlain(PackedFont, TEXT("A")));
	Probes.Add(MakePlain(MultiPageFont, TEXT("AB")));
	UBMFontRichTextBlock* Rich = NewObject<UBMFontRichTextBlock>(this);
	OwnedWidgets.Add(Rich);
	Rich->SetFontAsset(PlainFont);
	Rich->SetDecoratePlainTextRuns(false);
	Rich->SetText(FText::FromString(TEXT("<bmfont>A</>")));
	Probes.Add(Rich->TakeWidget());

	ViewportWidget = SNew(SBorder).Padding(24.0f).BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.05f, 1.0f))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
		[SNew(STextBlock).Text(FText::FromString(TEXT("Unreal BMFont - imported and cooked asset showcase")))]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("UNREAL BMFONT SHOWCASE"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("0123456789"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("あぁいぃうぅえぇおぉ かがきぎくぐけげこご"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("さざしじすずせぜそぞ ただちぢっつづてでとど"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("なにぬねの はばぱひびぴふぶぷへべぺほぼぽ"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("まみむめも ゃやゅゆょよ らりるれろ ゎわゐゑをん ゔゕゖ ー"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(ShowcaseFont, TEXT("零一二三四五六七八九  A?B"), 0.7f)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
		[SNew(STextBlock).Text(FText::FromString(TEXT("Below: plain + fallback / packed green channel / alternating atlas pages")))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)[MakePlain(PlainFont, TEXT("AB ? AB"), 3.0f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(PackedFont, TEXT("AV?"), 3.0f)]
		+ SVerticalBox::Slot().AutoHeight()[MakePlain(MultiPageFont, TEXT("ABAB"), 3.0f)]
	];
	if (GEngine != nullptr && GEngine->GameViewport != nullptr)
	{
		GEngine->GameViewport->AddViewportWidgetContent(ViewportWidget.ToSharedRef());
	}
}

void ABMFontShowcaseActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bValidate || ++ValidationFrame < 10)
	{
		return;
	}
	ValidateRendering();
}

void ABMFontShowcaseActor::ValidateRendering()
{
	const bool bAssetsValid = PlainFont != nullptr && PackedFont != nullptr && MultiPageFont != nullptr
		&& ShowcaseFont != nullptr && PlainFont->GetPageTexture(0) != nullptr
		&& PackedFont->GetPageTexture(0) != nullptr && MultiPageFont->GetPageTexture(0) != nullptr
		&& MultiPageFont->GetPageTexture(1) != nullptr && ShowcaseFont->GetPageTexture(0) != nullptr;
	bool bPassed = bAssetsValid && !GUsingNullRHI;
	TArray<TSharedPtr<FJsonValue>> Cases;
	const TCHAR* Names[] = { TEXT("Plain"), TEXT("Packed"), TEXT("MultiPage"), TEXT("RichText") };
	IFileManager::Get().MakeDirectory(*EvidenceDirectory, true);
	if (bAssetsValid && !GUsingNullRHI)
	{
		FWidgetRenderer Renderer(false, true);
		for (int32 Index = 0; Index < Probes.Num(); ++Index)
		{
			UTextureRenderTarget2D* Target = Renderer.DrawWidget(Probes[Index], FVector2D(ProbeWidth, ProbeHeight));
			if (ValidationFrame < 13)
			{
				continue; // Warm Slate resource registration across real frames, not a synthetic tick.
			}
			TArray<FColor> Pixels;
			if (Target != nullptr)
			{
				Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels);
			}
			int32 Mismatches = 0;
			bool bCasePassed = CheckPixels(Pixels, Index, Mismatches);
			TArray64<uint8> Png;
			if (Pixels.Num() == ProbeWidth * ProbeHeight)
			{
				FImageUtils::PNGCompressImageArray(ProbeWidth, ProbeHeight, Pixels, Png);
			}
			const FString Filename = FPaths::Combine(EvidenceDirectory, FString(Names[Index]) + TEXT(".png"));
			bCasePassed &= !Png.IsEmpty() && FFileHelper::SaveArrayToFile(Png, *Filename);
			bPassed &= bCasePassed;
			TSharedPtr<FJsonObject> Case = MakeShared<FJsonObject>();
			Case->SetStringField(TEXT("name"), Names[Index]);
			Case->SetBoolField(TEXT("passed"), bCasePassed);
			Case->SetNumberField(TEXT("mismatchedPixels"), Mismatches);
			Case->SetStringField(TEXT("image"), Filename);
			Cases.Add(MakeShared<FJsonValueObject>(Case));
		}
		if (ValidationFrame < 13)
		{
			return;
		}
		bPassed &= Cast<UMaterialInstanceDynamic>(PackedFont->GetPageRenderResource(0, 2)) != nullptr;
		if (ViewportWidget.IsValid())
		{
			// The off-screen renderer adopts this widget into a virtual window. Detach
			// it from the live viewport first; normal interactive Play never takes this path.
			if (GEngine != nullptr && GEngine->GameViewport != nullptr)
			{
				GEngine->GameViewport->RemoveViewportWidgetContent(ViewportWidget.ToSharedRef());
			}
			UTextureRenderTarget2D* Showcase = Renderer.DrawWidget(ViewportWidget.ToSharedRef(), FVector2D(960, 600));
			TArray<FColor> Pixels;
			if (Showcase != nullptr)
			{
				Showcase->GameThread_GetRenderTargetResource()->ReadPixels(Pixels);
			}
			TArray64<uint8> Png;
			if (Pixels.Num() == 960 * 600)
			{
				FImageUtils::PNGCompressImageArray(960, 600, Pixels, Png);
			}
			bPassed &= !Png.IsEmpty() && FFileHelper::SaveArrayToFile(Png,
				*FPaths::Combine(EvidenceDirectory, TEXT("Showcase.png")));
		}
	}
	TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetBoolField(TEXT("passed"), bPassed);
	Result->SetBoolField(TEXT("assetsLoaded"), bAssetsValid);
	Result->SetBoolField(TEXT("nullRHI"), GUsingNullRHI);
	Result->SetStringField(TEXT("engineVersion"), FEngineVersion::Current().ToString());
	Result->SetArrayField(TEXT("cases"), Cases);
	FString Json;
	FJsonSerializer::Serialize(Result, TJsonWriterFactory<>::Create(&Json));
	const bool bSaved = FFileHelper::SaveStringToFile(Json, *FPaths::Combine(EvidenceDirectory, TEXT("result.json")),
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	UE_LOG(LogTemp, Display, TEXT("BMFONT_PACKAGED_RENDER=%s"), bPassed && bSaved ? TEXT("PASS") : TEXT("FAIL"));
	bValidate = false;
	FPlatformMisc::RequestExit(false);
}

void ABMFontShowcaseActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ViewportWidget.IsValid() && GEngine != nullptr && GEngine->GameViewport != nullptr)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ViewportWidget.ToSharedRef());
	}
	ViewportWidget.Reset();
	Probes.Reset();
	OwnedWidgets.Reset();
	Super::EndPlay(EndPlayReason);
}
