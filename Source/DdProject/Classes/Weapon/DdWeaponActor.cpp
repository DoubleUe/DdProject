#include "Weapon/DdWeaponActor.h"

#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"

ADdWeaponActor::ADdWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	ApplyCameraCollisionIgnores();
}

void ADdWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	ApplyCameraCollisionIgnores();
	ApplyAttachmentCollisionSettings(GetOwner());
}

void ADdWeaponActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyCameraCollisionIgnores();
	ApplyAttachmentCollisionSettings(GetOwner());
}

void ADdWeaponActor::OnRep_Owner()
{
	Super::OnRep_Owner();

	ApplyAttachmentCollisionSettings(GetOwner());
}

USceneComponent* ADdWeaponActor::GetWeaponAttachComponent() const
{
	TArray<UMeshComponent*> MeshComponents;
	GetComponents(MeshComponents);

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent != nullptr)
		{
			return MeshComponent;
		}
	}

	return GetRootComponent();
}

void ADdWeaponActor::PrepareForCharacterAttachment(AActor* InOwnerActor)
{
	if (InOwnerActor != nullptr)
	{
		SetOwner(InOwnerActor);

		if (const APawn* OwnerPawn = Cast<APawn>(InOwnerActor))
		{
			SetInstigator(const_cast<APawn*>(OwnerPawn));
		}
	}

	ApplyAttachmentCollisionSettings(InOwnerActor);
}

void ADdWeaponActor::ApplyCameraCollisionIgnores()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr || PrimitiveComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
}

void ADdWeaponActor::ApplyAttachmentCollisionSettings(AActor* InOwnerActor)
{
	SetActorEnableCollision(false);

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		PrimitiveComponent->SetSimulatePhysics(false);
		PrimitiveComponent->SetEnableGravity(false);
		PrimitiveComponent->SetGenerateOverlapEvents(false);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

		if (InOwnerActor != nullptr)
		{
			PrimitiveComponent->IgnoreActorWhenMoving(InOwnerActor, true);
		}
	}

	if (InOwnerActor == nullptr)
	{
		return;
	}

	TArray<UPrimitiveComponent*> OwnerPrimitiveComponents;
	InOwnerActor->GetComponents(OwnerPrimitiveComponents);

	for (UPrimitiveComponent* OwnerPrimitiveComponent : OwnerPrimitiveComponents)
	{
		if (OwnerPrimitiveComponent != nullptr)
		{
			OwnerPrimitiveComponent->IgnoreActorWhenMoving(this, true);
		}
	}
}
