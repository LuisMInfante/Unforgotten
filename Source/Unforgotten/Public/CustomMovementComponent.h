// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WallRunSide.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "CustomMovementComponent.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

#pragma region Defaults
private:
	// The ground speed when running
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Grounded Movement", Meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 400.0f; //300
	// The ground speed when sprinting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Grounded Movement", Meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 800.0f;
	// The acceleration when running
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Grounded Movement", Meta = (AllowPrivateAccess = "true"))
	float RunAcceleration = 2000.0f;
	// The acceleration when sprinting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Grounded Movement", Meta = (AllowPrivateAccess = "true"))
	float SprintAcceleration = 2000.0f;
	// The amount of vertical room between the two line traces when checking if the character is still on the wall
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Wall Running Movement", Meta = (AllowPrivateAccess = "true"))
	float LineTraceVerticalTolerance = 50.0f;
	// The player's velocity while wall running
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Wall Running Movement", Meta = (AllowPrivateAccess = "true"))
	float WallRunSpeed = 625.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Wall Running Movement", Meta = (AllowPrivateAccess = "true"))
	float WallRunJump = JumpZVelocity / 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Wall Running Movement", Meta = (AllowPrivateAccess = "true"))
	bool bWallStick = false;
#pragma endregion

#pragma region Sprinting Functions
public:
	// Sets sprinting to either enabled or disabled
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void SetSprinting(bool Sprinting);
#pragma endregion

#pragma region Wall Running Functions
	// Requests that the character begins wall running. Will return false if the required keys are not being pressed
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	bool BeginWallRun();
	// Ends the character's wall run
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void EndWallRun();
	// Returns true if the required wall run keys are currently down
	void GetKeyDown();
	// Returns true if the player is next to a wall that can be wall ran
	bool IsNextToWall(float VerticalTolerance = 0.0f);
	// Finds the wall run direction and side based on the specified surface normal
	void FindWallRunDirectionAndSide(const FVector &SurfaceNormal, FVector &Direction, bool &Side) const;
	// Helper function that returns true if the specified surface normal can be wall ran on
	bool CanSurfaceBeWallRan(const FVector &SurfaceNormal) const;
	// Returns true if the movement mode is custom and matches the provided custom movement mode
	bool IsCustomMovementMode(uint8 CustomMovementMode) const;

	void CrouchUpdate(bool bClientSimulation);
	void Crouch(bool bClientSimulation);
	void UnCrouch(bool bClientSimulation);

	void PhysSlide(float deltaTime, int32 Iterations);
	void EnterSlide();
	void ExitSlide();

private:
	// Called when the owning actor hits something (to begin the wall run)
	UFUNCTION()
	void OnActorHit(AActor *SelfActor, AActor *OtherActor, FVector NormalImpulse, const FHitResult &Hit);

	FHitResult CheckSurrounding(bool bForward, bool bRight);
	
#pragma endregion
	// Perfoms the jump
	bool DoJump(bool bReplayingMoves);
	// Called when jump reached its peak
	void NotifyJumpApex();

#pragma region Overrides
protected:
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	void PhysWallRunning(float DeltaTime, int32 Iterations);
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual void ProcessLanded(const FHitResult &Hit, float RemainingTime, int32 Iterations) override;
#pragma endregion

#pragma region Compressed Flags
private:
	uint8 WantsToSprint : 1;
	uint8 WallRunKeysDown : 1;
#pragma endregion

#pragma region Private Variables
	// True if the sprint key is down
	bool bSprintHeld = false;
	// The direction the character is currently wall running in
	FVector WallRunDirection;
	// The normal of the wall
	FVector WallRunNormal;
	// The side of the wall the player is running on.
	bool bWallRunRight;
	// If the wall run is held
	bool bWallRunHeld = false;
	bool bCrouchHeld = false;
#pragma endregion
};