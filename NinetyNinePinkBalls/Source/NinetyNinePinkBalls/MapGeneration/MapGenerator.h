#pragma once

#include <memory>

// UE
#include <memory>
#include <vector>
#include <vector>

#include "CoreMinimal.h"
#include "UObject/Object.h"

// Generated
#include "MapGenerator.generated.h"

namespace pavage
{
	struct collider_t;
}

struct collider_t;
struct map_config_t;

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
	std::vector<std::shared_ptr<collider_t>> CalculatePositionsWithMap();
	std::vector<std::shared_ptr<pavage::collider_t>> CalculatePositionsWithPavage();
	map_config_t GetConfig() const;

	void SetMapReady();
	void SpawnWallsAndDoors();
	void GenerateMap();
	void SpawnMapElement(USceneComponent* ComponentToSpawn, const FVector& Position, const FRotator& Rotation = {});

	void PlaceObstacle();
	
	void SpawnFloor();
	
	//template<class T>
	void SpawnWalls();
	
	
	void SpawnBalls();
	
	UPROPERTY(VisibleAnywhere)
	TArray<TWeakObjectPtr<USceneComponent>> _spawnedMapElements;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Elements")	
	TArray<UStaticMesh*> WallMeshes;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Elements")	
	TArray<UStaticMesh*> FloorMeshes;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Elements")
	TArray<UStaticMesh*> DoorMeshes;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	bool UsesPavage = true;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 MapWidth;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 MapHeight;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	int32 Threshold = 30;
	
	UPROPERTY(EditDefaultsOnly, Category="Map Options")
	float Scale = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Tile Settings")
	float TileSize = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Balls settings")
	TSubclassOf<AActor> _ballActorClass;

	UPROPERTY(EditDefaultsOnly, Category="Balls settings")
	TSubclassOf<AActor> _hauntedBallActorClass;

	UPROPERTY(EditDefaultsOnly, Category="Balls settings")
	int32 _ballCount = 0;

	bool _isMapReady = false;
	FVector _playerStartPosition = FVector::Zero();
	FVector _ghostPosition = FVector::Zero();;

};
