// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelActor.generated.h"

struct FMeshSection
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;

	int32 ElementID = 0;
};

UCLASS()
class TEFMINECRAFT_API AVoxelActor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AVoxelActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray <UMaterialInterface *> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 RandomSeed = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 VoxelSize = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 ChunkLineElements = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 ChunkXIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 ChunkYIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float XMult = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float YMult = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ZMult = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Weight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Freq = 1;

	UPROPERTY()
		int32 ChunkTotalElements;

	UPROPERTY()
		int32 ChunkZElements;

	UPROPERTY()
		int32 ChunkLineElementsP2;

	UPROPERTY()
		int32 ChunkLineElementsExt;

	UPROPERTY()
		int32 ChunkLineElementsExtP2;

	UPROPERTY()
		int32 VoxelSizeHalf;

	UPROPERTY()
		TArray <int32> ChunkFields;

	UPROPERTY()
		UProceduralMeshComponent* ProceduralComponent;

	UFUNCTION(BlueprintNativeEvent)
		void AddInstanceVoxel(FVector instanceLoc);
	virtual void AddInstanceVoxel_Implementation(FVector instanceLoc);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetVoxel(FVector LocalPos, int32 value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform & Transform) override;

private:

	void bGenerateChunk();

	void UpdateMesh();

	bool InRange(int32 value, int32 range);

	TArray <int32> CalculateNoise();

};
