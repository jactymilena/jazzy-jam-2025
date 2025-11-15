#include "MapGenerator.h"
#include "maze.cpp"


bool AMapGenerator::IsMapReady() const
{
	return _isMapReady;
}

FVector AMapGenerator::GetPlayerStartPosition() const
{
	return FVector(MapWidth * TileSize / 2.f, MapHeight * TileSize / 2.f, 200.f);
}

void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	GenerateMap();
}

std::vector<std::shared_ptr<collider_t>> AMapGenerator::CalculateWallPositions()
{
	map_config_t config;
	config.height = MapHeight;
	config.width = MapWidth;
	config.segment_length = TileSize;
	config.threshold = Threshold;
	
	map_t map {config};
	auto walls = map.get_walls();
	
	return walls;
}

void AMapGenerator::SetMapReady()
{
	_isMapReady = true;
	OnMapReady.Broadcast();
}

void AMapGenerator::GenerateMap()
{
	SpawnFloor();
	SpawnWalls();
	PlaceObstacle();
	PlacePlayer();
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::SetMapReady));
	SetMapReady();
}

void AMapGenerator::SpawnMapElement(USceneComponent* ComponentToSpawn, const FVector& Position, const FRotator& Rotation)
{
	ComponentToSpawn->SetRelativeLocation(Position);
	ComponentToSpawn->SetRelativeRotation(Rotation);
			
	ComponentToSpawn->RegisterComponent();
	_spawnedMapElements.Add(ComponentToSpawn);
}

void AMapGenerator::PlaceObstacle()
{
}

void AMapGenerator::PlacePlayer()
{
	FVector mapCenter = FVector(MapWidth * TileSize, MapHeight * TileSize, 10.f);
	_playerStartPosition = mapCenter;
}

void AMapGenerator::SpawnFloor()
{
	FVector startPosition = FVector(250.f, 250.f, 0.f);
	for (int i = 0; i< MapWidth; i++)
	{
		for (int j = 0; j< MapHeight; j++)
		{
			UStaticMeshComponent* spawnedFloorTile = NewObject<UStaticMeshComponent>(this);
			spawnedFloorTile->SetStaticMesh(FloorMeshes[0]);
			const auto position = FVector(TileSize * i, TileSize * j, 0.f) + startPosition;
			SpawnMapElement(spawnedFloorTile, position);
		}
	}
}

void AMapGenerator::SpawnWalls()
{
	std::vector<std::shared_ptr<collider_t>> wallPositions = CalculateWallPositions();
	
	for (const auto& wallPosition : wallPositions)
	{
		UStaticMeshComponent* wallToSpawn = NewObject<UStaticMeshComponent>(this);
	
		wallToSpawn->SetStaticMesh(WallMeshes[0]);
		
		FRotator rotation = wallPosition->orientation == wall_orientation::V
			? FRotator{} 
			: FRotator(0, 90.f, 0.f);
		
		SpawnMapElement(wallToSpawn, wallPosition->centroid, rotation);
	}
}
