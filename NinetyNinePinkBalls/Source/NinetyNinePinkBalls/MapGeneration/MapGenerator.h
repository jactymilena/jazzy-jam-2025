#pragma once

#include <memory>

// UE
#include <vector>

#include "CoreMinimal.h"
#include "UObject/Object.h"

// Generated
#include "MapGenerator.generated.h"

struct collider_t;
struct FWallPosition;

enum class EWallOrientation
{
	Vertical = 0,
	Horizontal,
};



// struct FWallPosition
// {
// 	FWallPosition(const FVector& Position, const EWallOrientation Orientation) : Position(Position), Orientation(Orientation) {}
// 	FVector Position{};
// 	EWallOrientation Orientation {};
// };

/**
 * Handles the spawning of floor tiles and walls
 */
UCLASS(Abstract, Blueprintable)
class NINETYNINEPINKBALLS_API  AMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:
	DECLARE_EVENT(AMapGenerator, MapReadyEvent);
	MapReadyEvent OnMapReady;
	bool IsMapReady() const;
	
	FVector GetPlayerStartPosition() const;
	
protected:
	virtual void BeginPlay() override;
	
private:
	std::vector<std::shared_ptr<collider_t>>  CalculateWallPositions();

	void SetMapReady();
	void GenerateMap();
	void SpawnMapElement(USceneComponent* ComponentToSpawn, const FVector& Position, const FRotator& Rotation = {});

	void PlaceObstacle();
	void PlacePlayer();
	
	void SpawnFloor();
	void SpawnWalls();
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TWeakObjectPtr<USceneComponent>> _spawnedMapElements;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Elements")	
	TArray<UStaticMesh*> WallMeshes;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Elements")	
	TArray<UStaticMesh*> FloorMeshes;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 MapWidth;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 MapHeight;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 Threshold = 30;
	
	UPROPERTY(EditDefaultsOnly, Category="Tile Settings")
	float TileSize = 500.f;
	
	bool _isMapReady = false;
	FVector _playerStartPosition = FVector::Zero();
};
