#include "MapGenerator.h"

void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	GenerateMap();
}

TArray<FWallPosition> AMapGenerator::CalculateWallPositions()
{
	TArray<FWallPosition> WallPositions = {
		FWallPosition{FVector(500.f, 500.f, 0.f), EWallOrientation::Vertical},
		FWallPosition{FVector(1000.f, 1000.f, 0.f), EWallOrientation::Horizontal}
	};
	return WallPositions;
}

void AMapGenerator::GenerateMap()
{
	SpawnFloor();
	SpawnWalls();
}

void AMapGenerator::SpawnMapElement(USceneComponent* ComponentToSpawn, const FVector& Position, const FRotator& Rotation)
{
	ComponentToSpawn->SetRelativeLocation(Position);
	ComponentToSpawn->SetRelativeRotation(Rotation);
			
	ComponentToSpawn->RegisterComponent();
	SpawnedMapElements.Add(ComponentToSpawn);
}

void AMapGenerator::SpawnFloor()
{
	for (int i =0; i< MapWidth; i++)
	{
		for (int j =0; j< MapHeight; j++)
		{
			UStaticMeshComponent* spawnedFloorTile = NewObject<UStaticMeshComponent>(this);
			spawnedFloorTile->SetStaticMesh(FloorMeshes[0]);
			const auto Position = FVector(TileSize * i, TileSize * j, 0.f);
			
			
			SpawnMapElement(spawnedFloorTile, Position);
		}
	}
}

void AMapGenerator::SpawnWalls()
{
	TArray<FWallPosition> wallPositions = CalculateWallPositions();
	for (FWallPosition wallPosition : wallPositions)
	{
		UStaticMeshComponent* wallToSpawn = NewObject<UStaticMeshComponent>(this);

		wallToSpawn->SetStaticMesh(WallMeshes[0]);
		
		FRotator rotation = wallPosition.Orientation == EWallOrientation::Vertical 
			? FRotator{} 
			: FRotator(0, 90.f, 0.f);
		
		SpawnMapElement(wallToSpawn, wallPosition.Position, rotation);
	}
}
