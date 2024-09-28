// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterHUD.h"
#include "CharacterOverlay.h"
#include "PreGameOverlay.h"
#include "CrosshairOverlay.h"
#include "GameFramework/PlayerController.h"

//deprecated
void ACharacterHUD::DrawHUD()
{
	Super::DrawHUD();
	/*FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		if (CrosshairsCenter)
		{
			DrawCrosshair(CrosshairsCenter, ViewportCenter, CrosshairsColor);
		}
	}*/
}

void ACharacterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f),
		ViewportCenter.Y - (TextureHeight / 2.f)
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor, CrosshairsCenterBlendMode,1.f
	);
}

void ACharacterHUD::DrawCorsshairWidget()
{
	CrosshairOverlay->ShowNormalOverlay();
}

void ACharacterHUD::DrawCorsshairEnemyHit(bool bDraw)
{
	CrosshairOverlay->ShowEnemyHitOverlay(bDraw);
}

void ACharacterHUD::DrawCorsshairDragonEnemyHit(bool bDraw)
{
	CrosshairOverlay->ShowDragonHitOverlay(bDraw);
}


void ACharacterHUD::OnMatchStart()
{
	PreGameOverlay->RemoveFromViewport();
	CharacterOverlay->AddToViewport();
}

void ACharacterHUD::BeginPlay()
{
	Super::BeginPlay();
	ULocalPlayer* Localplayer = GetWorld()->GetFirstLocalPlayerFromController();
	APlayerController* Controller = Localplayer->GetPlayerController(GetWorld());
	if (Controller)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(Controller, OverlayClass);
		//CharacterOverlay->AddToViewport();
		PreGameOverlay = CreateWidget<UPreGameOverlay>(Controller, PreGameOverlayClass);
		CrosshairOverlay = CreateWidget<UCrosshairOverlay>(Controller, CrosshairOverlayClass);
		PreGameOverlay->AddToViewport();
		CrosshairOverlay->AddToViewport();
		DrawCorsshairEnemyHit(false);
		CrosshairOverlay->HideNormalOverlay();
		CrosshairOverlay->ShowDragonOverlay(false);
		CrosshairOverlay->ShowDragonHitOverlay(false);
		PreGameOverlay->SetReadyText(false);
	}
	CrosshairsColor = FLinearColor::White;

	UE_LOG(LogTemp, Warning, TEXT("HUD BeginPlay"));
}

