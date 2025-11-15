#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Ball.generated.h"

UCLASS(Abstract)
class NINETYNINEPINKBALLS_API ABall : public AActor
{
	GENERATED_BODY()

public:
	ABall();

protected:
	UPROPERTY(EditAnywhere, Category = "Ball|Physics", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float BounceAngleDegree = 45.0f;
	
	UPROPERTY(EditAnywhere, Category = "Ball|Physics", meta = (ClampMin = "0.0"))
	float BounceForce = 1500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SphereMesh;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBallHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
