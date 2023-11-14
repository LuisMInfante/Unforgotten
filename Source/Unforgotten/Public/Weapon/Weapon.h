// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Intial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class UNFORGOTTEN_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ShowPickupWidget(bool bShowWidget);

	virtual void Fire(const FVector& HitTarget);

	void SetHUDAmmo();

	// Crosshair Textures
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CenterCrosshair;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* LeftCrosshair;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* RightCrosshair;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* TopCrosshair;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* BottomCrosshair;

	// Auto Fire
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bIsAutomatic = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	); 

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex
	);

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* OverlapVolume;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
	int32 MagazineCapacity;

	void SpendAmmo();
	
	UPROPERTY()
	class AUnforgottenCharacter* UnforgottenOwnerCharacter;
	UPROPERTY()
	class AUnforgottenPlayerController* UnforgottenOwnerController;

public:	

	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; }
	FORCEINLINE USphereComponent* GetOverlapVolume() const { return OverlapVolume; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	bool IsEmpty();
};
