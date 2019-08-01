#include "MultiplayerGameInstance.h"

#include "Engine\Engine.h"
#include "Classes\GameFramework\PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint\UserWidget.h"

#include "PlatformTrigger.h"

UMultiplayerGameInstance::UMultiplayerGameInstance(const FObjectInitializer & ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MenuClassBP(TEXT("/Game/UI/MenuSystem/WBP_MainMenu"));
	if (!ensure(MenuClassBP.Class != nullptr)) return;
	/*UE_LOG(LogTemp, Warning, TEXT("Class found: %s"), *MenuClassBP.Class->GetName());*/
	MenuClass = MenuClassBP.Class;
}

void UMultiplayerGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Warning, TEXT("MenuWidget class : %s"), *MenuClass->GetName());
}

void UMultiplayerGameInstance::Host()
{
	UEngine * Engine = GetEngine();
	Engine->AddOnScreenDebugMessage(0, 1.5f, FColor::Green, TEXT("Hosting"));

	UWorld* world = GetWorld();
	if (!ensure(world != nullptr)) return;

	world->ServerTravel("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
}

void UMultiplayerGameInstance::Join(FString address)
{
	UEngine * Engine = GetEngine();
	FString string = "Joining " + address;
	Engine->AddOnScreenDebugMessage(0, 1.5f, FColor::Green, string);

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;
	PlayerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
}