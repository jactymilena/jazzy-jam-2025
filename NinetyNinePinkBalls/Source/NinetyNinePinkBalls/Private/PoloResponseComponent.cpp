#include "PoloResponseComponent.h"

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
	
	GetWorld()->GetTimerManager().SetTimer(
		MyDelayTimerHandle,
		this,
		&UPoloResponseComponent::PlayResponseSound,
		DelayToRespond,
		false
	);
}
