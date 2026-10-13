// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

#include "AssetEditor/SBMFontAtlasPreview.h"

#include "BMFontAsset.h"
#include "BMFontRendering.h"
#include "Engine/Texture2D.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"

void SBMFontAtlasPreview::Construct(const FArguments& InArgs)
{
	FontAsset = InArgs._FontAsset;
	PageId = InArgs._PageId;
	OnGlyphSelected = InArgs._OnGlyphSelected;

	SetClipping(EWidgetClipping::ClipToBounds);
}

void SBMFontAtlasPreview::SetPageId(const int32 InPageId)
{
	if (PageId != InPageId)
	{
		PageId = InPageId;
		CachedPageTexture.Reset();
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void SBMFontAtlasPreview::SetSelectedCodepoint(const int32 InCodepoint)
{
	if (SelectedCodepoint != InCodepoint)
	{
		SelectedCodepoint = InCodepoint;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

int32 SBMFontAtlasPreview::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	const bool bParentEnabled) const
{
	const FSlateBrush* BackgroundBrush = FAppStyle::GetBrush(TEXT("Brushes.Secondary"));
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		BackgroundBrush,
		bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect,
		InWidgetStyle.GetColorAndOpacityTint() * BackgroundBrush->GetTint(InWidgetStyle)
	);
	++LayerId;

	UBMFontAsset* Font = FontAsset.Get();
	if (Font == nullptr)
	{
		return LayerId;
	}

	UTexture2D* PageTexture = Font->GetPageTexture(PageId);
	if (PageTexture == nullptr || PageTexture->GetResource() == nullptr)
	{
		return LayerId;
	}

	if (CachedPageTexture.Get() != PageTexture
		|| CachedDataRevision != Font->GetDataRevision()
		|| CachedRenderResourceRevision != Font->GetRenderResourceRevision())
	{
		PageBrushes.Reset();
		TArray<int32, TInlineAllocator<4>> PreviewChannels;
		if (Font->FontData.Common.bPacked)
		{
			// Composite the coverage channels used on this page. Raw RGBA sampling
			// hides RGB-packed glyphs when the atlas alpha channel is zero.
			TArray<FBMFontPackedChannelMapping, TInlineAllocator<4>> Mappings;
			for (const TPair<int32, FBMFontGlyph>& Entry : Font->FontData.Glyphs)
			{
				const FBMFontGlyph& Glyph = Entry.Value;
				if (Glyph.Page != PageId || Glyph.Width <= 0 || Glyph.Height <= 0)
				{
					continue;
				}
				const FBMFontPackedChannelMapping Mapping = BMFontRendering::ComputePackedChannelMapping(
					Font->FontData.Common, Glyph.Channel);
				if (!Mappings.ContainsByPredicate([&Mapping](const FBMFontPackedChannelMapping& Existing)
					{ return Existing.ChannelWeights == Mapping.ChannelWeights && Existing.ConstantBias == Mapping.ConstantBias; }))
				{
					Mappings.Add(Mapping);
					PreviewChannels.Add(Glyph.Channel);
				}
			}
			PreviewChannels.Sort();
		}
		if (PreviewChannels.IsEmpty())
		{
			PreviewChannels.Add(15);
		}
		for (const int32 Channel : PreviewChannels)
		{
			FSlateBrush& Brush = PageBrushes.AddDefaulted_GetRef();
			Brush.DrawAs = ESlateBrushDrawType::Image;
			Brush.SetResourceObject(Font->GetPageRenderResource(PageId, Channel));
			Brush.SetImageSize(FVector2D(PageTexture->GetSizeX(), PageTexture->GetSizeY()));
		}
		CachedPageTexture = PageTexture;
		CachedDataRevision = Font->GetDataRevision();
		CachedRenderResourceRevision = Font->GetRenderResourceRevision();
	}

	float ViewScale = 1.0f;
	FVector2D ViewOffset = FVector2D::ZeroVector;
	ComputeViewTransform(AllottedGeometry.GetLocalSize(), ViewScale, ViewOffset);
	if (ViewScale <= 0.0f)
	{
		return LayerId;
	}

	for (const FSlateBrush& Brush : PageBrushes)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(PageTexture->GetSizeX(), PageTexture->GetSizeY()) * ViewScale,
				FSlateLayoutTransform(ViewOffset)
			),
			&Brush,
			bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect,
			InWidgetStyle.GetColorAndOpacityTint()
		);
	}

	const FLinearColor GlyphLineColor(1.0f, 1.0f, 1.0f, 0.25f);
	const FLinearColor SelectedLineColor(0.35f, 0.65f, 1.0f, 1.0f);
	for (const TPair<int32, FBMFontGlyph>& Entry : Font->FontData.Glyphs)
	{
		const FBMFontGlyph& Glyph = Entry.Value;
		if (Glyph.Page != PageId || Glyph.Width <= 0 || Glyph.Height <= 0)
		{
			continue;
		}

		const bool bSelected = Entry.Key == SelectedCodepoint;
		const FLinearColor LineColor = bSelected ? SelectedLineColor : GlyphLineColor;

		const FVector2D TopLeft = ViewOffset + FVector2D(Glyph.X, Glyph.Y) * ViewScale;
		const FVector2D BottomRight = ViewOffset + FVector2D(Glyph.X + Glyph.Width, Glyph.Y + Glyph.Height) * ViewScale;

		TArray<FVector2D> Corners;
		Corners.Reserve(5);
		Corners.Add(TopLeft);
		Corners.Add(FVector2D(BottomRight.X, TopLeft.Y));
		Corners.Add(BottomRight);
		Corners.Add(FVector2D(TopLeft.X, BottomRight.Y));
		Corners.Add(TopLeft);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			Corners,
			bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect,
			LineColor,
			false,
			bSelected ? 2.0f : 1.0f
		);
	}
	++LayerId;

	return LayerId;
}

FReply SBMFontAtlasPreview::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const UBMFontAsset* Font = FontAsset.Get();
	UTexture2D* PageTexture = Font != nullptr ? Font->GetPageTexture(PageId) : nullptr;
	if (PageTexture == nullptr)
	{
		return FReply::Unhandled();
	}

	float ViewScale = 1.0f;
	FVector2D ViewOffset = FVector2D::ZeroVector;
	ComputeViewTransform(MyGeometry.GetLocalSize(), ViewScale, ViewOffset);
	if (ViewScale <= 0.0f)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FVector2D AtlasPosition = (LocalPosition - ViewOffset) / ViewScale;

	for (const TPair<int32, FBMFontGlyph>& Entry : Font->FontData.Glyphs)
	{
		const FBMFontGlyph& Glyph = Entry.Value;
		if (Glyph.Page != PageId)
		{
			continue;
		}
		if (AtlasPosition.X >= Glyph.X && AtlasPosition.X < Glyph.X + Glyph.Width
			&& AtlasPosition.Y >= Glyph.Y && AtlasPosition.Y < Glyph.Y + Glyph.Height)
		{
			OnGlyphSelected.ExecuteIfBound(Entry.Key);
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

void SBMFontAtlasPreview::ComputeViewTransform(const FVector2D& LocalSize, float& OutScale, FVector2D& OutOffset) const
{
	OutScale = 0.0f;
	OutOffset = FVector2D::ZeroVector;

	const UBMFontAsset* Font = FontAsset.Get();
	UTexture2D* PageTexture = Font != nullptr ? Font->GetPageTexture(PageId) : nullptr;
	if (PageTexture == nullptr || PageTexture->GetSizeX() <= 0 || PageTexture->GetSizeY() <= 0)
	{
		return;
	}

	const float ScaleX = LocalSize.X / static_cast<float>(PageTexture->GetSizeX());
	const float ScaleY = LocalSize.Y / static_cast<float>(PageTexture->GetSizeY());
	OutScale = FMath::Min(ScaleX, ScaleY);
	OutOffset = FVector2D(
		(LocalSize.X - PageTexture->GetSizeX() * OutScale) * 0.5,
		(LocalSize.Y - PageTexture->GetSizeY() * OutScale) * 0.5
	);
}
