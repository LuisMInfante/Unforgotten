// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemies/EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "UnforgottenCharacter.h"

// Sets default values
AEnemy::AEnemy() :
	CurrentHealth(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(3.f),
	bCanHitReact(true),
	HitReactTimeMin(0.5f),
	HitReactTimeMax(3.0f),
	HitNumberDestroyTime(1.5f),
	bStunnned(false),
	AttackLFast(TEXT("AttackLFast")),
	AttackRFast(TEXT("AttackRFast")),
	AttackL(TEXT("AttackL")),
	AttackR(TEXT("AttackR"))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Box"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));

	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);

	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	EnemyController = Cast<AEnemyController>(GetController());
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);

		EnemyController->RunBehaviorTree(BehaviorTree);
	}

}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::Die() 
{
	HideHealthBar();
}

void AEnemy::SetStunned(bool Stunned) 
{
	bStunnned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			TEXT("Stunned"),
			Stunned
		);
	}
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate) 
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}

		bCanHitReact = false;
		const float HitReactTime = FMath::FRandRange(HitReactTimeMin, HitReactTimeMax);
		GetWorldTimerManager().SetTimer(
			HitReactTimer,
			this,
			&AEnemy::ResetHitReactTimer,
			HitReactTime
		);
	}
}

void AEnemy::ResetHitReactTimer() 
{
	bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location) 
{
	HitNumbers.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
	GetWorld()->GetTimerManager().SetTimer(
		HitNumberTimer, 
		HitNumberDelegate, 
		HitNumberDestroyTime, 
		false
	);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber) 
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers() 
{
	for (auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{ HitPair.Key };
		const FVector Location{ HitPair.Value };
		FVector2D ScreenPosition;

		UGameplayStatics::ProjectWorldToScreen(
			GetWorld()->GetFirstPlayerController(), 
			Location,
			ScreenPosition
		);

		HitNumber->SetPositionInViewport(ScreenPosition);
	}
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult) 
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation()
		);
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticles,
			HitResult.Location,
			FRotator(0.f),
			true
		);
	}

	ShowHealthBar();
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		PlayHitMontage(FName("HitReactFront"));
		SetStunned(true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController* EventInstigator, AActor* DamageCauser) 
{
	if (CurrentHealth - DamageAmount <= 0.f)
	{
		CurrentHealth = 0.f;
		Die();
	}
	else
	{
		CurrentHealth -= DamageAmount;
	}

	return DamageAmount;
}

void AEnemy::ShowHealthBar_Implementation() 
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(
		HealthBarTimer, 
		this, 
		&AEnemy::HideHealthBar,
		HealthBarDisplayTime
	);
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) 
{
	if (!OtherActor) return;
	AUnforgottenCharacter* Character = Cast<AUnforgottenCharacter>(OtherActor);

	if (Character)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(
			TEXT("Target"),
			Character
		);
	}
}

void AEnemy::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) 
{
	if (!OtherActor) return;

	AUnforgottenCharacter* Character = Cast<AUnforgottenCharacter>(OtherActor);

	if (Character)
	{
		bInAttackRange = true;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("InAttackRange"),
				true
			);
		}
	}
}

void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex) 
{
	if (!OtherActor) return;

	AUnforgottenCharacter* Character = Cast<AUnforgottenCharacter>(OtherActor);

	if (Character)
	{
		bInAttackRange = false;

		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("InAttackRange"),
				false
			);
		}
	}
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate) 
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}
}

FName AEnemy::GetAttackSectionName() 
{	
	FName SectionName;
	const int32 Section = FMath::RandRange(1, 4);

	switch (Section)
	{
		case 1:
			SectionName = AttackLFast;
			break;
		case 2:
			SectionName = AttackRFast;
			break;
		case 3:
			SectionName = AttackL;
			break;
		case 4:
			SectionName = AttackR;
			break;
	}

	return SectionName;
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) 
{

}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) 
{

}

void AEnemy::ActivateLeftWeapon() 
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon() 
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon() 
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon() 
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
