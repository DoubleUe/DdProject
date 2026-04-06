#include "DdBaseCharacter.h"

#include "CharacterTrajectoryComponent.h"
#include "Components/DdCombatComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Table/Data/DdActionTable.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Table/Data/DdResourceTable.h"
#include "Table/Data/DdWeaponTable.h"
#include "Table/DdTableSubsystem.h"
#include "Util/DdUtil.h"
#include "Weapon/DdWeaponActor.h"

ADdBaseCharacter::ADdBaseCharacter()
{
	bReplicates = true;

	TrajectoryComponent = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("TrajectoryComponent"));
	CombatComponent = CreateDefaultSubobject<UDdCombatComponent>(TEXT("CombatComponent"));

	ApplyCameraCollisionIgnores();
	ApplyRotationModeFromState();
	ApplyWalkSpeedFromState();
}

void ADdBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyCameraCollisionIgnores();

	if (HasAuthority())
	{
		InitializeWeaponActor();
	}
}

void ADdBaseCharacter::Destroyed()
{
	DestroyWeaponActor();
	Super::Destroyed();
}

void ADdBaseCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyCameraCollisionIgnores();
}

UCharacterTrajectoryComponent* ADdBaseCharacter::GetTrajectoryComponent() const
{
	return TrajectoryComponent;
}

void ADdBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADdBaseCharacter, bUseControllerDesiredRotationMode);
	DOREPLIFETIME(ADdBaseCharacter, bUseSlowWalkSpeed);
}

void ADdBaseCharacter::SetMovementInputBlocked(bool bBlocked)
{
	bMovementInputBlocked = bBlocked;
}

void ADdBaseCharacter::SetAttackBlocked(bool bBlocked)
{
	bAttackBlocked = bBlocked;
}

void ADdBaseCharacter::ToggleRotationMode()
{
	const bool bNextUseControllerDesiredRotationMode = !bUseControllerDesiredRotationMode;
	SetUseControllerDesiredRotationMode(bNextUseControllerDesiredRotationMode);

	if (!HasAuthority())
	{
		ServerSetUseControllerDesiredRotationMode(bNextUseControllerDesiredRotationMode);
	}
}

void ADdBaseCharacter::ToggleWalkSpeed()
{
	const bool bNextUseSlowWalkSpeed = !bUseSlowWalkSpeed;
	SetUseSlowWalkSpeed(bNextUseSlowWalkSpeed);

	if (!HasAuthority())
	{
		ServerSetUseSlowWalkSpeed(bNextUseSlowWalkSpeed);
	}
}

void ADdBaseCharacter::ApplyCameraCollisionIgnores()
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

void ADdBaseCharacter::ApplyWalkSpeedFromState()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	CharacterMovementComponent->MaxWalkSpeed = bUseSlowWalkSpeed ? SlowWalkSpeed : NormalWalkSpeed;
}

void ADdBaseCharacter::SetUseSlowWalkSpeed(bool bInUseSlowWalkSpeed)
{
	bUseSlowWalkSpeed = bInUseSlowWalkSpeed;
	ApplyWalkSpeedFromState();
}

void ADdBaseCharacter::ApplyRotationModeFromState()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	CharacterMovementComponent->bOrientRotationToMovement = !bUseControllerDesiredRotationMode;
	CharacterMovementComponent->bUseControllerDesiredRotation = bUseControllerDesiredRotationMode;
}

void ADdBaseCharacter::SetUseControllerDesiredRotationMode(bool bInUseControllerDesiredRotationMode)
{
	bUseControllerDesiredRotationMode = bInUseControllerDesiredRotationMode;
	ApplyRotationModeFromState();
}

void ADdBaseCharacter::OnRep_UseControllerDesiredRotationMode()
{
	ApplyRotationModeFromState();
}

void ADdBaseCharacter::OnRep_UseSlowWalkSpeed()
{
	ApplyWalkSpeedFromState();
}

void ADdBaseCharacter::ServerSetUseControllerDesiredRotationMode_Implementation(bool bInUseControllerDesiredRotationMode)
{
	SetUseControllerDesiredRotationMode(bInUseControllerDesiredRotationMode);
}

void ADdBaseCharacter::ServerSetUseSlowWalkSpeed_Implementation(bool bInUseSlowWalkSpeed)
{
	SetUseSlowWalkSpeed(bInUseSlowWalkSpeed);
}

void ADdBaseCharacter::InitializeWeaponActor()
{
	DestroyWeaponActor();

	if (WeaponId <= 0)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to initialize weapon. GameInstance is null."), *GetName());
		return;
	}

	UDdTableSubsystem* TableSubsystem = GameInstance->GetSubsystem<UDdTableSubsystem>();
	if (TableSubsystem == nullptr || !TableSubsystem->IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to initialize weapon. TableSubsystem is not ready."), *GetName());
		return;
	}

	const FDdWeaponTableRow* WeaponRow = TableSubsystem->GetWeaponTable().FindRowById(WeaponId);
	if (WeaponRow == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to find weapon row for WeaponId %d."), *GetName(), WeaponId);
		return;
	}

	const FDdResourceTableRow* ResourceRow = TableSubsystem->GetResourceTable().FindRowById(WeaponRow->ResourceId);
	if (ResourceRow == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to find resource row for ResourceId %d."), *GetName(), WeaponRow->ResourceId);
		return;
	}

	UClass* WeaponActorClass = FDdUtil::LoadClassFromPath(ResourceRow->Path, ADdWeaponActor::StaticClass());
	if (WeaponActorClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to resolve weapon class from path: %s"), *GetName(), *ResourceRow->Path);
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedWeaponActor = GetWorld()->SpawnActor<ADdWeaponActor>(WeaponActorClass, GetActorTransform(), SpawnParameters);
	if (EquippedWeaponActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to spawn weapon actor for WeaponId %d."), *GetName(), WeaponId);
		return;
	}

	EquippedWeaponActor->PrepareForCharacterAttachment(this);
	AttachWeaponActorToCharacter(*WeaponRow, EquippedWeaponActor);

	// 무기의 콤보 데이터를 CombatComponent에 로드
	if (CombatComponent != nullptr && WeaponRow->ComboIds.Num() > 0)
	{
		CombatComponent->LoadComboData(WeaponRow->ComboIds, TableSubsystem);
	}
}

void ADdBaseCharacter::DestroyWeaponActor()
{
	if (EquippedWeaponActor == nullptr)
	{
		return;
	}

	if (EquippedWeaponActor->IsValidLowLevel())
	{
		EquippedWeaponActor->Destroy();
	}

	EquippedWeaponActor = nullptr;
}

void ADdBaseCharacter::AttachWeaponActorToCharacter(const FDdWeaponTableRow& WeaponRow, ADdWeaponActor* WeaponActor) const
{
	if (WeaponActor == nullptr || GetMesh() == nullptr || WeaponActor->GetRootComponent() == nullptr)
	{
		return;
	}

	const FName CharacterBoneName = WeaponRow.CharacterBoneName.IsEmpty() ? NAME_None : FName(*WeaponRow.CharacterBoneName);
	const FName WeaponBoneName = WeaponRow.WeaponBoneName.IsEmpty() ? NAME_None : FName(*WeaponRow.WeaponBoneName);
	const bool bHasCharacterSocket = CharacterBoneName != NAME_None && GetMesh()->DoesSocketExist(CharacterBoneName);

	const FTransform ParentAttachWorldTransform = bHasCharacterSocket
		? GetMesh()->GetSocketTransform(CharacterBoneName, RTS_World)
		: GetMesh()->GetComponentTransform();

	const USceneComponent* WeaponAttachComponent = WeaponActor->GetWeaponAttachComponent();
	FTransform WeaponAttachWorldTransform = WeaponActor->GetActorTransform();
	if (WeaponAttachComponent != nullptr)
	{
		const bool bHasWeaponSocket = WeaponBoneName != NAME_None && WeaponAttachComponent->DoesSocketExist(WeaponBoneName);
		WeaponAttachWorldTransform = bHasWeaponSocket
			? WeaponAttachComponent->GetSocketTransform(WeaponBoneName, RTS_World)
			: WeaponAttachComponent->GetComponentTransform();
	}

	const FTransform WeaponAttachToActorTransform = WeaponAttachWorldTransform.GetRelativeTransform(WeaponActor->GetActorTransform());
	const FTransform DesiredActorTransform = WeaponAttachToActorTransform.Inverse() * ParentAttachWorldTransform;

	WeaponActor->SetActorTransform(DesiredActorTransform);
	WeaponActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, bHasCharacterSocket ? CharacterBoneName : NAME_None);
}
