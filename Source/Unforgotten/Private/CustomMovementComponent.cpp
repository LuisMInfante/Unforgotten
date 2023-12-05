#include "CustomMovementComponent.h"
#include "GameFramework/Character.h"
#include "CustomMovementMode.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"

/* Initialization */
void UCustomMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AirControl = 1.0f;

	bWallRunHeld = false;
	GetNavAgentPropertiesRef().bCanCrouch = true;

	(APlayerController*) CharacterOwner->Controller;

	// We don't want simulated proxies detecting their own collision
	if (GetPawnOwner()->GetLocalRole() > ROLE_SimulatedProxy)
	{
		// Bind to the OnActorHit component so we're notified when the owning actor hits something (like a wall)
		GetPawnOwner()->OnActorHit.AddDynamic(this, &UCustomMovementComponent::OnActorHit);
	}
}

void UCustomMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *TickFunction)
{
	// Peform local only checks
	if (GetPawnOwner()->IsLocallyControlled())
	{
		if (bSprintHeld == true)
		{
			// Only set WantsToSprint to true if the player is moving forward (so that he can't sprint backwards)
			FVector CurrentVelocity = GetPawnOwner()->GetVelocity();
			FVector Forward = GetPawnOwner()->GetActorForwardVector();
			CurrentVelocity.Z = 0.0f;
			Forward.Z = 0.0f;
			CurrentVelocity.Normalize();
			Forward.Normalize();

			WantsToSprint = FVector::DotProduct(CurrentVelocity, Forward) > 0.5f;
		}
		else
			WantsToSprint = false;

		// Update if the required wall run key(s) are being pressed
		GetKeyDown();
		
		CrouchUpdate(false);
	}

	// TODO: AOE Wall ride like Lucio rather than angled checks

	FHitResult HitRightForward = CheckSurrounding(true, true);
	FHitResult HitLeftForward = CheckSurrounding(true, false);
	FHitResult HitRightBack = CheckSurrounding(false, true);
	FHitResult HitLeftBack = CheckSurrounding(false, false);

	if (HitRightForward.bBlockingHit || HitLeftForward.bBlockingHit || HitRightBack.bBlockingHit || HitLeftBack.bBlockingHit)
		GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::Green, TEXT("Touching Wall"), false);
	else
		GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::Red, TEXT("Not Touching Wall"), false);
	

	Super::TickComponent(DeltaTime, TickType, TickFunction);
}

/* Events */
void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
            case (uint8) ECustomMovementMode::MOVE_WallRun:
            {
                // Stop current movement and constrain the character to only horizontal movement
                // StopMovementImmediately();
                bConstrainToPlane = true;
                // SetPlaneConstraintNormal(FVector(0.0f, 0.0f, 1.0f));
				break;
            }
            

			case (uint8) ECustomMovementMode::MOVE_Slide:
			{
				EnterSlide();
				break;
			}
		}
	}

	if (PreviousMovementMode == MOVE_Custom)
	{
		switch (PreviousCustomMode)
		{
            case (uint8) ECustomMovementMode::MOVE_WallRun:
            {
                // Unconstrain the character from horizontal movement
                bConstrainToPlane = false;
				break;
            }

			case (uint8) ECustomMovementMode::MOVE_Slide:
			{
				ExitSlide();
				break;
			}
		}
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UCustomMovementComponent::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	if (IsCustomMovementMode((uint8) ECustomMovementMode::MOVE_WallRun))
		return;

	if (!IsFalling())
		return;

	if (!CanSurfaceBeWallRan(Hit.ImpactNormal))
		return;

	FindWallRunDirectionAndSide(Hit.ImpactNormal, WallRunDirection, bWallRunRight);

	if (!IsNextToWall())
		return;

	BeginWallRun();
}

bool UCustomMovementComponent::DoJump(bool bReplayingMoves)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString::Printf(TEXT("Jumped")));
	bNotifyApex = true;
	return Super::DoJump(bReplayingMoves);
}

void UCustomMovementComponent::NotifyJumpApex()
{
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, TEXT("Reached peak"));
	Super::NotifyJumpApex();
}

void UCustomMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	if (IsCustomMovementMode((uint8) ECustomMovementMode::MOVE_WallRun))
		EndWallRun();
	
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString::Printf(TEXT("Landed")));
}

void UCustomMovementComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (GetPawnOwner() != nullptr && GetPawnOwner()->GetLocalRole() > ROLE_SimulatedProxy)
		GetPawnOwner()->OnActorHit.RemoveDynamic(this, &UCustomMovementComponent::OnActorHit);

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

/* Utility */
bool UCustomMovementComponent::BeginWallRun()
{
	if (bWallRunHeld && bSprintHeld)
	{
		SetMovementMode(EMovementMode::MOVE_Custom, (uint8) ECustomMovementMode::MOVE_WallRun);
		return true;
	}

	return false;
}

void UCustomMovementComponent::EndWallRun()
{
	SetMovementMode(EMovementMode::MOVE_Falling);
}

FHitResult UCustomMovementComponent::CheckSurrounding(bool bForward, bool bRight)
{
	FName TraceTag("WallTraceTag");
	//Uncomment to see the trace shape from the player to the wall
	GetWorld()->DebugDrawTraceTag = TraceTag;

	//Set up trace parameters
	FCollisionQueryParams TraceParams = FCollisionQueryParams(TraceTag, true, GetPawnOwner());
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bFindInitialOverlaps = true;

	FHitResult FHit(ForceInit);

	//Set up size/shape of trace
	FCollisionShape TraceShape = FCollisionShape::MakeSphere(30.0f); //CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius());

	// FVector EndPoint = GetPawnOwner()->GetActorLocation() + GetPawnOwner()->GetActorForwardVector() * -5;

	FVector EndPoint = GetPawnOwner()->GetActorLocation() + (GetPawnOwner()->GetActorForwardVector() * (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * (bForward ? 1 : -1))) + (GetPawnOwner()->GetActorRightVector() * (40.0f * (bRight ? 1 : -1)));

	
	//Perform trace
	GetWorld()->SweepSingleByChannel(
		FHit,
		GetPawnOwner()->GetActorLocation(),
		EndPoint,
		GetPawnOwner()->GetActorRotation().Quaternion(),
		ECollisionChannel::ECC_Visibility,
		TraceShape,
		TraceParams
	);
	
	return FHit;
}

void UCustomMovementComponent::GetKeyDown()
{
	// Since this function is checking for input, it should only be called for locally controlled character
	if (GetPawnOwner()->IsLocallyControlled() == false)
		return;
	
	// TODO: Use nore dynamic way to detect held buttons
	APlayerController* PlayerController = Cast<APlayerController>(CharacterOwner->Controller);

	bWallRunHeld =  PlayerController->IsInputKeyDown(EKeys::SpaceBar);
	bSprintHeld = PlayerController->IsInputKeyDown(EKeys::LeftShift);
	bCrouchHeld = PlayerController->IsInputKeyDown(EKeys::LeftControl);

}

bool UCustomMovementComponent::IsNextToWall(float VerticalTolerance)
{
	FVector ActorLocation = GetPawnOwner()->GetActorLocation() + (WallRunDirection * 20.0f);
	FVector EndPoint = ActorLocation + (FVector::CrossProduct(WallRunDirection, bWallRunRight ? FVector(0.0f, 0.0f, 1.0f) : FVector(0.0f, 0.0f, -1.0f)) * 100);

	FHitResult HitResult;

	// Create a helper lambda for performing the line trace
	auto WallHit = [&](const FVector &Start, const FVector &End)
	{
		return (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility));
	};

	// If a vertical tolerance was provided we want to do two line traces - one above and one below the calculated line
	if (VerticalTolerance > FLT_EPSILON)
	{
		// If both line traces miss the wall then return false, we're not next to a wall
		if (WallHit(FVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z + VerticalTolerance / 2.0f), FVector(EndPoint.X, EndPoint.Y, EndPoint.Z + VerticalTolerance / 2.0f)) == false &&
			WallHit(FVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z - VerticalTolerance / 2.0f), FVector(EndPoint.X, EndPoint.Y, EndPoint.Z - VerticalTolerance / 2.0f)) == false)
		{
			return false;
		}
		DrawDebugLine(GetWorld(), ActorLocation, EndPoint, FColor::Yellow, true, 1.0f);
		// DrawDebugLine(GetWorld(), traceStart, EndPoint, FColor::Red, true, 1.0f);
	}
	// If no vertical tolerance was provided we just want to do one line trace using the caclulated line
	else
	{
		// return false if the line trace misses the wall
		if (!WallHit(ActorLocation, EndPoint))
			return false;
	}

	// Make sure we're still on the side of the wall we expect to be on
	bool bIsWallRunRight;
	WallRunNormal = HitResult.ImpactNormal;
	FindWallRunDirectionAndSide(WallRunNormal, WallRunDirection, bIsWallRunRight);
	if (bIsWallRunRight != bWallRunRight)
		return false;

	return true;
}

void UCustomMovementComponent::FindWallRunDirectionAndSide(const FVector &WallNormal, FVector &Direction, bool &Side) const
{
	FVector CrossVector;

	FVector RightVector = FVector::CrossProduct(Velocity, FVector::UpVector);
	RightVector.Normalize();

	DrawDebugPoint(GetWorld(), GetPawnOwner()->GetActorLocation() + (RightVector * 20.0f), 5, FColor::Red, true, 1.0f);
	DrawDebugPoint(GetWorld(), GetPawnOwner()->GetActorLocation() + (WallNormal * 20.0f), 5, FColor::Green, true, 1.0f);
	DrawDebugPoint(GetWorld(), GetPawnOwner()->GetActorLocation() + (GetPawnOwner()->GetActorRightVector() * 20.0f), 5, FColor::Blue, true, 1.0f);

	DrawDebugLine(GetWorld(), GetPawnOwner()->GetActorLocation(), GetPawnOwner()->GetActorLocation() + (RightVector * 20.0f), FColor::Red, true, 1.0f);
	DrawDebugLine(GetWorld(), GetPawnOwner()->GetActorLocation(), GetPawnOwner()->GetActorLocation() + (WallNormal * 20.0f), FColor::Green, true, 1.0f);
	DrawDebugLine(GetWorld(), GetPawnOwner()->GetActorLocation(), GetPawnOwner()->GetActorLocation() + (GetPawnOwner()->GetActorRightVector() * 20.0f), FColor::Blue, true, 1.0f);

	if (FVector2D::DotProduct(FVector2D(WallNormal), FVector2D(GetPawnOwner()->GetActorRightVector())) > 0.0)
	{
		Side = true;
		CrossVector = FVector(0.0f, 0.0f, 1.0f);
	}
	else
	{
		Side = false;
		CrossVector = FVector(0.0f, 0.0f, -1.0f);
	}

	// Find the direction parallel to the wall in the direction the player is moving
	Direction = FVector::CrossProduct(WallNormal, CrossVector);
}

bool UCustomMovementComponent::CanSurfaceBeWallRan(const FVector &WallNormal) const
{
	// Return false if the surface normal is facing down
	if (WallNormal.Z < -0.05f)
		return false;

	FVector CurrentWallNormal = FVector(WallNormal.X, WallNormal.Y, 0.0f);
	CurrentWallNormal.Normalize();

	// Find the angle of the wall
	float WallAngle = FMath::Acos(FVector::DotProduct(CurrentWallNormal, WallNormal));

	// Return true if the wall angle is less than the walkable floor angle
	return WallAngle < GetWalkableFloorAngle();
}

void UCustomMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	// Phys* functions should only run for characters with ROLE_Authority or ROLE_AutonomousProxy. However, Unreal calls PhysCustom in
	// two seperate locations, one of which doesn't check the role, so we must check it here to prevent this code from running on simulated proxies.
	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
		return;

	switch (CustomMovementMode)
	{
		case (uint8) ECustomMovementMode::MOVE_WallRun:
		{
			PhysWallRunning(DeltaTime, Iterations);
			break;
		}
		case (uint8) ECustomMovementMode::MOVE_Slide:
		{
			PhysSlide(DeltaTime, Iterations);
			break;
		}
	}

	// Not sure if this is needed
	Super::PhysCustom(DeltaTime, Iterations);
}

void UCustomMovementComponent::PhysWallRunning(float DeltaTime, int32 Iterations)
{
	// IMPORTANT NOTE: This function (and all other Phys* functions) will be called on characters with ROLE_Authority and ROLE_AutonomousProxy
	// but not ROLE_SimulatedProxy. All movement should be performed in this function so that is runs locally and on the server. UE4 will handle
	// replicating the final position, velocity, etc.. to the other simulated proxies.

	// Make sure the required wall run keys are still down
	if (bSprintHeld == false)
	{
		EndWallRun();
		return;
	}

	if(bWallRunHeld == false)
	{

		UCameraComponent* Camera = nullptr;

		for (UActorComponent* Component : CharacterOwner->GetComponents())
			if (UCameraComponent* CameraComponent = Cast<UCameraComponent>(Component))
				Camera = CameraComponent;

		FTransform CameraTransform = Camera->GetComponentTransform();

		float CameraPitch = Camera == nullptr ? 0.0f : CameraTransform.GetRotation().Rotator().Pitch;
		CameraPitch = CameraPitch >= 0 ? CameraPitch : 0;

		// GEngine->AddOnScreenDebugMessage(-1, 5.5f, FColor::Green, FString::Printf(TEXT("Vel: (%f, %f, %f) SIZE (%f, %f, %f)"), Velocity.X, Velocity.Y, Velocity.Z, FVector(Velocity.X, 0, 0).Size(), FVector(0, Velocity.Y, 0).Size(), FVector(0, 0, Velocity.Z).Size()));
		FVector newVelocity = WallRunNormal + CharacterOwner->GetActorForwardVector();
		// newVelocity.Z = CameraPitch / 120;
		// GEngine->AddOnScreenDebugMessage(-1, 5.5f, FColor::Green, FString::Printf(TEXT("WallNormal: (%f, %f, %f)"), newVelocity.X, newVelocity.Y, newVelocity.Z));
		// GEngine->AddOnScreenDebugMessage(-1, 5.5f, FColor::Green, FString::Printf(TEXT("Actor: (%f, %f, %f)"), CharacterOwner->GetActorForwardVector().X, CharacterOwner->GetActorForwardVector().Y, CameraPitch));

		newVelocity *= FMath::Abs(Velocity.X) > FMath::Abs(Velocity.Y) ? FMath::Abs(Velocity.X) : FMath::Abs(Velocity.Y);
		newVelocity.Z = bWallStick ? 0 : WallRunJump + WallRunJump * (CameraPitch / 180);
		Velocity = newVelocity;

		const FVector Adjusted = Velocity * DeltaTime;
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

		// GEngine->AddOnScreenDebugMessage(-1, 5.5f, FColor::Green, FString::Printf(TEXT("Jumped Off Wall Run: (%f, %f, %f)"), newVelocity.X, newVelocity.Y, newVelocity.Z));

		// DrawDebugLine(GetWorld(), GetPawnOwner()->GetActorLocation() + (WallRunDirection * 20.0f), GetPawnOwner()->GetActorLocation() + (WallRunDirection * 20.0f) + Adjusted, FColor::Red, true, 1.0f);
		// DrawDebugPoint(GetWorld(), GetPawnOwner()->GetActorLocation() + (WallRunDirection * 20.0f) + Adjusted, 5, FColor::Green, true, 1.0f);
		// DrawDebugPoint(GetWorld(), GetPawnOwner()->GetActorLocation() + (WallRunDirection * 20.0f), 5, FColor::Blue, true, 1.0f);

		// DrawDebugLine(GetWorld(), GetPawnOwner()->GetActorLocation() + (WallRunNormal), GetPawnOwner()->GetActorLocation() + (WallRunNormal * 20.0f), FColor::Green, true, 1.0f);
		// DrawDebugPoint(GetWorld(), GetPawnOwner()->GetActorLocation() + (WallRunNormal), 5, FColor::Green, true, 1.0f);
		// DrawDebugPoint(GetWorld(), GetPawnOwner()->GetActorLocation() + (WallRunNormal * 20.0f), 5, FColor::Blue, true, 1.0f);

		EndWallRun();
		return;
	}
	

	// Make sure we're still next to a wall. Provide a vertial tolerance for the line trace since it's possible the the server has
	// moved our character slightly since we've began the wall run. In the event we're right at the top/bottom of a wall we need this
	// tolerance value so we don't immiedetly fall of the wall 
	if (IsNextToWall(LineTraceVerticalTolerance) == false)
	{
		EndWallRun();
		return;
	}

	// Set the owning player's new velocity based on the wall run direction
	FVector newVelocity = WallRunDirection;
	newVelocity.X *= FVector(Velocity.X, 0, 0).Size();
	newVelocity.Y *= FVector(0, Velocity.Y, 0).Size();
	newVelocity.Z = bWallStick ? 0 : NewFallVelocity(Velocity, FVector(0.f, 0.f, GetGravityZ() / (2 / GravityScale)), DeltaTime).Z;
	Velocity = newVelocity;

	const FVector Adjusted = Velocity * DeltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);
}

void UCustomMovementComponent::CrouchUpdate(bool bClientSimulation)
{
	if(bCrouchHeld)
		Crouch(bClientSimulation);
	else
		UnCrouch(bClientSimulation);
}
void UCustomMovementComponent::Crouch(bool bClientSimulation)
{
	if(bCrouchHeld && bSprintHeld && !IsCustomMovementMode((uint8)ECustomMovementMode::MOVE_Slide) && Velocity.SizeSquared() > pow(700.0f, 2) && !IsFalling())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.5f, FColor::Green, FString::Printf(TEXT("Entered SLide")));
		SetMovementMode(EMovementMode::MOVE_Custom, (uint8) ECustomMovementMode::MOVE_Slide);
	}
	GetCharacterOwner()->Crouch(bClientSimulation);
	Super::Crouch(bClientSimulation);
}

void UCustomMovementComponent::UnCrouch(bool bClientSimulation)
{
	GetCharacterOwner()->UnCrouch(bClientSimulation);
	Super::UnCrouch(bClientSimulation);
}

void UCustomMovementComponent::EnterSlide()
{
	// bWantsToCrouch = true;
	bOrientRotationToMovement = false;
	Velocity += Velocity.GetSafeNormal2D() * 400.0f;

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, NULL);
}
void UCustomMovementComponent::ExitSlide()
{
	// bWantsToCrouch = false;
	bOrientRotationToMovement = true;
	SetMovementMode(EMovementMode::MOVE_Walking);
}

void UCustomMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	
	if (!(Velocity.SizeSquared() > pow(400.0f, 2))) // Min Slide speed is 400
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.5f, FColor::Green, FString::Printf(TEXT("Not enough velocity")));
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = DeltaTime;

	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * 4000.0f * DeltaTime; // Slide Gravity Force
		
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector().GetSafeNormal2D());

		// Apply acceleration
		CalcVelocity(timeTick, GroundFriction * 0.06f, false, GetMaxBrakingDeceleration()); // SLide Friction 0.06
		
		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if ( IsFalling() )
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
			else if ( IsSwimming() ) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}


		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}
		
		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}


	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}

float UCustomMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
	{
		if (IsCrouching())
		{
			return MaxWalkSpeedCrouched;
		}
		else
		{
			if (WantsToSprint)
				return SprintSpeed;

			return RunSpeed;
		}
	}
	case MOVE_Falling:
		if (WantsToSprint)
			return SprintSpeed;
		return RunSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}

float UCustomMovementComponent::GetMaxAcceleration() const
{
	if (IsMovingOnGround())
	{
		if (WantsToSprint)
		{
			return SprintAcceleration;
		}

		return RunAcceleration;
	}

	return Super::GetMaxAcceleration();
}

void UCustomMovementComponent::SetSprinting(bool sprinting)
{
	bSprintHeld = sprinting;
}

bool UCustomMovementComponent::IsCustomMovementMode(uint8 custom_movement_mode) const
{
	return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == custom_movement_mode;
}