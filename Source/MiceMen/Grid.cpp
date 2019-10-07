// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid.h"
#include "Block.h"
#include "Engine/World.h"
#include "Components/TextRenderComponent.h"

// Sets default values
AGrid::AGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IterationOffset = 100.0f;
}

// Called when the game starts or when spawned
void AGrid::BeginPlay()
{
	Super::BeginPlay();
	GridInitialization();
	Populate();
}

// Called every frame
void AGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckGoal(-1);
	CheckGoal(19);

	bCanSettle = SettleBoard();
}

// Initialize the grid coordinates and randomly place cheese blocks
void AGrid::GridInitialization()
{
	for (int32 x = 0; x < 19; ++x)
	{
		for (int32 y = 0; y < 13; ++y)
		{
			FIntPoint NewPoint(x, y);

			if (x == 0 || x == 18)
			{
				if (y % 3 == 0)
				{
					AddBlock(NewPoint, 0);
				}
				else
				{
					BlockMap.Add(NewPoint);
				}
			}
			else if (FMath::RandRange(0, 1) == 0)
			{
				BlockMap.Add(NewPoint);
			}
			else
			{
				AddBlock(NewPoint, 0);
			}
		}
	}
}

// Populate the grid with mice of both teams
void AGrid::Populate()
{
	int NumberOfMice = 12;
	int TeamsToPopulate = 2;
	while (NumberOfMice > 0 && TeamsToPopulate == 2)
	{
		FIntPoint NewPoint(FMath::RandRange(0, 8), FMath::RandRange(0, 12));
		if (BlockMap[NewPoint] == nullptr)
		{
			AddBlock(NewPoint, 2);
			--NumberOfMice;
		}
	}
	--TeamsToPopulate;
	NumberOfMice = 12;
	while (NumberOfMice > 0 && TeamsToPopulate == 1)
	{
		FIntPoint NewPoint(FMath::RandRange(10, 18), FMath::RandRange(0, 12));
		if (BlockMap[NewPoint] == nullptr)
		{
			AddBlock(NewPoint, 1);
			--NumberOfMice;
		}
	}
}

// Add a single block to the grid with a specific coordinate and type as inputs
void AGrid::AddBlock(FIntPoint Coordinates, int32 type)
{
	ABlock* NewBlock = GetWorld()->SpawnActor<ABlock>(FVector(Coordinates.X * IterationOffset, 0.0f, Coordinates.Y * IterationOffset), FRotator(0, 0, 0));
	NewBlock->SetType(EType(type));
	NewBlock->SetCoordinates(Coordinates);
	NewBlock->GameBoard = this;
	BlockMap.Add(Coordinates, NewBlock);

	if (type == 0)
	{
		NewBlock->SetMesh(CheeseMesh);
	}
	else
	{
		NewBlock->SetMesh(MiceMesh);
	}
}

// Shift all blocks of a column upward or downward by 1 unit
void AGrid::MoveColumn(int32 HorizontalCoordinate, bool Upward)
{
	//Create New TMap with items moved 1 unit above or below, append to original TMap to replace items
	TMap<FIntPoint, AActor*> NewGrid;
	for (int32 y = 0; y < 13; ++y)
	{
		FIntPoint PointToMove = FIntPoint(HorizontalCoordinate, y); //Point of current iteration on a given horizontal coordinate
		if (BlockMap.Contains(PointToMove))
		{
			ABlock* Item = Cast<ABlock>(BlockMap[PointToMove]);	//Item associated to point in grid
			FIntPoint MovedPoint;
			if (Upward)
			{
				if (y < 12)
				{
					MovedPoint = FIntPoint(HorizontalCoordinate, y + 1); //Move blocks upward
					if (Item != nullptr)
					{
						Item->MoveTo(Item->GetActorLocation() + FVector(0.0f, 0.0f, +IterationOffset), 1);
					}

				}
				else
				{
					MovedPoint = FIntPoint(HorizontalCoordinate, 0);
					if (Item != nullptr)
					{
						FVector CurrentLocation = Item->GetActorLocation();
						Item->SetActorLocation(CurrentLocation + FVector(0.0f, 100.0f, 0.0f));
						Item->MoveTo(CurrentLocation + FVector(0.0f, 100.0f, -CurrentLocation.Z), 2);
					}
				}
			}
			else
			{
				if (y != 0)
				{
					MovedPoint = FIntPoint(HorizontalCoordinate, y - 1);
					if (Item != nullptr)
					{
						Item->MoveTo(Item->GetActorLocation() + FVector(0.0f, 0.0f, -IterationOffset), 1);
					}
				}
				else
				{
					MovedPoint = FIntPoint(HorizontalCoordinate, 12);
					if (Item != nullptr)
					{
						FVector CurrentLocation = Item->GetActorLocation();
						Item->SetActorLocation(CurrentLocation + FVector(0.0f, 100.0f, 0.0f));
						Item->MoveTo(CurrentLocation + FVector(0.0f, 100.0f, IterationOffset * 12.0f), 2);
					}

				}
			}
			if (Item != nullptr)
			{
				NewGrid.Add(MovedPoint, Item);
				Item->SetCoordinates(MovedPoint);
			}
		}

	}

	for (int32 j = 0; j < 13; ++j)
	{
		FIntPoint CurrentPoint = FIntPoint(HorizontalCoordinate, j);
		BlockMap.Remove(CurrentPoint);
	}

	BlockMap.Append(NewGrid);
}

// Toggle highlight on cheese blocks of a specific column
void AGrid::PaintColumn(int32 column)
{
	for (int32 y = 0; y < 13; ++y)
	{
		FIntPoint NewPoint(column, y);
		if (BlockMap.Contains(NewPoint))
		{
			ABlock* PaintedObject = Cast<ABlock>(BlockMap[NewPoint]);
			if (PaintedObject != nullptr)
			{
				PaintedObject->HighlightToggle();
			}
		}
	}
}

// Look for mice that can move
bool AGrid::SettleBoard()
{
	for (int32 y = 0; y < 13; ++y)
	{
		for (int32 x = 0; x < 19; ++x)
		{
			FIntPoint NewPoint(x, y);
			if (BlockMap.Contains(NewPoint))
			{
				ABlock* BoardPiece = Cast<ABlock>(BlockMap[NewPoint]);
				if (BoardPiece != nullptr)
				{
					// If actor is not a static block
					if (BoardPiece->ActorType != EType::Block)
					{
						BoardPiece->UpdateNeighbours();
						if (BoardPiece->IsFalling())
						{
							FVector Destination = BoardPiece->GetActorLocation() + FVector(0.0f, 0.0f, IterationOffset * -1);
							FIntPoint MovedPoint = FIntPoint(NewPoint.X, NewPoint.Y - 1);
							BlockMap.Add(MovedPoint, BoardPiece);
							BoardPiece->MoveTo(Destination, 1);
							BoardPiece->SetCoordinates(MovedPoint);
							BlockMap.Remove(NewPoint);
							return true;
						}
						else if (BoardPiece->CanWalk())
						{
							FIntPoint MovedPoint;
							FVector Destination;
							if (BoardPiece->ActorType == EType::Blue)
							{
								Destination = BoardPiece->GetActorLocation() + FVector(IterationOffset * -1, 0.0f, 0.0f);
								MovedPoint = FIntPoint(NewPoint.X - 1, NewPoint.Y);
							}
							else
							{
								Destination = BoardPiece->GetActorLocation() + FVector(IterationOffset, 0.0f, 0.0f);
								MovedPoint = FIntPoint(NewPoint.X + 1, NewPoint.Y);
							}
							BlockMap.Add(MovedPoint, BoardPiece);
							BoardPiece->MoveTo(Destination, 1);
							BoardPiece->SetCoordinates(MovedPoint);
							BlockMap.Remove(NewPoint);
							return true;
						}
					}
				}
			}
		}
	}
	// If the foor loop is completed without detecting a mice than should move, return false (nullptr)
	return false;
}

// Create and return an array with the coordinates of columns that contain members of a specific team
TArray<int32> AGrid::TeamColumns(int32 Team)
{
	TArray<int32> TeamColumnArray;

	for (int x = 0; x < 19; ++x)
	{
		for (int y = 0; y < 13; ++y)
		{
			FIntPoint NewPoint = FIntPoint(x, y);
			if (BlockMap.Contains(NewPoint))
			{
				ABlock* BoardPiece = Cast<ABlock>(BlockMap[NewPoint]);
				if (BoardPiece != nullptr)
				{
					// If actor is not a static block
					if (BoardPiece->ActorType == EType(Team))
					{
						TeamColumnArray.Add(x);
						break;
					}
				}
			}
		}
	}
	return TeamColumnArray;
}

// Check if any mice has reached the goal (change this to prevent performance bottleneck)
void AGrid::CheckGoal(int32 GoalPosition)
{
	for (int y = 0; y < 13; ++y)
	{
		ABlock* Mice;
		FIntPoint GoalPoint = FIntPoint(GoalPosition, y);
		if (BlockMap.Contains(GoalPoint))
		{
			Mice = Cast<ABlock>(BlockMap[GoalPoint]);
			if (Mice != nullptr)
			{
				EType type = Mice->ActorType;
				// If actor is not a static block
				if (type != EType::Block)
				{
					if (Mice->CanWalk())
					{
						Mice->MoveTo(FVector(Mice->GetActorLocation().X, Mice->GetActorLocation().Y, IterationOffset * -2), Mice->Coordinates.Y + 2);
						BlockMap.Remove(Mice->Coordinates);
					}
				}
			}
		}
	}
}

// Increment score of a chosen team
void AGrid::AddToScore(bool bIsBlue)
{
	if (bIsBlue)
	{
		BlueScore++;
	}
	else
	{
		RedScore++;
	}
}