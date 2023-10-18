#include "CustomMovementComponent.h"

/* Initialization */
void UCustomMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AirControl = 1.0f;

	GetNavAgentPropertiesRef().bCanCrouch = true;
}
