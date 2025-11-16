#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "MarcoCallComponent.generated.h"


class UPoloResponseComponent;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NINETYNINEPINKBALLS_API UMarcoCallComponent : public USphereComponent
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* CallSound;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundAttenuation* AttenuationSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float VolumeMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float PitchMultiplier = 1.0f;
	
	UFUNCTION(BlueprintCallable)
	void CallMarco();
	
private:
	TWeakObjectPtr<UPoloResponseComponent> PoloComponentToCall;
	TWeakObjectPtr<AActor> ActorToCall;
	
	void PlayCallSound()const ;
};
