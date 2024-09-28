// Fill out your copyright notice in the Description page of Project Settings.


#include "PreGameOverlay.h"

void UPreGameOverlay::SetReadyText(bool State)
{
	if (State)
	{
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Ready!"))));
		FSlateColor NewColor(FColor::Green);
		PlayerReadyText->SetColorAndOpacity(NewColor);
	}
	else
	{
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Not Ready!"))));
		FSlateColor NewColor(FColor::Red);
		PlayerReadyText->SetColorAndOpacity(NewColor);
	}
}

void UPreGameOverlay::SetTeamText(ETeamID TeamId)
{
	switch (TeamId)
	{
	case ETeamID::Spectator:
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Spectator"))));
		PlayerReadyText->SetColorAndOpacity(FColor::White);
		break;
	case ETeamID::Red:
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Red"))));
		PlayerReadyText->SetColorAndOpacity(FColor::Red);
		break;
	case ETeamID::Blue:
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Blue"))));
		PlayerReadyText->SetColorAndOpacity(FColor::Blue);
		break;
	case ETeamID::Green:
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Green"))));
		PlayerReadyText->SetColorAndOpacity(FColor::Green);
		break;
	case ETeamID::Yello:
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Yellow"))));
		PlayerReadyText->SetColorAndOpacity(FColor::Yellow);
		break;
	case ETeamID::NoTeamId:
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Not assigned"))));
		PlayerReadyText->SetColorAndOpacity(FColor::Magenta);
		break;
	default:
		PlayerReadyText->SetText(FText::FromString(FString::Printf(TEXT("Not assigned"))));
		PlayerReadyText->SetColorAndOpacity(FColor::Magenta);
		break;
	}
}
