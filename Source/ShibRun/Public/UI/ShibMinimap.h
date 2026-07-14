// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShibMinimap.generated.h"

class UCanvasPanel;
class AShibGameState;

UCLASS()
class SHIBRUN_API UShibMinimap : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Minimap")
	FName OriginActorTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap|Player")
	TSubclassOf<UUserWidget> PlayerWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap|Player")
	FVector2D PlayerWidgetSize;
	
	UPROPERTY(EditAnywhere, Category = "Minimap|Position")
	FVector2D WorldSize;
	
	UPROPERTY(EditAnywhere, Category = "Minimap|Position")
	FVector2D MinimapSize;
	
	UPROPERTY(EditAnywhere, Category = "Minimap|Position")
	FVector2D MinimapPositionAdjustment;
	
	UPROPERTY(BlueprintReadWrite, Category = "Minimap")
	TObjectPtr<UCanvasPanel> ParentCanvas;

	UPROPERTY(BlueprintReadWrite, Category = "Minimap")
	TMap<TObjectPtr<APlayerState>, TObjectPtr<UUserWidget>> MinimapPlayers;

	UPROPERTY()
	TObjectPtr<AActor> OriginActor;
	
	UPROPERTY()
	TObjectPtr<AShibGameState> ShibGS;

	UFUNCTION(BlueprintCallable)
	void GetOriginActor();
	
	UFUNCTION(BlueprintCallable)
	void CreatePlayerWidgets();

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerPositions();

	UFUNCTION(BlueprintCallable)
	void ValidateMinimapPlayers();

	UFUNCTION(BlueprintCallable)
	void AdjustMinimapSettings(FVector2D NewPlayerWidgetSize = FVector2D::ZeroVector, FVector2D NewWorldSize = FVector2D::ZeroVector, FVector2D NewMinimapSize = FVector2D::ZeroVector, FVector2D NewMinimapPosition = FVector2D::ZeroVector);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Minimap")
	void AddPlayerWidget(APlayerState* PlayerToAdd);
	
private:
	TObjectPtr<AShibGameState> GetShibGS();
	void RemovePlayerWidget(APlayerState* PlayerToRemove);
};
