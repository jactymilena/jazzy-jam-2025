#include "PoloResponseComponent.h"

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UPoloResponseComponent::PlayResponseSound()
{
	if (IsValid(ResponseSound))
	{
		FVector SoundLocation = GetComponentLocation();
		FRotator SoundRotation = GetComponentRotation();
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ResponseSound,
			SoundLocation,
			SoundRotation,
			VolumeMultiplier,
			PitchMultiplier,
			0.0f,
			AttenuationSettings
		);
	}
}

void UPoloResponseComponent::RespondPolo()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Polo Response");
	
	OnPoloResponse.Broadcast();
	
	float MinDelay = std::max(DelayToRespond - DelayToRespondDelta, 0.0f);
	float MaxDelay = DelayToRespond + DelayToRespondDelta;
	float Delay = FMath::FRandRange(MinDelay, MaxDelay);
	
	GetWorld()->GetTimerManager().SetTimer(
		MyDelayTimerHandle,
		this,
		&UPoloResponseComponent::PlayResponseSound,
		Delay,
		false
	);
}
