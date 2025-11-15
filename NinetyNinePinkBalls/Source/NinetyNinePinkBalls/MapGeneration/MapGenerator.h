// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


// UE
#include "CoreMinimal.h"
#include "UObject/Object.h"

// Generated
#include "MapGenerator.generated.h"

/**
 * 
 */
enum class EWallOrientation
{
	Vertical = 0,
	Horizontal,
};

struct FWallPosition
{
	FWallPosition(const FVector& Position, const EWallOrientation Orientation) : Position(Position), Orientation(Orientation) {}
	FVector Position{};
	EWallOrientation Orientation {};
};

UCLASS()
class NINETYNINEPINKBALLS_API AMapGenerator : public AActor
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;
	
private:
	TArray<FWallPosition> CalculateWallPositions();
	void GenerateMap();
	void SpawnMapElement(USceneComponent* ComponentToSpawn, const FVector& Position, const FRotator& Rotation = {});

	void SpawnFloor();
	void SpawnWalls();
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TWeakObjectPtr<USceneComponent>> SpawnedMapElements;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Elements")	
	TArray<UStaticMesh*> WallMeshes;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Elements")	
	TArray<UStaticMesh*> FloorMeshes;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 MapWidth;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 MapHeight;
	
	UPROPERTY(EditDefaultsOnly, Category="Tile Settings")
	float TileSize = 500.f;
};
