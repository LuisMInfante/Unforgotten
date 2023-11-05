// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UnforgottenHUD.h"

void AUnforgottenHUD::DrawHUD() 
{
    Super::DrawHUD();

    FVector2D ViewportSize;
    if (GEngine)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        float ScaledSpread = MaxCrosshairSpread * HUDPackage.CrosshairSpread;

        if (HUDPackage.CenterCrosshair)
        {
            FVector2D Spread(0.f, 0.f); // don't spread
            DrawCrosshair(HUDPackage.CenterCrosshair, ViewportCenter, Spread);
        }
        if (HUDPackage.LeftCrosshair)
        {   
            FVector2D Spread(-ScaledSpread, 0.f); // Only spread left
            DrawCrosshair(HUDPackage.LeftCrosshair, ViewportCenter, Spread);
        }
        if (HUDPackage.RightCrosshair)
        {
            FVector2D Spread(ScaledSpread, 0.f); // Only spread right
            DrawCrosshair(HUDPackage.RightCrosshair, ViewportCenter, Spread);
        }
        if (HUDPackage.TopCrosshair)
        {
            FVector2D Spread(0.f, -ScaledSpread); // Only spread upwards
            DrawCrosshair(HUDPackage.TopCrosshair, ViewportCenter, Spread);
        }
        if (HUDPackage.BottomCrosshair)
        {
            FVector2D Spread(0.f, ScaledSpread); // Only spread downwards
            DrawCrosshair(HUDPackage.BottomCrosshair, ViewportCenter, Spread);
        }
    }
}

void AUnforgottenHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread) 
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();

    const FVector2D TextureDrawPoint(
        ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, 
        ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
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
        FLinearColor::White
    );
}
