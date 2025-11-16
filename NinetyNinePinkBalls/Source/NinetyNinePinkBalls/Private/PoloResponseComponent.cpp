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

bool UPoloResponseComponent::ShouldRespond() const
{
	constexpr float ProbabilityMinRange = 0.0f;
	constexpr float ProbabilityMaxRange = 100.0f;
	
	const float Probability = FMath::FRandRange(ProbabilityMinRange, ProbabilityMaxRange);
	
	return Probability < ProbabilityToRespond;
}

float UPoloResponseComponent::CalculateRandomDelay() const
{
	const float MinDelay = std::max(DelayToRespond - DelayToRespondDelta, 0.0f);
	const float MaxDelay = DelayToRespond + DelayToRespondDelta;
	
	return FMath::FRandRange(MinDelay, MaxDelay);
}

void UPoloResponseComponent::RespondPolo()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Polo Response");
	
	if (ShouldRespond())
	{
		OnPoloResponse.Broadcast();
        const float Delay = CalculateRandomDelay();
        
        GetWorld()->GetTimerManager().SetTimer(
        	MyDelayTimerHandle,
        	this,
        	&UPoloResponseComponent::PlayResponseSound,
        	Delay,
        	false
        );
	}
	
}
