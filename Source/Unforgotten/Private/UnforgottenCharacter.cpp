// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnforgottenCharacter.h"
#include "UnforgottenProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AUnforgottenCharacter

AUnforgottenCharacter::AUnforgottenCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;

	GetCharacterMovement()->AirControl = 1.0f; // 100% air control

	// Enable tick every frame
	PrimaryActorTick.bCanEverTick = true;

	WallNormal = FVector::ZeroVector;
	bIsWallRun = false;
	bWallRunStick = false;
	bWallRunCD = false;

	DefaultGravity = GetCharacterMovement()->GravityScale;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.0f, 0.0f, 60.0f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.0f, 0.0f, -150.0f));

}

void AUnforgottenCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

// Called every frame
void AUnforgottenCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
	
	if(!bWallRunCD)
		WallRunUpdate();
	
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("WALLLRUN: %d WALLRUNCD: %d"), bIsWallRun, bWallRunCD));
}

//////////////////////////////////////////////////////////////////////////// Input

void AUnforgottenCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AUnforgottenCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AUnforgottenCharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnforgottenCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUnforgottenCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AUnforgottenCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);

		// const FString s = FString("F: {0}, R: {1}");
		// const FString s_f = FString::Format(*s, {MovementVector.Y, MovementVector.X});
		GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("F: %f, R: %f"), MovementVector.Y, MovementVector.X));
	}
}

void AUnforgottenCharacter::WallRun()
{
	// WallRun_Implementation();
}

bool AUnforgottenCharacter::WallRun_Implementation(FVector Endpoint, float WallRunDirection)
{
	FVector PlayerLocation = GetActorLocation();

	FHitResult HitResult;

	bool bHitWall = GetWorld()->LineTraceSingleByChannel(HitResult, PlayerLocation, Endpoint, ECC_Visibility);

	if(bHitWall)
		DrawDebugLine(GetWorld(), PlayerLocation, Endpoint, FColor::Yellow, true, 1.0f);

	if(!bHitWall) return false; // If therew as no hit then return

	WallNormal = HitResult.Normal;

	if(WallNormal.Z < -0.52 || WallNormal.Z > 0.52) return false; // Greater than our threshold then dont wall run

	if(!GetCharacterMovement()->IsFalling()) return false;

	bIsWallRun = true;

	// CameraTilt(15.0f * WallRunDirection);

	FVector PlayerToWall = (PlayerLocation - WallNormal) * WallNormal;

	LaunchCharacter(PlayerToWall, false, false); // Stick to wall

	float WallRunSpeed = 850.0f; // Wall run speed static for now (Determine how we want to gauge it)

	FVector ForwardDirection = FVector::CrossProduct(WallNormal, FVector(0, 0, 1)) * (WallRunSpeed * WallRunDirection);

	LaunchCharacter(ForwardDirection, true, bWallRunStick); // Move forward

	return true;
}

void AUnforgottenCharacter::WallRunUpdate()
{
	FVector PlayerLocation = GetActorLocation();
	FVector ForwardDirection = GetActorForwardVector() * -35.0f;
	FVector RightDirection = GetActorRightVector() * 75.0f;

	FVector RightEndpoint = PlayerLocation + ForwardDirection + RightDirection;
	FVector LeftEndpoint = PlayerLocation + ForwardDirection + -RightDirection;
	
	bool bRight = WallRun_Implementation(RightEndpoint, -1.0f);
	bool bLeft = WallRun_Implementation(LeftEndpoint, 1.0f);

	if(bRight || bLeft)
		GetCharacterMovement()->GravityScale = FMath::Lerp(GetCharacterMovement()->GravityScale, 0.25, GetWorld()->GetDeltaSeconds());
	else if(bIsWallRun)
		StopWallRun(1.0f);
}

void AUnforgottenCharacter::StopWallRun(float CDTimer)
{
	bIsWallRun = false;
	// CameraTilt(0.0f);
	GetCharacterMovement()->GravityScale = DefaultGravity;
	bWallRunCD = true;

	if(!GetWorld()->GetTimerManager().IsTimerActive(WallRunHandle))
		GetWorld()->GetTimerManager().SetTimer(WallRunHandle, FTimerDelegate::CreateLambda([this]() {bWallRunCD = false;}), CDTimer, false);
}

void AUnforgottenCharacter::CameraTilt(float TargetRoll)
{
	AController *Camera = GetController();
	FRotator CameraRotation = Camera->GetControlRotation();

	Camera->SetControlRotation(FMath::RInterpTo(CameraRotation, FRotator(TargetRoll, CameraRotation.Pitch, CameraRotation.Yaw), GetWorld()->GetDeltaSeconds(), 10.0f));
}


void AUnforgottenCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    ACharacter::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Get the player's position
	FVector PlayerPosition = GetActorLocation();

	// Get the player's height
	float PlayerHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;

	if(HitLocation.Z <= (PlayerPosition - FVector(0, 0, PlayerHeight * 0.5f)).Z)
	{
		DrawDebugPoint(GetWorld(), HitLocation, 5, FColor::Red, true, 1.0f);
		return;
	}

	DrawDebugPoint(GetWorld(), HitLocation, 5, FColor::Green, true, 1.0f);

	FVector Direction = (HitLocation - PlayerPosition).GetSafeNormal();

	DrawDebugLine(GetWorld(), PlayerPosition, HitLocation, FColor::Blue, true, 1.0f);
	if(GetCharacterMovement()->Velocity.Z < 0) // Determine if player is descending
	{
		FireRays(Direction, HitNormal);
	}
}

void AUnforgottenCharacter::FireRays(FVector Direction, FVector HitNormal)
{
    FVector HeadLocation = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()); // Head location
	FVector ChestLocation = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2); // Chest location
    FVector CenterLocation = GetActorLocation(); // Center location
	FVector KneeLocation = GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2); // Knee location
    FVector FeetLocation = GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()); // Feet location

	Direction.Z = 0.0f; // Ignore vertical velocity

	float TraceDistance = GetCapsuleComponent()->GetScaledCapsuleRadius() * 2;

    FHitResult HitResult;

	// Perform ray tracing
	bool HeadHit = GetWorld()->LineTraceSingleByChannel(HitResult, HeadLocation, HeadLocation + Direction * TraceDistance, ECC_Visibility);
	bool ChestHit = GetWorld()->LineTraceSingleByChannel(HitResult, ChestLocation, ChestLocation + Direction * TraceDistance, ECC_Visibility);
	bool BodyHit = GetWorld()->LineTraceSingleByChannel(HitResult, CenterLocation, CenterLocation + Direction * TraceDistance, ECC_Visibility);
	bool KeeHit = GetWorld()->LineTraceSingleByChannel(HitResult, KneeLocation, KneeLocation + Direction * TraceDistance, ECC_Visibility);
	bool FeetHit = GetWorld()->LineTraceSingleByChannel(HitResult, FeetLocation, FeetLocation + Direction * TraceDistance, ECC_Visibility);

}

void AUnforgottenCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUnforgottenCharacter::Jump()
{
	ACharacter::Jump();

	if(bIsWallRun)
	{
		StopWallRun(0.35f);
		LaunchCharacter(FVector(WallNormal.X * 300.0f, WallNormal.Y * 300.0f, 400.0f), false, true); // Move forward
	}
}

//////////////////////////////////////////////////////////////////////////// Events

void AUnforgottenCharacter::OnJumped_Implementation()
{
	GetCharacterMovement()->bNotifyApex = true; // Enables the NotifyJumpApex function to fire, gets reset in the method
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("Player jumped!"));
}

void AUnforgottenCharacter::StopJumping()
{
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, TEXT("Player let go of jump!"));
}

void AUnforgottenCharacter::NotifyJumpApex()
{
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, TEXT("Player jump peak reached, now falling!"));
}

void AUnforgottenCharacter::Falling()
{
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, TEXT("Player entered falling state!"));
}

void AUnforgottenCharacter::Landed(const FHitResult & Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("Player Landed!"));

	if(bIsWallRun)
		StopWallRun(0.01f);
}

//////////////////////////////////////////////////////////////////////////// Getter / Setter

void AUnforgottenCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AUnforgottenCharacter::GetHasRifle()
{
	return bHasRifle;
}