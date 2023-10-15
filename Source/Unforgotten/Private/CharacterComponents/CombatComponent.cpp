// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "UnforgottenCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UCombatComponent::EquipWeapon(class AWeapon* WeaponToEquip) 
{
	if(!Character || !WeaponToEquip) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	// const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh1P()->GetSocketByName(FName("RightHandSocket"));

	if(RightHandSocket)
	{
		// RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh1P());
		Character->SetHasRifle(true);
	}

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->ShowPickupWidget(false);
}
