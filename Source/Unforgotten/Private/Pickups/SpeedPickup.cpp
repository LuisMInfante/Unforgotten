// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SpeedPickup.h"
#include "UnforgottenCharacter.h"
#include "Unforgotten/Public/CharacterComponents/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) 
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AUnforgottenCharacter* UnforgottenCharacter = Cast<AUnforgottenCharacter>(OtherActor);

    if(UnforgottenCharacter)
    {
        UBuffComponent* Buff = UnforgottenCharacter->GetBuff();

        if (Buff)
        {
            Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
        }
    }
    
    Destroy();
}