// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairOverlay.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include <Components/CanvasPanel.h>

void UCrosshairOverlay::ShowNormalOverlay()
{
	if(NormalCrosshair)
		NormalCrosshair->SetVisibility(ESlateVisibility::Visible);
}

void UCrosshairOverlay::HideNormalOverlay()
{
	if (NormalCrosshair)
		NormalCrosshair->SetVisibility(ESlateVisibility::Hidden);
}

void UCrosshairOverlay::ShowEnemyHitOverlay(bool bShow)
{
	if (!EnemyHitCrosshair)return;
	if (bShow)
		EnemyHitCrosshair->SetVisibility(ESlateVisibility::Visible);
	else
		EnemyHitCrosshair->SetVisibility(ESlateVisibility::Hidden);
}

void UCrosshairOverlay::ShowDragonHitOverlay(bool bShow)
{
	if (!DragonHitCrosshair)return;
	if (bShow)
		DragonHitCrosshair->SetVisibility(ESlateVisibility::Visible);
	else
		DragonHitCrosshair->SetVisibility(ESlateVisibility::Hidden);
}

void UCrosshairOverlay::ShowDragonOverlay(bool bShow)
{
	if (!DragonCrosshair)return;
	if (bShow)
		DragonCrosshair->SetVisibility(ESlateVisibility::Visible);
	else
		DragonCrosshair->SetVisibility(ESlateVisibility::Hidden);
}

void UCrosshairOverlay::AdjustDragonCrosshair(FVector2D NewPos)
{
    if (!GEngine || !GEngine->GameViewport) return;

    FVector2D ScreenSize;
    GEngine->GameViewport->GetViewportSize(ScreenSize);

    FVector2D ScreenPosition;

    // Convert screen positions to relative positions
    FVector2D RelativePosition = NewPos / ScreenSize;

    // Get the size of the Canvas
    FVector2D CanvasSize = GetCanvasSize();

    // Convert relative positions to absolute positions based on Canvas size
    FVector2D AbsolutePosition = RelativePosition * CanvasSize;

    // Set positions
    if (DragonCrosshair)
    {
        UCanvasPanelSlot* DragonCanvasSlot = Cast<UCanvasPanelSlot>(DragonCrosshair->Slot);
        if (DragonCanvasSlot)
        {
            DragonCanvasSlot->SetPosition(AbsolutePosition);
        }
    }

    if (DragonHitCrosshair)
    {
        UCanvasPanelSlot* DragonHitCanvasSlot = Cast<UCanvasPanelSlot>(DragonHitCrosshair->Slot);
        if (DragonHitCanvasSlot)
        {
            DragonHitCanvasSlot->SetPosition(AbsolutePosition);
        }
    }
}

FVector2D UCrosshairOverlay::GetCanvasSize()
{
	if (UCanvasPanel* CanvasPanel = Cast<UCanvasPanel>(GetRootWidget()))
	{
		FVector2D CanvasSize = CanvasPanel->GetCachedGeometry().GetLocalSize();
		return CanvasSize;
	}
	return FVector2D(0, 0);
}