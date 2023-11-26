// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/HealthPickup.h"
#include "Unforgotten/Public/UnforgottenCharacter.h"
#include "Unforgotten/Public/CharacterComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{

}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
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

        if (Buff && UnforgottenCharacter->GetCurrentHealth() < UnforgottenCharacter->GetMaxHealth())
        {
            Buff->Heal(HealAmount, HealTime);
            Destroy();
        }
    }
    
}
