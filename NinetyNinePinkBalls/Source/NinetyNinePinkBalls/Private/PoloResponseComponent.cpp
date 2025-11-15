#include "PoloResponseComponent.h"

void UPoloResponseComponent::RespondPolo()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Polo Response");
}
