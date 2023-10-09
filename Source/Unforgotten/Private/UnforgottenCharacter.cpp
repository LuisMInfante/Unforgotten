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
#include "DrawDebugHelpers.h" // Include this for debugging visualization

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AUnforgottenCharacter

AUnforgottenCharacter::AUnforgottenCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;

	// Enable tick every frame
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

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

    if (bIsWallSliding)
    {
        // Implement wall sliding logic here
        // Adjust character's movement or apply forces based on WallNormal
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, TEXT("Sliding!"));
    }
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

void AUnforgottenCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    ACharacter::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("NOTIFY HIT: %s"), *Other->GetName()));

	FireRays();

    // Check if the character hit a wall (you can define your wall conditions here)
    // if (/* Your wall detection conditions */)
    // {
    //     // Handle wall collision here
    //     HandleWallCollision();
    // }
}

void AUnforgottenCharacter::FireRays()
{
    FVector HeadLocation = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()); // Head location
    FVector CenterLocation = GetActorLocation(); // Center location
    FVector FeetLocation = GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()); // Feet location

    FVector RayDirection = GetActorForwardVector(); // Ray direction (you can change this to any direction)

	float TraceDistance = 60.0f;

    FHitResult HitResult;

    // Perform ray tracing from the head
    if (GetWorld()->LineTraceSingleByChannel(HitResult, HeadLocation, HeadLocation + RayDirection * TraceDistance, ECC_Visibility))
    {
        // Handle wall collision for the head ray
        HandleWallCollision(HitResult);
    }

    // Perform ray tracing from the center
    if (GetWorld()->LineTraceSingleByChannel(HitResult, CenterLocation, CenterLocation + RayDirection * TraceDistance, ECC_Visibility))
    {
        // Handle wall collision for the center ray
        HandleWallCollision(HitResult);
    }

    // Perform ray tracing from the feet
    if (GetWorld()->LineTraceSingleByChannel(HitResult, FeetLocation, FeetLocation + RayDirection * TraceDistance, ECC_Visibility))
    {
        // Handle wall collision for the feet ray
        HandleWallCollision(HitResult);
    }
}

void AUnforgottenCharacter::HandleWallCollision(const FHitResult& HitResult)
{
    // Handle wall collision here
    // You can access information about the hit, such as HitResult.ImpactPoint and HitResult.ImpactNormal
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Distance: %f"), HitResult.Distance));
}

void AUnforgottenCharacter::WallSlide()
{
    // Check for wall collision and initiate wall sliding
    FVector Start = GetActorLocation();
    FVector ForwardVector = GetActorForwardVector();
    FVector End = ((ForwardVector * 100.0f) + Start); // Adjust the distance as needed

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
    {
        if (HitResult.bBlockingHit)
        {
            // Determine the wall's normal vector and set it
            WallNormal = HitResult.ImpactNormal;
            bIsWallSliding = true;

            // Apply forces or custom logic for wall sliding here
        }
    }
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