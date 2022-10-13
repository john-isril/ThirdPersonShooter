// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "MainCharacter.h"

// Sets default values
AItem::AItem() :
	ItemName{ TEXT("Default Item") }, Amount{ 0 }, ItemState{ EItemState::EIS_Pickup },
	ItemType{ EItemType::EIT_MAX }, MaterialIndex{ 0 }, InventoryIndex{ 0 },
	RunningTime{ 0.0f }, InitialLocation{}
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	WidgetSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	WidgetSphere->SetupAttachment(GetRootComponent());

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(GetRootComponent());
}

void AItem::SetItemState(EItemState newItemState)
{
	ItemState = newItemState;
	SetItemProperties(newItemState);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	WidgetSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnWidgetSphereOverlap);
	WidgetSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnWidgetSphereEndOverlap);
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnPickupSphereOverlap);
	PickupSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnPickupSphereEndOverlap);

	SetItemProperties(ItemState);
	InitializeCustomDepth();

	InitialLocation = GetActorLocation();
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		Mesh->SetVisibility(true);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

		// needs to check if player overlaps
		WidgetSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		WidgetSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		EnableGlowMaterial();
		EnableCustomDepth();

		break;

	case EItemState::EIS_IdleInventory:
		Mesh->SetVisibility(true);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

		WidgetSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WidgetSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetPickupWidget()->SetVisibility(false);
		DisableGlowMaterial();
		DisableCustomDepth();
		break;

	case EItemState::EIS_Equipped:
		Mesh->SetVisibility(true);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

		WidgetSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WidgetSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetPickupWidget()->SetVisibility(false);

		DisableGlowMaterial();
		DisableCustomDepth();

		break;

	case EItemState::EIS_Falling:
		Mesh->SetVisibility(true);
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		WidgetSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WidgetSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_MAX:

		break;

	default:

		break;
	}
}

void AItem::OnWidgetSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (Cast<AMainCharacter>(OtherActor))
		{
			GetPickupWidget()->SetVisibility(true);
		}
	}

	PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AItem::OnWidgetSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		if (Cast<AMainCharacter>(OtherActor))
		{
			GetPickupWidget()->SetVisibility(false);
		}
	}

	PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItem::OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMainCharacter* Character_{ Cast<AMainCharacter>(OtherActor) };
		if (Character_)
		{
			Character_->SetOverlappedPickupItem(this);
		}
	}
}

void AItem::OnPickupSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMainCharacter* Character_{ Cast<AMainCharacter>(OtherActor) };
		if (Character_)
		{
			Character_->SetOverlappedPickupItem(nullptr);
		}
	}
}

void AItem::PickupSpinBounce(float DeltaTime)
{
	if (ItemState == EItemState::EIS_Pickup)
	{
		FRotator ItemRotation{ GetActorRotation() };
		ItemRotation.Yaw += (200.0f * DeltaTime);

		if (RunningTime > 360.0f)
		{
			RunningTime = 0.0f;
		}

		FVector Wave {0.0f, 0.0f, (BounceSineWaveAmplitude * FMath::Sin(RunningTime) + BounceSineWaveAmplitude) };

		SetActorRotation(ItemRotation);
		SetActorLocation(InitialLocation + Wave);

		RunningTime += (DeltaTime * 5.0f);
	}
}

void AItem::GetPickedUp()
{
}


void AItem::InitializeCustomDepth()
{
	EnableCustomDepth();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	if (MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		Mesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
		DisableGlowMaterial();
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PickupSpinBounce(DeltaTime);
}

void AItem::DisableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
	}
}

void AItem::EnableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 2);
	}
}

void AItem::EnableCustomDepth()
{
	Mesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	Mesh->SetRenderCustomDepth(false);
}

