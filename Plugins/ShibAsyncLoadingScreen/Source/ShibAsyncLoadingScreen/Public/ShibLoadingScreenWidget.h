// Copyright Shiba Inu Games LLC.

#pragma once

/**
 * NOT USED ANYMORE, KEPT HERE FOR REFERENCE ONLY
*/

#include "ShibLoadingScreenConfig.h"
#include "MoviePlayer.h"
#include "Widgets/Images/SThrobber.h"

/**
 * 
 */
class SHIBASYNCLOADINGSCREEN_API ShibLoadingScreenWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(ShibLoadingScreenWidget) {}
		SLATE_ARGUMENT(int32, MapIndex)
	SLATE_END_ARGS()
	
	ShibLoadingScreenWidget();
    virtual ~ShibLoadingScreenWidget() override;
    
	void Construct(const FArguments& InArgs)
    {
	    /*
        int32 MapIndex = InArgs._MapIndex;

	    // Hide mouse cursor during loading phase
	    SetCursor(EMouseCursor::None);

	    bCanSupportFocus = false;
	    
        const UShibLoadingScreenConfig* Settings = UShibLoadingScreenConfig::Get();
        if (!Settings) return;
	    
	    // Use Contains to check if the TMap has a brush for the full object path
        const FSlateBrush* MapBrush = nullptr;
        if (Settings->MapsWithLoadingScreen[MapIndex].MapBrush.HasUObject())
        {
            // Use the map-specific loading screen brush
            MapBrush = &Settings->MapsWithLoadingScreen[MapIndex].MapBrush;
            UE_LOG(LogTemp, Log, TEXT("MapBrushTezt: Match found, using custom brush for map: %s"), *Settings->MapsWithLoadingScreen[MapIndex].MapName);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("MapBrushTezt: No custom brush found, using default brush."));
            MapBrush = &Settings->LoadingScreenBrushDefault;
        }

        // Select a random loading text once and store it
        SelectedLoadingText = Settings->MapsWithLoadingScreen[MapIndex].LoadingScreenText;

        // Initialize child slot based on whether SpriteAnimationBrush is set
        if (Settings->SpriteAnimationBrush.GetResourceObject() != nullptr)
        {
            // Initialize sprite animation variables
            //SpriteBrush.SetResourceObject(Settings->SpriteAnimationBrush.GetResourceObject());
            Columns = 4;  // Number of columns in the sprite sheet
            Rows = 3;     // Number of rows in the sprite sheet
            FrameCount = Columns * Rows;
            FrameSize = FVector2D(1.0f / Columns, 1.0f / Rows);
            CurrentFrame = 0;

            // Register the timer using the AnimationSpeed from the config
            //RegisterActiveTimer(Settings->AnimationSpeed, FWidgetActiveTimerDelegate::CreateSP(this, &ShibLoadingScreenWidget::UpdateSpriteAnimation));
            UpdateSpriteAnimation(0.f, 0.f);
            
            // Construct the UI with sprite animation
            ChildSlot
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                [
                    SNew(SImage)
                    .Image(MapBrush)
                ]
                + SOverlay::Slot()
                .VAlign(VAlign_Bottom)
                .HAlign(HAlign_Right)
                [
                    SNew(SImage)
                    .Image(&SpriteBrush)
                    .DesiredSizeOverride(FVector2D(256, 256))
                    .RenderTransform(FSlateRenderTransform(FVector2D(-50, 50)))
                    .RenderTransformPivot(FVector2D(1.0f, 1.0f))
                ]
                + SOverlay::Slot()
                .VAlign(VAlign_Bottom)
                .HAlign(HAlign_Left)
                .Padding(FMargin(100, 0, 0, 80))
                [
                    SNew(STextBlock)
                    .Text(this, &ShibLoadingScreenWidget::GetSelectedLoadingText)
                    .ColorAndOpacity(FColor(255, 122, 34))
                    .Font(Settings->LoadingScreenFont)
                    .Justification(ETextJustify::Center)
                    .RenderTransformPivot(FVector2D(0.f, 0.f))  // Pivot at (0.5, 1)
                    .Visibility(this, &ShibLoadingScreenWidget::GetMessageIndicatorVisibility)
                ]
            ];
        }
        else
        {
            // Construct the UI with throbber
            ChildSlot
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                [
                    SNew(SImage).Image(MapBrush)
                ]
                + SOverlay::Slot()
                .VAlign(VAlign_Bottom)
                .HAlign(HAlign_Left)
                .Padding(FMargin(0, 0, 0, 50)) 
                [
                    SNew(SThrobber)
                    .Visibility(this, &ShibLoadingScreenWidget::GetLoadIndicatorVisibility)
                ]
                + SOverlay::Slot()
                .VAlign(VAlign_Bottom)
                .HAlign(HAlign_Center)
                .Padding(FMargin(100, 0, 0, 80))
                [
                    SNew(STextBlock)
                    .Text(this, &ShibLoadingScreenWidget::GetSelectedLoadingText)
                    .ColorAndOpacity(FColor(255, 122, 34))
                    .Font(Settings->LoadingScreenFont)
                    .Justification(ETextJustify::Center)
                    .RenderTransformPivot(FVector2D(0.f, 0.f))  // Pivot at (0.5, 1)
                    .Visibility(this, &ShibLoadingScreenWidget::GetMessageIndicatorVisibility)
                ]
            ];
        }*/
    }

private:
    FSlateBrush SpriteBrush;
    FVector2D CurrentUV;
    FVector2D FrameSize;
    int32 FrameCount;
    int32 CurrentFrame;
    int32 Columns;
    int32 Rows;
    FText SelectedLoadingText;
    
    EVisibility GetLoadIndicatorVisibility() const
    {
        return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Collapsed : EVisibility::Visible;
    }

    EVisibility GetMessageIndicatorVisibility() const
    {
        return EVisibility::Visible;
    }
    
    FText GetSelectedLoadingText() const
    {
        return SelectedLoadingText;
    }

    // Called every tick to update the animation
    EActiveTimerReturnType UpdateSpriteAnimation(double InCurrentTime, float InDeltaTime)
    {
        // Update UV based on current frame
        CurrentFrame = (CurrentFrame + 1) <= FrameCount ? CurrentFrame + 1 : 1;
        int32 ColumnIndex = CurrentFrame % Columns != 0 ? CurrentFrame % Columns : Columns;
        int32 RowIndex = ceil(CurrentFrame / Columns);
        CurrentUV = FVector2D(ColumnIndex * FrameSize.X, RowIndex * FrameSize.Y);

        // Update the brush's UVRegion
        SpriteBrush.SetUVRegion(FBox2D(CurrentUV, CurrentUV + FrameSize));

        if (GetMoviePlayer()->IsLoadingFinished())
        {
            return EActiveTimerReturnType::Stop;
        }

        return EActiveTimerReturnType::Continue;
    }
};

