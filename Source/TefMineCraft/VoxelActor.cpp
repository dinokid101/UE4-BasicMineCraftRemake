// Fill out your copyright notice in the Description page of Project Settings.

#include "TefMineCraft.h"
#include "VoxelActor.h"
#include "SimplexNoiseLibrary.h"

const int32 bTriangles[] = { 2, 1, 0, 0, 3, 2 };
const FVector2D bUVs[] = { FVector2D(0.000000, 0.000000), FVector2D(0.000000, 1.000000), FVector2D(1.000000, 1.000000), FVector2D(1.000000, 0.000000) };
const FVector bNormals0[] = { FVector(0, 0, 1), FVector(0, 0, 1), FVector(0, 0, 1), FVector(0, 0, 1) };
const FVector bNormals1[] = { FVector(0, 0, -1), FVector(0, 0, -1), FVector(0, 0, -1), FVector(0, 0, -1) };
const FVector bNormals2[] = { FVector(0, 1, 0), FVector(0, 1, 0), FVector(0, 1, 0), FVector(0, 1, 0) };
const FVector bNormals3[] = { FVector(0, -1, 0), FVector(0, -1, 0), FVector(0, -1, 0), FVector(0, -1, 0) };
const FVector bNormals4[] = { FVector(1, 0, 0), FVector(1, 0, 0), FVector(1, 0, 0), FVector(1, 0, 0) };
const FVector bNormals5[] = { FVector(-1, 0, 0), FVector(-1, 0, 0), FVector(-1, 0, 0), FVector(-1, 0, 0) };
const FVector bMask[] = { FVector(0.000000, 0.000000, 1.000000), FVector(0.000000, 0.000000, -1.000000), FVector(0.000000, 1.000000, 0.000000), FVector(0.000000, -1.000000, 0.000000), FVector(1.000000, 0.000000, 0.000000), FVector(-1.000000, 0.000000, 0.000000) };


// Sets default values
AVoxelActor::AVoxelActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVoxelActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVoxelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVoxelActor::OnConstruction(const FTransform & Transform)
{
	ChunkZElements = 80;
	ChunkLineElementsP2 = ChunkLineElements * ChunkLineElements;
	ChunkTotalElements = ChunkLineElementsP2 * ChunkZElements;
	VoxelSizeHalf = VoxelSize / 2;

	FString String = "Voxel_" + FString::FromInt(ChunkXIndex) + "_" + FString::FromInt(ChunkYIndex);
	FName Name = FName(*String);
	ProceduralComponent = NewObject<UProceduralMeshComponent>(this, Name);
	ProceduralComponent->RegisterComponent();

	RootComponent = ProceduralComponent;
	RootComponent->SetWorldTransform(Transform);

	Super::OnConstruction(Transform);

	bGenerateChunk();
	UpdateMesh();
}

void AVoxelActor::bGenerateChunk()
{
	FRandomStream RandomStream = FRandomStream(RandomSeed);
	TArray<FIntVector> TreeCenters;

	ChunkFields.SetNumUninitialized(ChunkTotalElements);

	TArray <int32> Noise = CalculateNoise();

	for (int32 x = 0; x < ChunkLineElements; x++)
	{
		for (int32 y = 0; y < ChunkLineElements; y++)
		{
			for (int32 z = 0; z < ChunkZElements; z++)
			{
				int32 Index = x + (y * ChunkLineElements) + (z * ChunkLineElementsP2);

				if (RandomStream.FRand() < 0.04 && z == 31 + Noise[x + y * ChunkLineElements])
					ChunkFields[Index] = -1;
				else if (z == 30 + Noise[x + y * ChunkLineElements])
					ChunkFields[Index] = 11;
				else if (z < 30 + Noise[x + y * ChunkLineElements] && z > 23 + Noise[x + y * ChunkLineElements])
					ChunkFields[Index] = 12;
				else if (z <= 23 + Noise[x + y * ChunkLineElements])
					ChunkFields[Index] =  13;
				else
					ChunkFields[Index] = 0;
			}
		}
	}

	for (int32 x = 2; x < ChunkLineElements - 2; x++)
	{
		for (int32 y = 2; y < ChunkLineElements - 2; y++)
		{
			for (int32 z = 0; z < ChunkZElements; z++)
			{
				if (RandomStream.FRand() < 0.01 && z == 31 + Noise[x + y * ChunkLineElements])
					TreeCenters.Add(FIntVector(x, y, z));
			}
		}
	}

	for (FIntVector TreeCenter : TreeCenters)
	{
		int32 TreeHeight = RandomStream.RandRange(3, 6);
		int32 RandX = RandomStream.RandRange(0, 2);
		int32 RandY = RandomStream.RandRange(0, 2);
		int32 RandZ = RandomStream.RandRange(0, 2);

		// Tree leaves
		for (int32 tree_x = -2; tree_x < 3; tree_x++)
		{
			for (int32 tree_y = -2; tree_y < 3; tree_y++)
			{
				for (int32 tree_z = -2; tree_z < 3; tree_z++)
				{
					if (InRange(tree_x + TreeCenter.X, ChunkLineElements) && InRange(tree_y + TreeCenter.Y, ChunkLineElements) && InRange(tree_z + TreeCenter.Z + TreeHeight, ChunkZElements))
					{
						float radius = FVector(tree_x * RandX, tree_y * RandY, tree_z * RandZ).Size();

						if (radius <= 2.8)
						{
							if (RandomStream.FRand() < 0.3 || radius <= 1.25)
							{
								ChunkFields[TreeCenter.X + tree_x + (ChunkLineElements * (TreeCenter.Y + tree_y)) + (ChunkLineElementsP2 * (TreeCenter.Z + tree_z + TreeHeight))] = 1;
							}
						}
					}
				}
			}
		}

		//Tree trunk
		for (int32 h = 0; h < TreeHeight; h++)
		{
			ChunkFields[TreeCenter.X + (TreeCenter.Y * ChunkLineElements) + ((TreeCenter.Z + h) * ChunkLineElementsP2)] = 14;
		}
	}
}

void AVoxelActor::UpdateMesh()
{
	TArray<FMeshSection> MeshSections;
	MeshSections.SetNum(Materials.Num());
	int32 El_Num = 0;

	for (int32 x = 0; x < ChunkLineElements; x++)
	{
		for (int32 y = 0; y < ChunkLineElements; y++)
		{
			for (int32 z = 0; z < ChunkZElements; z++)
			{
				int32 Index = x + (y * ChunkLineElements) + (z * ChunkLineElementsP2);
				int32 MeshIndex = ChunkFields[Index];

				if (MeshIndex > 0)
				{
					MeshIndex--;

					TArray<FVector> &Vertices = MeshSections[MeshIndex].Vertices;
					TArray<int32> &Triangles = MeshSections[MeshIndex].Triangles;
					TArray<FVector> &Normals = MeshSections[MeshIndex].Normals;
					TArray<FVector2D> &UVs = MeshSections[MeshIndex].UVs;
					TArray<FProcMeshTangent> &Tangents = MeshSections[MeshIndex].Tangents;
					TArray<FColor> &VertexColors = MeshSections[MeshIndex].VertexColors;
					int32 ElementID = MeshSections[MeshIndex].ElementID;

					int triangle_num = 0;
					for (int i = 0; i < 6; i++)
					{
						int NewIndex = Index + bMask[i].X + (bMask[i].Y * ChunkLineElements) + (bMask[i].Z * ChunkLineElementsP2);

						bool Flag = false;
						if (MeshIndex >= 20) Flag = true;
						else if ((x + bMask[i].X < ChunkLineElements) && (x + bMask[i].X >= 0) && (y + bMask[i].Y < ChunkLineElements) && (y + bMask[i].Y >= 0))
						{
							if (NewIndex < ChunkFields.Num() && NewIndex >= 0)
								if (ChunkFields[NewIndex] < 10) Flag = true;
						}
						else Flag = true;

						if (Flag)
						{
							Triangles.Add(bTriangles[0] + triangle_num + ElementID);
							Triangles.Add(bTriangles[1] + triangle_num + ElementID);
							Triangles.Add(bTriangles[2] + triangle_num + ElementID);
							Triangles.Add(bTriangles[3] + triangle_num + ElementID);
							Triangles.Add(bTriangles[4] + triangle_num + ElementID);
							Triangles.Add(bTriangles[5] + triangle_num + ElementID);
							triangle_num += 4;

							switch (i)
							{
							case 0: {
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));

								Normals.Append(bNormals0, ARRAY_COUNT(bNormals0));
								break;
							}
							case 1: {
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));

								Normals.Append(bNormals1, ARRAY_COUNT(bNormals1));
								break;
							}
							case 2: {
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));

								Normals.Append(bNormals2, ARRAY_COUNT(bNormals2));
								break;
							}
							case 3: {
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));

								Normals.Append(bNormals3, ARRAY_COUNT(bNormals3));
								break;
							}
							case 4: {
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));

								Normals.Append(bNormals4, ARRAY_COUNT(bNormals4));
								break;
							}
							case 5: {
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), -VoxelSizeHalf + (z * VoxelSize)));
								Vertices.Add(FVector(-VoxelSizeHalf + (x * VoxelSize), -VoxelSizeHalf + (y * VoxelSize), VoxelSizeHalf + (z * VoxelSize)));

								Normals.Append(bNormals5, ARRAY_COUNT(bNormals5));
								break;
							}
							}

							UVs.Append(bUVs, ARRAY_COUNT(bUVs));
							FColor color = FColor(255, 255, 255, i);
							VertexColors.Add(color); VertexColors.Add(color); VertexColors.Add(color); VertexColors.Add(color);
						}
					}
					El_Num += triangle_num;
					MeshSections[MeshIndex].ElementID += triangle_num;
				}
				else if (MeshIndex == -1)
				{
					AddInstanceVoxel(FVector(x * VoxelSize, y * VoxelSize, z * VoxelSize));
				}
			}
		}
	}

	ProceduralComponent->ClearAllMeshSections();
	for (int i = 0; i < MeshSections.Num(); i++)
	{
		if (MeshSections[i].Vertices.Num() > 0)
			ProceduralComponent->CreateMeshSection(i, MeshSections[i].Vertices, MeshSections[i].Triangles, MeshSections[i].Normals, MeshSections[i].UVs, MeshSections[i].VertexColors, MeshSections[i].Tangents, true);
	}

	int s = 0;
	while (s < Materials.Num())
	{
		ProceduralComponent->SetMaterial(s, Materials[s]);
		s++;
	}
}

TArray<int32> AVoxelActor::CalculateNoise()
{
	TArray<int32> Noise;
	Noise.Reserve(ChunkLineElementsP2);

	for (int32 y = 0; y < ChunkLineElements; y++)
	{
		for (int32 x = 0; x < ChunkLineElements; x++)
		{
			int32 XRef = ChunkXIndex * ChunkLineElements + x;
			int32 YRef = ChunkYIndex * ChunkLineElements + y;

			float NoiseVal = USimplexNoiseLibrary::SimplexNoise2D(XRef * 0.01f, YRef * 0.01) * 4;
			NoiseVal += USimplexNoiseLibrary::SimplexNoise2D(XRef * 0.01f, YRef * 0.01) * 8;
			NoiseVal += USimplexNoiseLibrary::SimplexNoise2D(XRef * 0.004f, YRef * 0.004) * 16;
			NoiseVal += FMath::Clamp(USimplexNoiseLibrary::SimplexNoise2D(XRef * 0.01f, YRef * 0.01), 0.0f, 5.0f) * 4;
			Noise.Add(FMath::FloorToInt(NoiseVal));
		}
	}
	return (Noise);
}

void AVoxelActor::SetVoxel(FVector LocalPos, int32 Value)
{
	int32 X = LocalPos.X / VoxelSize;
	int32 Y = LocalPos.Y / VoxelSize;
	int32 Z = LocalPos.Z / VoxelSize;

	int32 Index = X + (Y * ChunkLineElements) + (Z * ChunkLineElementsP2);

	ChunkFields[Index] = Value;

	UpdateMesh();
}

bool AVoxelActor::InRange(int32 value, int32 range)
{
	return (value >= 0 && value < range);
}

void AVoxelActor::AddInstanceVoxel_Implementation(FVector instanceLoc)
{
}
