#include "DdPlayerCharacter.h"

#include "Components/SceneComponent.h"
#include "Components/DdPlayerCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "Table/Data/DdResourceTable.h"
#include "Table/Data/DdWeaponTable.h"
#include "Table/DdTableSubsystem.h"
#include "Util/DdUtil.h"
#include "Weapon/DdWeaponActor.h"

ADdPlayerCharacter::ADdPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->bOrientRotationToMovement = true;
	CharacterMovementComponent->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	CharacterMovementComponent->JumpZVelocity = 500.0f;
	CharacterMovementComponent->AirControl = 0.35f;
	CharacterMovementComponent->MaxWalkSpeed = 500.0f;
	CharacterMovementComponent->MinAnalogWalkSpeed = 20.0f;
	CharacterMovementComponent->BrakingDecelerationWalking = 2000.0f;
	CharacterMovementComponent->BrakingDecelerationFalling = 1500.0f;

	PlayerCameraComp = CreateDefaultSubobject<UDdPlayerCameraComponent>(TEXT("PlayerCameraComp"));

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -97.0f), FRotator(0.0f, -90.0f, 0.0f));
}

void ADdPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitializeWeaponActor();
	}
}

void ADdPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PlayerCameraComp != nullptr)
	{
		PlayerCameraComp->UpdateCameraZoom(DeltaSeconds);
	}
}

void ADdPlayerCharacter::ApplyCameraZoomInput(const FInputActionValue& Value)
{
	if (PlayerCameraComp != nullptr)
	{
		PlayerCameraComp->ZoomCamera(Value.Get<float>());
	}
}

void ADdPlayerCharacter::ApplyMoveInput(const FInputActionValue& Value)
{
	if (Controller == nullptr || IsMovementInputBlocked())
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ADdPlayerCharacter::ApplyLookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (!FMath::IsNearlyZero(LookAxisVector.X))
	{
		AddControllerYawInput(LookAxisVector.X);
	}

	const float PitchInput = -LookAxisVector.Y;
	if (!FMath::IsNearlyZero(PitchInput))
	{
		AddControllerPitchInput(PitchInput);
	}
}

void ADdPlayerCharacter::Destroyed()
{
	DestroyWeaponActor();
	Super::Destroyed();
}

void ADdPlayerCharacter::TryAttack()
{
}

void ADdPlayerCharacter::InitializeWeaponActor()
{
	DestroyWeaponActor();

	if (WeaponId <= 0)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter failed to initialize weapon. GameInstance is null."));
		return;
	}

	UDdTableSubsystem* TableSubsystem = GameInstance->GetSubsystem<UDdTableSubsystem>();
	if (TableSubsystem == nullptr || !TableSubsystem->IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter failed to initialize weapon. TableSubsystem is not ready."));
		return;
	}

	const FDdWeaponTableRow* WeaponRow = TableSubsystem->GetWeaponTable().FindRowById(WeaponId);
	if (WeaponRow == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter failed to find weapon row for WeaponId %d."), WeaponId);
		return;
	}

	const FDdResourceTableRow* ResourceRow = TableSubsystem->GetResourceTable().FindRowById(WeaponRow->ResourceId);
	if (ResourceRow == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter failed to find resource row for ResourceId %d."), WeaponRow->ResourceId);
		return;
	}

	UClass* WeaponActorClass = FDdUtil::LoadClassFromPath(ResourceRow->Path, ADdWeaponActor::StaticClass());
	if (WeaponActorClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter failed to resolve weapon class from path: %s"), *ResourceRow->Path);
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedWeaponActor = GetWorld()->SpawnActor<ADdWeaponActor>(WeaponActorClass, GetActorTransform(), SpawnParameters);
	if (EquippedWeaponActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter failed to spawn weapon actor for WeaponId %d."), WeaponId);
		return;
	}

	EquippedWeaponActor->PrepareForCharacterAttachment(this);
	AttachWeaponActorToCharacter(*WeaponRow, EquippedWeaponActor);
}

void ADdPlayerCharacter::DestroyWeaponActor()
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
void ADdPlayerCharacter::AttachWeaponActorToCharacter(const FDdWeaponTableRow& WeaponRow, ADdWeaponActor* WeaponActor) const
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
