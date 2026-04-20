#include "Lab_SurvivorCharacter.h"
#include "Lab_PlayerState.h"
#include "Lab_PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ALab_SurvivorCharacter::ALab_SurvivorCharacter()
{
	bReplicates = true;
	SetReplicatingMovement(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ALab_SurvivorCharacter::BeginPlay()
{
	Super::BeginPlay();

	// PlayerState is available by BeginPlay on the server and listen server client.
	// Remote clients receive it shortly after via replication — binding here is fine
	// for the locally controlled pawn since its PlayerState arrives with the pawn.
	if (IsLocallyControlled())
	{
		if (ALab_PlayerState* PS = GetPlayerState<ALab_PlayerState>())
		{
			PS->OnSurvivorCaught.AddDynamic(this, &ALab_SurvivorCharacter::OnCaught);
		}
	}
}

void ALab_SurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// SetupPlayerInputComponent is called after the controller is fully assigned,
	// making it more reliable than BeginPlay for input and mode setup in multiplayer.
	if (ALab_PlayerController* PC = Cast<ALab_PlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
				Sub->AddMappingContext(DefaultMappingContext, 0);
			if (MouseLookMappingContext)
				Sub->AddMappingContext(MouseLookMappingContext, 1);
		}
		PC->EnableGameInputMode();
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALab_SurvivorCharacter::Move);
		if (LookAction)
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALab_SurvivorCharacter::Look);
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
	}
}

void ALab_SurvivorCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller) return;

	const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void ALab_SurvivorCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ALab_SurvivorCharacter::OnCaught()
{
	// TODO: show a "CAUGHT" widget or play a local camera effect here
	UE_LOG(LogTemp, Log, TEXT("ALab_SurvivorCharacter: local player was caught"));
}
