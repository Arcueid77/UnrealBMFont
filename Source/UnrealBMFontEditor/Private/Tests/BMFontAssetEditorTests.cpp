// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetEditor/BMFontAssetEditor.h"
#include "AssetEditor/SBMFontAtlasPreview.h"
#include "AssetRegistry/AssetData.h"
#include "BMFontAsset.h"
#include "BMFontAssetDefinition.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/HittestGrid.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/ObjectThumbnail.h"
#include "ObjectTools.h"
#include "Rendering/DrawElements.h"
#include "RHI.h"
#include "ShaderCompiler.h"
#include "Slate/WidgetRenderer.h"
#include "Styling/AppStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBMFontAssetEditorSummaryTest,
	"UnrealBMFont.Editor.AssetEditorSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBMFontAssetEditorSummaryTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UBMFontAsset> Asset(NewObject<UBMFontAsset>());
	Asset->FontData.Info.Face = TEXT("Summary fixture");
	TSharedRef<SVerticalBox> Summary = SNew(SVerticalBox);
	FBMFontAssetEditor::PopulateSummaryPanel(Summary, Asset.Get());
	Summary->SlatePrepass(1.0f);

	TestEqual(TEXT("Plain summary attaches all metadata rows"), Summary->GetChildren()->Num(), 11);
	TestTrue(TEXT("Summary has a nonzero desired height"), Summary->GetDesiredSize().Y > 0.0f);

	Asset->FontData.Common.bPacked = true;
	FBMFontAssetEditor::PopulateSummaryPanel(Summary, Asset.Get());
	TestEqual(TEXT("Packed summary adds four channel rows"), Summary->GetChildren()->Num(), 15);
	FBMFontAssetEditor::PopulateSummaryPanel(Summary, Asset.Get());
	TestEqual(TEXT("Refresh replaces rather than duplicates rows"), Summary->GetChildren()->Num(), 15);
	FBMFontAssetEditor::PopulateSummaryPanel(Summary, nullptr);
	TestEqual(TEXT("Missing asset clears stale summary rows"), Summary->GetChildren()->Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBMFontAtlasPreviewPaintTest,
	"UnrealBMFont.Editor.AtlasPreviewPaint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBMFontAtlasPreviewPaintTest::RunTest(const FString& Parameters)
{
	TSharedRef<SBMFontAtlasPreview> Preview = SNew(SBMFontAtlasPreview);
	TSharedRef<SWindow> Window = SNew(SWindow).ClientSize(FVector2D(64.0f, 64.0f));
	FSlateWindowElementList Elements(Window);
	FHittestGrid HitTestGrid;
	FPaintArgs PaintArgs(nullptr, HitTestGrid, FVector2D::ZeroVector, 0.0, 0.0f);
	FWidgetStyle Style;
	Style.BlendColorAndOpacityTint(FLinearColor(0.5f, 0.6f, 0.7f, 0.8f));
	Preview->OnPaint(
		PaintArgs,
		FGeometry::MakeRoot(FVector2D(64.0f, 64.0f), FSlateLayoutTransform()),
		FSlateRect(0.0f, 0.0f, 64.0f, 64.0f),
		Elements,
		0,
		Style,
		true
	);
	const auto& Boxes = Elements.GetUncachedDrawElements().Get<static_cast<uint8>(EElementType::ET_Box)>();
	if (TestEqual(TEXT("An empty preview still paints its background"), Boxes.Num(), 1))
	{
		const FLinearColor ExpectedTint = Style.GetColorAndOpacityTint()
			* FAppStyle::GetBrush(TEXT("Brushes.Secondary"))->GetTint(Style);
		TestEqual(TEXT("Background applies theme and inherited tint"), Boxes[0].GetTint(), ExpectedTint);
		TestTrue(TEXT("Background is not opaque white"), Boxes[0].GetTint() != FLinearColor::White);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBMFontAtlasPreviewPackedChannelsTest,
	"UnrealBMFont.Editor.AtlasPreviewPackedChannels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBMFontAtlasPreviewPackedChannelsTest::RunTest(const FString& Parameters)
{
	if (GUsingNullRHI)
	{
		AddInfo(TEXT("Packed preview pixels require a GPU run without -NullRHI; skipping."));
		return true;
	}

	TStrongObjectPtr<UBMFontAsset> Asset(NewObject<UBMFontAsset>());
	TStrongObjectPtr<UTexture2D> Texture(UTexture2D::CreateTransient(8, 8, PF_B8G8R8A8));
	Texture->SRGB = false;
	Texture->Filter = TF_Nearest;
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	FColor* AtlasPixels = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
	for (int32 Y = 0; Y < 8; ++Y)
	{
		for (int32 X = 0; X < 8; ++X)
		{
			// Two glyphs share the same rectangle but use different RGB channels;
			// their alpha is always zero, so raw-texture preview cannot show either.
			AtlasPixels[Y * 8 + X] = FColor(
				X >= 1 && X <= 2 && Y >= 1 && Y <= 6 ? 255 : 0,
				X >= 5 && X <= 6 && Y >= 1 && Y <= 6 ? 255 : 0, 0, 0);
		}
	}
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	FBMFontData Data;
	Data.Common.ScaleWidth = 8;
	Data.Common.ScaleHeight = 8;
	Data.Common.LineHeight = 8;
	Data.Common.bPacked = true;
	Data.Common.AlphaChannel = EBMFontChannelContent::Zero;
	Data.Common.RedChannel = EBMFontChannelContent::Glyph;
	Data.Common.GreenChannel = EBMFontChannelContent::Glyph;
	Data.Common.BlueChannel = EBMFontChannelContent::Zero;
	FBMFontPage& Page = Data.Pages.AddDefaulted_GetRef();
	Page.Id = 0;
	Page.Texture = Texture.Get();
	for (const int32 Channel : {2, 4})
	{
		FBMFontGlyph& Glyph = Data.Glyphs.Add(Channel);
		Glyph.Codepoint = Channel;
		Glyph.Page = 0;
		Glyph.Width = 8;
		Glyph.Height = 8;
		Glyph.Channel = Channel;
	}
	Asset->SetFontData(MoveTemp(Data));
	TSharedRef<SBMFontAtlasPreview> Preview = SNew(SBMFontAtlasPreview).FontAsset(Asset.Get());
	FWidgetRenderer Renderer(false, true);
	const FVector2D DrawSize(64.0f, 64.0f);
	Renderer.DrawWidget(Preview, DrawSize);
	FSlateApplication::Get().Tick();
	if (GShaderCompilingManager != nullptr)
	{
		GShaderCompilingManager->FinishAllCompilation();
	}
	FSlateApplication::Get().Tick();
	TStrongObjectPtr<UTextureRenderTarget2D> Target(Renderer.DrawWidget(Preview, DrawSize));
	if (!TestNotNull(TEXT("Packed atlas preview renders to a target"), Target.Get()))
	{
		return false;
	}
	TArray<FColor> Pixels;
	Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels);
	if (!TestEqual(TEXT("Packed preview has a complete pixel buffer"), Pixels.Num(), 64 * 64))
	{
		return false;
	}
	const auto IsWhite = [&Pixels](int32 X, int32 Y)
	{
		const FColor& Pixel = Pixels[Y * 64 + X];
		return Pixel.R > 240 && Pixel.G > 240 && Pixel.B > 240;
	};
	TestTrue(TEXT("Red-channel coverage is visible with zero atlas alpha"), IsWhite(12, 28));
	TestTrue(TEXT("Green-channel coverage is also visible on the same page"), IsWhite(44, 28));
	TestFalse(TEXT("Unused atlas pixels remain background"), IsWhite(32, 28));

	FObjectThumbnail Thumbnail;
	ThumbnailTools::RenderThumbnail(Asset.Get(), 64, 64,
		ThumbnailTools::EThumbnailTextureFlushMode::AlwaysFlush, nullptr, &Thumbnail);
	const TArray<uint8>& ThumbnailBytes = Thumbnail.GetUncompressedImageData();
	if (!TestTrue(TEXT("Packed font thumbnail produces RGBA pixels"), ThumbnailBytes.Num() >= 8 * 8 * 4))
	{
		return false;
	}
	int32 VisibleThumbnailPixels = 0;
	for (int32 Index = 0; Index + 3 < ThumbnailBytes.Num(); Index += 4)
	{
		if (ThumbnailBytes[Index] > 240 && ThumbnailBytes[Index + 1] > 240 && ThumbnailBytes[Index + 2] > 240)
		{
			++VisibleThumbnailPixels;
		}
	}
	TestEqual(TEXT("Packed thumbnail shows both 2-by-6 coverage regions despite zero alpha"), VisibleThumbnailPixels, 24);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBMFontAssetDefinitionImportMetadataTest,
	"UnrealBMFont.Editor.AssetDefinitionImportMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBMFontAssetDefinitionImportMetadataTest::RunTest(const FString& Parameters)
{
	const UBMFontAssetDefinition* Definition = GetDefault<UBMFontAssetDefinition>();
	TestTrue(TEXT("Asset definition enables standard import/reimport actions"), Definition->CanImport());

	TStrongObjectPtr<UBMFontAsset> Asset(NewObject<UBMFontAsset>());
	const FString SourceFilename = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectIntermediateDir(), TEXT("UnrealBMFontTests/source font.fnt"))
	);
	Asset->AssetImportData->UpdateFilenameOnly(SourceFilename);
	FAssetData Metadata(Asset.Get());
	FString SourceTag;
	TestTrue(TEXT("Asset registry exposes source-file metadata"), Metadata.GetTagValue(UObject::SourceFileTagName(), SourceTag));

	FAssetSourceFilesArgs Args;
	Args.Assets = MakeArrayView(&Metadata, 1);
	Args.FilePathFormat = EPathUse::AbsolutePath;
	TArray<FString> Sources;
	const EAssetCommandResult Result = Definition->GetSourceFiles(Args,
		[&Sources](const FAssetSourceFilesResult& Source)
		{
			Sources.Add(Source.FilePath);
			return true;
		});
	TestEqual(TEXT("Standard asset definition resolves imported source files"), Result, EAssetCommandResult::Handled);
	if (TestEqual(TEXT("One descriptor source is exposed"), Sources.Num(), 1))
	{
		TestEqual(TEXT("Source filename round-trips through registry metadata"), Sources[0], SourceFilename);
	}
	return true;
}

#endif
