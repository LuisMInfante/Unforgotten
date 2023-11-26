// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/BuffComponent.h"
#include "UnforgottenCharacter.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealingRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealTime) 
{
	bHealing = true;
	HealingRate = HealAmount / HealTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealingRampUp(float DeltaTime) 
{
	if (!bHealing || !Character) return;

	const float HealingPerFrame = HealingRate * DeltaTime;
	Character->SetCurrentHealth(FMath::Clamp(Character->GetCurrentHealth() + HealingPerFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealingPerFrame;

	if (AmountToHeal <= 0.f || Character->GetCurrentHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

