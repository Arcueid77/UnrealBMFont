// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#include "Thumbnail/BMFontThumbnailRenderer.h"

#include "BMFontAsset.h"
#include "BMFontRendering.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine/Texture2D.h"
#include "Texture2DPreview.h"

namespace
{
	class FBMFontPackedThumbnailParameters final : public FBatchedElementTexture2DPreviewParameters
	{
	public:
		explicit FBMFontPackedThumbnailParameters(const FLinearColor& Coverage)
			: FBatchedElementTexture2DPreviewParameters(-1.0f, 0.0f, -1.0f, false, true, false, false, false, true)
		{
			const FPlane Weights(Coverage.R, Coverage.G, Coverage.B, Coverage.A);
			ColorWeights = FMatrix(Weights, Weights, Weights, Weights);
		}

		virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit,
			ERHIFeatureLevel::Type FeatureLevel, const FMatrix& Transform, float Gamma,
			const FMatrix& InColorWeights, const FTexture* Texture) override
		{
			FBatchedElementTexture2DPreviewParameters::BindShaders(
				RHICmdList, GraphicsPSOInit, FeatureLevel, Transform, Gamma, ColorWeights, Texture);
		}

	private:
		FMatrix ColorWeights;
	};
}

UTexture2D* UBMFontThumbnailRenderer::ResolvePreviewTexture(const UBMFontAsset* Asset)
{
	return Asset != nullptr && !Asset->FontData.Pages.IsEmpty()
		? Asset->GetPageTexture(Asset->FontData.Pages[0].Id)
		: nullptr;
}

void UBMFontThumbnailRenderer::Draw(
	UObject* Object,
	const int32 X,
	const int32 Y,
	const uint32 Width,
	const uint32 Height,
	FRenderTarget* Viewport,
	FCanvas* Canvas,
	const bool bAdditionalViewFamily)
{
	const UBMFontAsset* Asset = Cast<UBMFontAsset>(Object);
	UTexture2D* Texture = ResolvePreviewTexture(Asset);
	if (Texture == nullptr || Texture->GetResource() == nullptr)
	{
		return;
	}

	const float TextureWidth = static_cast<float>(Texture->GetSizeX());
	const float TextureHeight = static_cast<float>(Texture->GetSizeY());
	if (TextureWidth <= 0.0f || TextureHeight <= 0.0f)
	{
		return;
	}

	// Fit the page into the thumbnail without upscaling small atlases.
	const float FitScale = FMath::Min3(
		static_cast<float>(Width) / TextureWidth,
		static_cast<float>(Height) / TextureHeight,
		1.0f
	);
	const float DrawWidth = TextureWidth * FitScale;
	const float DrawHeight = TextureHeight * FitScale;
	const float DrawX = static_cast<float>(X) + (static_cast<float>(Width) - DrawWidth) * 0.5f;
	const float DrawY = static_cast<float>(Y) + (static_cast<float>(Height) - DrawHeight) * 0.5f;

	FCanvasTileItem TileItem(
		FVector2D(DrawX, DrawY),
		Texture->GetResource(),
		FVector2D(DrawWidth, DrawHeight),
		FVector2D(0.0f, 0.0f),
		FVector2D(1.0f, 1.0f),
		FLinearColor::White
	);
	TileItem.BlendMode = SE_BLEND_Translucent;
	TRefCountPtr<FBatchedElementParameters> PackedParameters;
	if (Asset->FontData.Common.bPacked)
	{
		FLinearColor Coverage = FLinearColor::Transparent;
		float ConstantBias = 0.0f;
		const int32 PageId = Asset->FontData.Pages[0].Id;
		for (const TPair<int32, FBMFontGlyph>& Entry : Asset->FontData.Glyphs)
		{
			const FBMFontGlyph& Glyph = Entry.Value;
			if (Glyph.Page != PageId || Glyph.Width <= 0 || Glyph.Height <= 0)
			{
				continue;
			}
			const FBMFontPackedChannelMapping Mapping = BMFontRendering::ComputePackedChannelMapping(
				Asset->FontData.Common, Glyph.Channel);
			Coverage.R = FMath::Max(Coverage.R, Mapping.ChannelWeights.R);
			Coverage.G = FMath::Max(Coverage.G, Mapping.ChannelWeights.G);
			Coverage.B = FMath::Max(Coverage.B, Mapping.ChannelWeights.B);
			Coverage.A = FMath::Max(Coverage.A, Mapping.ChannelWeights.A);
			ConstantBias = FMath::Max(ConstantBias, Mapping.ConstantBias);
		}
		if (ConstantBias > 0.0f)
		{
			FCanvasTileItem SolidTile(FVector2D(DrawX, DrawY), FVector2D(DrawWidth, DrawHeight), FLinearColor::White);
			Canvas->DrawItem(SolidTile);
			return;
		}
		PackedParameters = new FBMFontPackedThumbnailParameters(Coverage);
		TileItem.BatchedElementParameters = PackedParameters;
		TileItem.BlendMode = SE_BLEND_Opaque;
	}
	Canvas->DrawItem(TileItem);
}
