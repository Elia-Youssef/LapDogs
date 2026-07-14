// Copyright Shiba Inu Games LLC.


#include "UI/ShibMinimap.h"

#include "Character/ShibCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Game/ShibGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UShibMinimap::GetOriginActor()
{
	if (OriginActorTag.IsNone()) return;
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithTag(this, OriginActorTag, Actors);
	
	if (Actors.IsEmpty()) return;
	
	OriginActor = Actors[0];
}

void UShibMinimap::CreatePlayerWidgets()
{
	if (!GetShibGS()) return;

	for (APlayerState* PS : ShibGS->PlayerArray)
	{
		if (MinimapPlayers.Contains(PS)) continue;
		
		if (!Cast<AShibCharacter>(PS->GetPawn())) continue; // make sure this player state possess a valid shibcharacter pawn

		AddPlayerWidget(PS);
	}
}

void UShibMinimap::UpdatePlayerPositions()
{
	if (!GetShibGS() || !OriginActor) return;

	TArray<TObjectPtr<APlayerState>> MinimapStates;
	MinimapPlayers.GetKeys(MinimapStates);

	TObjectPtr<UUserWidget> WidgetRef;
	
	// loop through minimap players to update their positions
	// Check if the player state exist and is valid at the same time
	for (auto PS : MinimapStates)
	{
		if (!IsValid(PS))
		{
			RemovePlayerWidget(PS);
			continue;
		}
		
		if (!ShibGS->PlayerArray.Contains(PS))
		{
			RemovePlayerWidget(PS);
			continue;
		}
		
		WidgetRef = MinimapPlayers.FindRef(PS);
		if (!WidgetRef)
		{
			RemovePlayerWidget(PS);
			continue;
		}
		
		if (IsValid(PS->GetPawn()))
		{
			FVector RelativePosition = OriginActor->GetActorLocation() - PS->GetPawn()->GetActorLocation();

			FVector2D WorldHalfSize = WorldSize / 2;
			FVector2D MinimapHalfSize = MinimapSize / 2;
			float MappedX = FMath::GetMappedRangeValueClamped(
				FVector2D{-WorldHalfSize.X, WorldHalfSize.X},
				FVector2D{-MinimapHalfSize.X, MinimapHalfSize.X},
				RelativePosition.X
			);
			float MappedY = FMath::GetMappedRangeValueClamped(
				FVector2D{-WorldHalfSize.Y, WorldHalfSize.Y},
				FVector2D{-MinimapHalfSize.Y, MinimapHalfSize.Y},
				RelativePosition.Y
			);
	
			FVector2D NewLocation;
			NewLocation.X = FMath::Clamp(MappedX * MinimapPositionAdjustment.X, -MinimapHalfSize.X, MinimapHalfSize.X);
			NewLocation.Y = FMath::Clamp(MappedY * MinimapPositionAdjustment.Y, -MinimapHalfSize.Y, MinimapHalfSize.Y);

			WidgetRef->SetRenderTranslation(NewLocation);
		}
		else
		{
			RemovePlayerWidget(PS);
		}
	}
}

void UShibMinimap::ValidateMinimapPlayers()
{
	if (!GetShibGS()) return;

	TArray<TObjectPtr<APlayerState>> MinimapPlayersList;
	MinimapPlayers.GetKeys(MinimapPlayersList);
	for (auto PlayerToValidate : ShibGS->PlayerArray)
	{
		if (!PlayerToValidate) continue;

		if (!MinimapPlayersList.Contains(PlayerToValidate) && Cast<AShibCharacter>(PlayerToValidate->GetPawn()))
		{
			AddPlayerWidget(PlayerToValidate);
		}
	}	
}

void UShibMinimap::AdjustMinimapSettings(FVector2D NewPlayerWidgetSize, FVector2D NewWorldSize,
	FVector2D NewMinimapSize, FVector2D NewMinimapPosition)
{
	if(NewPlayerWidgetSize != FVector2D::ZeroVector)
	{
		PlayerWidgetSize = NewPlayerWidgetSize;
	}

	if(NewWorldSize != FVector2D::ZeroVector)
	{
		WorldSize = NewWorldSize;
	}

	if(NewMinimapSize != FVector2D::ZeroVector)
	{
		MinimapSize = NewMinimapSize;
	}

	if(NewMinimapPosition != FVector2D::ZeroVector)
	{
		MinimapPositionAdjustment = NewMinimapPosition;
	}
}

TObjectPtr<AShibGameState> UShibMinimap::GetShibGS()
{
	if (!ShibGS) ShibGS = Cast<AShibGameState>(UGameplayStatics::GetGameState(this));
	return ShibGS;
}

void UShibMinimap::AddPlayerWidget_Implementation(APlayerState* PlayerToAdd)
{
	UUserWidget* Widget = CreateWidget(GetOwningPlayer(), PlayerWidget);
	auto* CanvasSlot = Cast<UCanvasPanelSlot>(ParentCanvas->AddChild(Widget));
	CanvasSlot->SetSize(PlayerWidgetSize);
	CanvasSlot->SetAnchors(FAnchors{.5f, .5f, .5f, .5f});
	CanvasSlot->SetPosition(FVector2D{0.f, 0.f});
	CanvasSlot->SetAlignment(FVector2D{.5f, .5f});
		
	MinimapPlayers.Add(PlayerToAdd, Widget);
}

void UShibMinimap::RemovePlayerWidget(APlayerState* PlayerToRemove)
{
	if (TObjectPtr<UUserWidget> Widget = MinimapPlayers.FindRef(PlayerToRemove); Widget)
	{
		Widget->RemoveFromParent();
		MinimapPlayers.Remove(PlayerToRemove);
	}
}
