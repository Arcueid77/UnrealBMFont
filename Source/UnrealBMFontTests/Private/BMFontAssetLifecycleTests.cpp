// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#if WITH_DEV_AUTOMATION_TESTS

#include "BMFontAsset.h"
#include "BMFontCustomVersion.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformTime.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"
#include "Serialization/MemoryWriter.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/BMFontRichTextBlock.h"
#include "Widgets/SBMFontText.h"
#include "Widgets/SInvalidationPanel.h"

namespace
{
	FBMFontData MakeLifecycleData(UBMFontAsset* Asset)
	{
		FBMFontData Data;
		Data.DescriptorFormat = EBMFontDescriptorFormat::Text;
		Data.Common.LineHeight = 16;
		Data.Common.Base = 12;
		Data.Common.ScaleWidth = 16;
		Data.Common.ScaleHeight = 16;
		Data.Common.PageCount = 1;
		FBMFontPage& Page = Data.Pages.AddDefaulted_GetRef();
		Page.Id = 0;
		Page.File = TEXT("atlas.png");
		Page.Texture = NewObject<UTexture2D>(Asset);
		FBMFontGlyph& Glyph = Data.Glyphs.Add(65);
		Glyph.Codepoint = 65;
		Glyph.Width = 8;
		Glyph.Height = 10;
		Glyph.XAdvance = 8;
		Glyph.Page = 0;
		return Data;
	}

	class FWaitForPackedRecovery final : public IAutomationLatentCommand
	{
	public:
		FWaitForPackedRecovery(FAutomationTestBase* InTest, UBMFontAsset* InAsset,
			const TSharedRef<SBMFontText>& InWidget, UMaterial* InMaterial, uint32 InRevision)
			: Test(InTest), Asset(InAsset), Material(InMaterial), Widget(InWidget), Revision(InRevision)
			, Deadline(FPlatformTime::Seconds() + 5.0)
		{
			Handle = Asset->OnChanged().AddLambda([this](EBMFontAssetChange Change)
			{
				bNotified |= Change == EBMFontAssetChange::RenderResources;
			});
		}

		virtual ~FWaitForPackedRecovery() override
		{
			Asset->OnChanged().Remove(Handle);
		}

		virtual bool Update() override
		{
			if (Asset->GetRenderResourceRevision() > Revision && bNotified)
			{
				Test->TestNotNull(TEXT("Automatic recovery replaces an existing raw-texture brush"),
					Cast<UMaterialInstanceDynamic>(Widget->GetCachedGlyphResourceForTesting(65)));
				return true;
			}
			if (FPlatformTime::Seconds() >= Deadline)
			{
				Test->AddError(TEXT("Late material was not adopted and broadcast by the recovery ticker"));
				return true;
			}
			return false;
		}

	private:
		FAutomationTestBase* Test;
		TStrongObjectPtr<UBMFontAsset> Asset;
		TStrongObjectPtr<UMaterial> Material;
		TSharedRef<SBMFontText> Widget;
		uint32 Revision;
		double Deadline;
		FDelegateHandle Handle;
		bool bNotified = false;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBMFontAssetLiveChangeTest,
	"UnrealBMFont.Widget.AssetChangesInvalidateLayout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBMFontAssetLiveChangeTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UBMFontAsset> Asset(NewObject<UBMFontAsset>());
	Asset->SetFontData(MakeLifecycleData(Asset.Get()));
	TSharedRef<SBMFontText> Plain = SNew(SBMFontText).FontAsset(Asset.Get()).Text(FText::FromString(TEXT("AA")));
	TSharedRef<SInvalidationPanel> Panel = SNew(SInvalidationPanel)[Plain];
	Panel->SlatePrepass(1.0f);
	TestEqual(TEXT("Initial plain width"), static_cast<double>(Plain->GetDesiredSize().X), 16.0);
	TStrongObjectPtr<UBMFontRichTextBlock> Rich(NewObject<UBMFontRichTextBlock>());
	Rich->SetFontAsset(Asset.Get());
	Rich->SetDecoratePlainTextRuns(true);
	Rich->SetText(FText::FromString(TEXT("AA")));
	TSharedPtr<SWidget> RichSlate = Rich->TakeWidget();
	RichSlate->SlatePrepass(1.0f);
	TestEqual(TEXT("Initial rich width"), static_cast<double>(RichSlate->GetDesiredSize().X), 16.0, 0.5);

	FBMFontData Updated = Asset->FontData;
	Updated.Glyphs[65].XAdvance = 12;
	Asset->SetFontData(MoveTemp(Updated));
	TestTrue(TEXT("Asset change invalidates a previously measured plain widget"), Plain->NeedsPrepass());
	Panel->SlatePrepass(1.0f);
	RichSlate->SlatePrepass(1.0f);
	TestEqual(TEXT("Existing plain widget observes new metrics"), static_cast<double>(Plain->GetDesiredSize().X), 24.0);
	TestEqual(TEXT("Existing rich widget remeasures its runs"), static_cast<double>(RichSlate->GetDesiredSize().X), 24.0, 0.5);
	Rich->ReleaseSlateResources(true);
	RichSlate.Reset(); // UWidget's weak Slate cache cannot expire while the test owns it.
	Rich->TakeWidget()->SlatePrepass(1.0f);
	Updated = Asset->FontData;
	Updated.Glyphs[65].XAdvance = 9;
	Asset->SetFontData(MoveTemp(Updated));
	Rich->TakeWidget()->SlatePrepass(1.0f);
	TestEqual(TEXT("Rich subscriptions survive Slate reconstruction"), static_cast<double>(Rich->TakeWidget()->GetDesiredSize().X), 18.0, 0.5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBMFontPackedRecoveryTest,
	"UnrealBMFont.Rendering.PackedMaterialRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBMFontPackedRecoveryTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UBMFontAsset> Asset(NewObject<UBMFontAsset>());
	const FString PackageName = TEXT("/Temp/BMFontLateMaterial_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	UPackage* Package = CreatePackage(*PackageName);
	Asset->PackedRenderMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(PackageName + TEXT(".LateMaterial")));
	// UE defines 0 as one-or-more matches, not zero matches. Diagnostic volume can
	// differ between engine loaders; the test still requires a real failed load.
	AddExpectedMessage(TEXT("Failed to find object"), ELogVerbosity::Warning,
		EAutomationExpectedMessageFlags::Contains, 0, false);
	AddExpectedMessage(TEXT("pages fall back to raw textures"), ELogVerbosity::Warning,
		EAutomationExpectedMessageFlags::Contains, 1, false);
	FBMFontData Data = MakeLifecycleData(Asset.Get());
	Data.Common.bPacked = true;
	Asset->SetFontData(MoveTemp(Data));
	TSharedRef<SBMFontText> Widget = SNew(SBMFontText).FontAsset(Asset.Get()).Text(FText::FromString(TEXT("A")));
	Widget->GetCachedGlyphBrushCountForTesting(100.0f);
	TestEqual(TEXT("Failed load initially uses the documented raw fallback"),
		Widget->GetCachedGlyphResourceForTesting(65), static_cast<UObject*>(Asset->GetPageTexture(0)));
	const uint32 Before = Asset->GetRenderResourceRevision();
	// Complete the same soft object without touching the font or widget again.
	// Wait across real engine ticks: a manual refresh would hide ticker regressions.
	UMaterial* LateMaterial = NewObject<UMaterial>(Package, TEXT("LateMaterial"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForPackedRecovery(this, Asset.Get(), Widget, LateMaterial, Before));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBMFontAssetVersionTest,
	"UnrealBMFont.Asset.CustomVersionAndGarbageCollection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBMFontAssetVersionTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UBMFontAsset> Asset(NewObject<UBMFontAsset>());
	Asset->SetFontData(MakeLifecycleData(Asset.Get()));
	TArray<uint8> Bytes;
	FMemoryWriter Writer(Bytes);
	// UObject properties include object pointers; only exercise the custom-version
	// registration here. Real disk serialization is covered by PackedRealPackageSave.
	Writer.UsingCustomVersion(FBMFontCustomVersion::GUID);
	TestEqual(TEXT("Registered serialized schema version"), Writer.CustomVer(FBMFontCustomVersion::GUID),
		static_cast<int32>(FBMFontCustomVersion::LatestVersion));
	TSharedRef<SBMFontText> Widget = SNew(SBMFontText).FontAsset(Asset.Get()).Text(FText::FromString(TEXT("A")));
	Widget->GetCachedGlyphBrushCountForTesting(100.0f);
	Asset.Reset();
	CollectGarbage(RF_NoFlags);
	TestEqual(TEXT("A raw Slate widget tolerates collection of its weak font reference"),
		Widget->GetCachedGlyphBrushCountForTesting(100.0f), 0);
	return true;
}

#endif
