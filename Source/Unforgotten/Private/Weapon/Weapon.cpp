// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
// ******* CHANGE THIS INCLUDE LATER *********
#include "UnforgottenCharacter.h"

#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Unforgotten/Public/UnforgottenPlayerController.h"
#include "CharacterComponents/CombatComponent.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	OverlapVolume = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapVolume"));
	OverlapVolume->SetupAttachment(RootComponent);
	OverlapVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	OverlapVolume->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OverlapVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	OverlapVolume->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UnforgottenOwnerCharacter)
	{
		UnforgottenOwnerCharacter->GetCombatComponent()->SetHUDCrosshairs(DeltaTime);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	AUnforgottenCharacter* UnforgottenCharacter = Cast<AUnforgottenCharacter>(OtherActor);
	if(UnforgottenCharacter && PickupWidget)
	{	
		if(UnforgottenCharacter->GetHasRifle() == false)
		{
			UnforgottenCharacter->SetOverlappingWeapon(this);
			PickupWidget->SetVisibility(true);
		}
		else
		{
			UnforgottenCharacter->SetOverlappingWeapon(nullptr);
			PickupWidget->SetVisibility(false);
		}
	}
	
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) 
{
	AUnforgottenCharacter* UnforgottenCharacter = Cast<AUnforgottenCharacter>(OtherActor);
	if(UnforgottenCharacter && PickupWidget)
	{
		UnforgottenCharacter->SetOverlappingWeapon(nullptr);
		PickupWidget->SetVisibility(false);
	}
}


void AWeapon::ShowPickupWidget(bool bShowWidget) 
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}


void AWeapon::Fire(const FVector& HitTarget) 
{
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if(CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName(TEXT("AmmoEject")));

		if(AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());

			UWorld* World = GetWorld();

			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}

	SpendAmmo();
}


void AWeapon::SpendAmmo() 
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagazineCapacity);
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo() 
{
	UnforgottenOwnerCharacter = !UnforgottenOwnerCharacter ? Cast<AUnforgottenCharacter>(GetOwner()) : UnforgottenOwnerCharacter;

	if (UnforgottenOwnerCharacter)
	{
		UnforgottenOwnerController = !UnforgottenOwnerController ? Cast<AUnforgottenPlayerController>(UnforgottenOwnerCharacter->Controller) : UnforgottenOwnerController;

		if (UnforgottenOwnerController)
		{
			UnforgottenOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::AddAmmo(int32 AmmoToAdd) 
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagazineCapacity);
}

bool AWeapon::IsEmpty() 
{
	return Ammo <= 0;
}

void AWeapon::EnableCustomDepth(bool bEnable) 
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// Weapon States
