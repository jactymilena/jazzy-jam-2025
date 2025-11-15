#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "MarcoCallComponent.generated.h"


UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NINETYNINEPINKBALLS_API UMarcoCallComponent : public USphereComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float MyMarcoCollisionRadius = 100.0f;

	UFUNCTION(BlueprintCallable)
	void CallMarco();
};
