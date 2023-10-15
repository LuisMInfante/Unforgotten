// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
// ******* CHANGE THIS INCLUDE LATER *********
#include "UnforgottenCharacter.h"

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

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	AUnforgottenCharacter* UnforgottenCharacter = Cast<AUnforgottenCharacter>(OtherActor);
	if(UnforgottenCharacter && PickupWidget)
	{
		UnforgottenCharacter->SetOverlappingWeapon(this);
		PickupWidget->SetVisibility(true);
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