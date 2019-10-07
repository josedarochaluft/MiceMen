// Fill out your copyright notice in the Description page of Project Settings.


#include "ControllerPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Sets default values
AControllerPawn::AControllerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Automatically possess this pawn with main player input
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	//Create a camera
	UCameraComponent* GameCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	//Attach our camera to our root component. Offest and rotate the camera.
	GameCamera->SetupAttachment(RootComponent);
	GameCamera->SetRelativeLocation(LocationOffset);
	GameCamera->SetRelativeRotation(RotationOffset);

	SelectedColumn = 1;
	ErrorMargin = 0.75f;
	InputDelay = 0.0f;
	CurrentTeam = 1;
	ColumnIterator = 0;
}

// Called when the game starts or when spawned
void AControllerPawn::BeginPlay()
{
	Super::BeginPlay();
	PreviousColumn = 99;
	PreviousMovedColumn = 99;
	CurrentTeam = FMath::RandRange(1, 2);
}

// Called every frame
void AControllerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!GameBoard->bCanSettle)
	{
		InputDelay += 1.0f * DeltaTime;
	}
	else
	{
		InputDelay -= 1.0f * DeltaTime;
	}
	if (InputDelay > ErrorMargin && !bReady)
	{
		bReady = true;
		//UpdateText(GameBoard->BlueScore, GameBoard->RedScore);
		UpdateColumns();
		if (bDrawContdownBegan && TurnsBeforeDraw == 0)
		{
			bDraw = true;
		}
	}
	if (GameBoard->BlueScore == 11 && GameBoard->RedScore == 11)
	{
		bDrawContdownBegan = true;
	}
}

// Called to bind functionality to input
void AControllerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Up", IE_Pressed, this, &AControllerPawn::MoveUp);
	PlayerInputComponent->BindAction("Down", IE_Pressed, this, &AControllerPawn::MoveDown);
	PlayerInputComponent->BindAction("Right", IE_Pressed, this, &AControllerPawn::MoveRight);
	PlayerInputComponent->BindAction("Left", IE_Pressed, this, &AControllerPawn::MoveLeft);
	PlayerInputComponent->BindAction("Reset", IE_Pressed, this, &AControllerPawn::LevelReset);
}

void AControllerPawn::MoveUp()
{
	if (!bFinished && !bDraw)
	{
		MoveSelectedColumn(true);
		if (bDrawContdownBegan)
		{
			TurnsBeforeDraw--;
		}
	}
}

void AControllerPawn::MoveDown()
{
	if (!bFinished && !bDraw)
	{
		MoveSelectedColumn(false);
		if (bDrawContdownBegan)
		{
			TurnsBeforeDraw--;
		}
	}
}

void AControllerPawn::MoveRight()
{
	if (!bFinished && !bDraw)
	{
		if (bReady)
		{
			PreviousColumn = SelectedColumn;
			// If iterator is equal or lower than the minimum index
			if (ColumnIterator <= 0)
			{
				// Wrap around to max index
				ColumnIterator = TeamColumns.Num() - 1;
			}
			// Else decrement iterator
			else
			{
				ColumnIterator--;
			}
			SelectedColumn = TeamColumns[ColumnIterator];
			GameBoard->PaintColumn(SelectedColumn);
			GameBoard->PaintColumn(PreviousColumn);
		}
	}
}

void AControllerPawn::MoveLeft()
{
	if (!bFinished && !bDraw)
	{
		if (bReady)
		{
			PreviousColumn = SelectedColumn;
			// If iterator is equal or higher than the max index
			if (ColumnIterator >= TeamColumns.Num() - 1)
			{
				// Wrap around to minimum index
				ColumnIterator = 0;
			}
			// Else increment iterator
			else
			{
				ColumnIterator++;
			}
			SelectedColumn = TeamColumns[ColumnIterator];
			GameBoard->PaintColumn(SelectedColumn);
			GameBoard->PaintColumn(PreviousColumn);
		}
	}
}

void AControllerPawn::SwapActiveTeam()
{
	if (CurrentTeam == 1)
	{
		CurrentTeam = 2;
	}
	else
	{
		CurrentTeam = 1;
	}
	UpdateColumns();
}

void AControllerPawn::UpdateColumns()
{
	TeamColumns = GameBoard->TeamColumns(CurrentTeam);
	ColumnIterator = 0;
	PreviousColumn = SelectedColumn;
	RemoveInvalidColumn();
	SelectedColumn = TeamColumns[ColumnIterator];
	GameBoard->PaintColumn(SelectedColumn);
	if (bFirstUpdate)
	{
		bFirstUpdate = false;
	}
	else
	{
		GameBoard->PaintColumn(PreviousColumn);
	}
}

void AControllerPawn::RemoveInvalidColumn()
{
	// If the other team has moved at least once and my mice are in more than one column
	if (CurrentTeam == 1)
	{
		if (RedPreviousMoves.Num() > 0 && TeamColumns.Num() > 1)
		{
			for (int i = 0; i < TeamColumns.Num(); ++i)
			{
				// If selected column was moved in the other player's last move
				if (TeamColumns[i] == RedPreviousMoves[RedPreviousMoves.Num() - 1])
				{
					// Remove column from array of possible selections
					TeamColumns.RemoveAt(i);
				}
				// If the selected column was moved by me on my last 6 turns
				else if (CountEqualMoves(BluePreviousMoves, TeamColumns[i]) == 6)
				{
					// Remove column from array of possible selections
					TeamColumns.RemoveAt(i);
				}
			}
		}
	}
	else
	{
		if (BluePreviousMoves.Num() > 0 && TeamColumns.Num() > 1)
		{
			for (int i = 0; i < TeamColumns.Num(); ++i)
			{
				// If selected column was moved in the other player's last move
				if (TeamColumns[i] == BluePreviousMoves[BluePreviousMoves.Num() - 1])
				{
					// Remove column from array of possible selections
					TeamColumns.RemoveAt(i);
				}
				// If the selected column was moved by me on my last 6 turns
				else if (CountEqualMoves(RedPreviousMoves, TeamColumns[i]) == 6)
				{
					// Remove column from array of possible selections
					TeamColumns.RemoveAt(i);
				}
			}
		}
	}
	
}

void AControllerPawn::LevelReset()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("Level"), true);
}

void AControllerPawn::MoveSelectedColumn(bool bUpward)
{
	if (bReady)
	{
		bReady = false;
		InputDelay = 0.0f;

		// Perform operations on current team's array of previous moves
		if(CurrentTeam == 1)
		{
			if (BluePreviousMoves.Num() == 6)
			{
				BluePreviousMoves.RemoveAt(0);
			}
			BluePreviousMoves.Add(SelectedColumn);
		}
		else
		{
			if (RedPreviousMoves.Num() == 6)
			{
				RedPreviousMoves.RemoveAt(0);
			}
			RedPreviousMoves.Add(SelectedColumn);
		}
		GameBoard->MoveColumn(SelectedColumn, bUpward);
		SwapActiveTeam();
	}
}

int32 AControllerPawn::CountEqualMoves(TArray<int32> TeamMovesArray, int32 Move)
{
	int32 NumberOfEqualMoves = 0;
	for (int i = 0; i < TeamMovesArray.Num(); ++i)
	{
		if (TeamMovesArray[i] == Move)
		{
			NumberOfEqualMoves++;
		}
	}
	return NumberOfEqualMoves;
}

void AControllerPawn::FinishMatch()
{
	bFinished = true;
}

bool AControllerPawn::IsFinished()
{
	return bFinished;
}

bool AControllerPawn::DidMatchDraw()
{
	return bDraw;
}
