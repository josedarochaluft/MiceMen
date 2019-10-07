// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"

// Sets default values
ABlock::ABlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Cube;
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> RedMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> HighlightMaterial;
		FConstructorStatics()
			: Cube(TEXT("/Engine/BasicShapes/Cube.Cube"))
			, BaseMaterial(TEXT("/Game/Materials/BaseMaterial.BaseMaterial"))
			, BlueMaterial(TEXT("/Game/Materials/BlueMaterialInstance.BlueMaterialInstance"))
			, RedMaterial(TEXT("/Game/Materials/RedMaterialInstance.RedMaterialInstance"))
			, HighlightMaterial(TEXT("/Game/Materials/HighlightedMaterialInstance.HighlightedMaterialInstance"))
		{
		}

	};
	static FConstructorStatics ConstructorStatics;

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	//Create static mesh component
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	VisualMesh->SetStaticMesh(ConstructorStatics.Cube.Get());
	VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	VisualMesh->SetupAttachment(RootComponent);

	// Save pointers to the materials
	BaseMaterial = ConstructorStatics.BaseMaterial.Get();
	BlueMaterial = ConstructorStatics.BlueMaterial.Get();
	RedMaterial = ConstructorStatics.RedMaterial.Get();
	HighlightMaterial = ConstructorStatics.HighlightMaterial.Get();
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMoving && LengthMoved < 1.0f) // If actor is moving but hasn't reached its destination
	{
		LengthMoved += MovementIteration * DeltaTime;
		SetActorLocation(FMath::Lerp(MovementStartLocation, MovementTargetLocation, LengthMoved));
	}
	else if (bMoving && LengthMoved >= 1.0f) // If actor is moving but destination has been reached
	{
		SetActorLocation(FMath::Lerp(MovementStartLocation, MovementTargetLocation, 1.0f));
		bMoving = false;
		if (GetActorLocation().Y != 0.0f)
		{
			MoveTo(FVector(GetActorLocation().X, 0.0f, GetActorLocation().Z), 1);
		}
	}
}

// Set the mesh of our block
void ABlock::SetMesh(UStaticMesh* mesh)
{
	VisualMesh->SetStaticMesh(mesh);
}

// Set the material for mesh
void ABlock::SetMaterial(UMaterial* material)
{
	VisualMesh->SetMaterial(0, material);
}

// Same as above, overloaded for Material Instance instead
void ABlock::SetMaterial(UMaterialInstance* material)
{
	VisualMesh->SetMaterial(0, material);
}

// Set the coordinates of our block
void ABlock::SetCoordinates(FIntPoint point)
{
	Coordinates = point;
}

// Set the block type, changing material accordingly
void ABlock::SetType(EType type)
{
	ActorType = type;
	switch (type)
	{
	case EType::Block:
		SetMaterial(BaseMaterial);
		break;
	case EType::Blue:
		SetMaterial(BlueMaterial);
		break;
	case EType::Red:
		SetMaterial(RedMaterial);
		break;
	}
}

// Start movement towards a given coordinate, distance input is used to determine how fast the block should move
void ABlock::MoveTo(FVector TargetLocation, int32 MovementDistanceInBlocks)
{
	if (!bMoving) //Only execute the code below if the actor is not already moving
	{
		MovementStartLocation = GetActorLocation();
		MovementTargetLocation = TargetLocation;

		//Divide movespeed by number of blocks to move through for speed consistency when using linear interpolation
		MovementIteration = MoveSpeed / MovementDistanceInBlocks;

		LengthMoved = 0.0f;
		bMoving = true;
	}
}

// Update neighbour variables
void ABlock::UpdateNeighbours()
{
	FIntPoint AheadCoordinates;
	FIntPoint BelowCoordinates = FIntPoint(Coordinates.X, Coordinates.Y - 1);
	if (ActorType == EType::Blue)
	{
		AheadCoordinates = FIntPoint(Coordinates.X - 1, Coordinates.Y);
	}
	else
	{
		AheadCoordinates = FIntPoint(Coordinates.X + 1, Coordinates.Y);
	}
	if (GameBoard->BlockMap.Contains(BelowCoordinates))
	{
		Below = GameBoard->BlockMap[BelowCoordinates];
	}
	else
	{
		Below = nullptr;
	}
	if (GameBoard->BlockMap.Contains(AheadCoordinates))
	{
		Ahead = GameBoard->BlockMap[AheadCoordinates];
	}
	else
	{
		Ahead = nullptr;
	}
}

// Toggle highlighting this block with a different material than usual
void ABlock::HighlightToggle()
{
	if (ActorType == EType::Block)
	{
		if (bPainted)
		{
			bPainted = false;
			SetMaterial(BaseMaterial);
		}
		else
		{
			bPainted = true;
			SetMaterial(HighlightMaterial);
		}
	}
}

// Returns true if there are no blocks below this one, unless this block is at the bottom of the grid. If the block is currently moving it will return false regardless. 
bool ABlock::IsFalling()
{
	return (Below == nullptr && Coordinates.Y != 0 && !bMoving);
}

// Returns true if there are no block ahead and this block is not currently moving.
bool ABlock::CanWalk()
{
	return (Ahead == nullptr && !bMoving);
}

int32 ABlock::GetTypeInInt()
{
	return int32(ActorType);
}