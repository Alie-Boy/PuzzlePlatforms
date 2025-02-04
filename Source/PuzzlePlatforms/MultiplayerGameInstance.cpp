#include "MultiplayerGameInstance.h"

#include "Engine\Engine.h"
#include "Classes\GameFramework\PlayerController.h"
#include "UObject\ConstructorHelpers.h"
#include "Blueprint\UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterface.h"

#include "UI\MainMenu.h"
#include "UI\PauseMenu.h"
#include "PlatformTrigger.h"

UMultiplayerGameInstance::UMultiplayerGameInstance(const FObjectInitializer & ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MenuClassBP(TEXT("/Game/UI/MenuSystem/WBP_MainMenu"));
	if (!ensure(MenuClassBP.Class != nullptr)) return;
	static ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuClassBP(TEXT("/Game/UI/MenuSystem/WBP_PauseMenu"));
	if (!ensure(PauseMenuClassBP.Class != nullptr)) return;
	UE_LOG(LogTemp, Warning, TEXT("Class found: %s"), *MenuClassBP.Class->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Class found: %s"), *PauseMenuClassBP.Class->GetName());
	MenuClass = MenuClassBP.Class;
	PauseMenuClass = PauseMenuClassBP.Class;
}

void UMultiplayerGameInstance::Init()
{
	Super::Init();
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS == nullptr) return;
	UE_LOG(LogTemp, Warning, TEXT("OSS pointer : %s"), *OSS->GetSubsystemName().ToString());
	SessionInterface = OSS->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerGameInstance::OnCreateSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayerGameInstance::OnCreateSessionComplete);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerGameInstance::OnFindSessionsComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerGameInstance::OnJoinSessionComplete);

		OnlineSessionSearch = MakeShareable(new FOnlineSessionSearch());
	}
}

void UMultiplayerGameInstance::Host()
{
	HostServer();
}

void UMultiplayerGameInstance::Join(uint32 Index)
{
	JoinServer(Index);
}

void UMultiplayerGameInstance::QuitToMenu()
{
	UWorld* world = GetWorld();
	if (!ensure(world != nullptr)) return;

	world->ServerTravel("/Game/Maps/MainMenu");
}

void UMultiplayerGameInstance::RefreshServerList()
{
	FindSessions();
}

void UMultiplayerGameInstance::LoadMenu()
{
	if (!ensure(MenuClass != nullptr)) return;
	MainMenu = CreateWidget<UMainMenu>(this, MenuClass);
	MainMenu->Setup();

	MainMenu->SetMenuInterface(this);
}

void UMultiplayerGameInstance::LoadPauseMenu()
{
	if (!ensure(PauseMenuClass != nullptr)) return;
	UPauseMenu* PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuClass);
	PauseMenu->Setup();

	PauseMenu->SetMenuInterface(this);
}

void UMultiplayerGameInstance::HostServer()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(FName("My Session"));
		if (ExistingSession) {
			SessionInterface->DestroySession(FName("My Session"));
		}
		else { 
			CreateSession(); 
		}
	}
}

void UMultiplayerGameInstance::CreateSession()
{
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 2;
	SessionInterface->CreateSession(0, FName("My Session"), SessionSettings);
}

void UMultiplayerGameInstance::FindSessions()
{
	if (OnlineSessionSearch.IsValid())
	{
		OnlineSessionSearch->bIsLanQuery = true;

		UE_LOG(LogTemp, Warning, TEXT("Starting session search"));
		SessionInterface->FindSessions(0, OnlineSessionSearch.ToSharedRef());
	}
}

void UMultiplayerGameInstance::OnCreateSessionComplete(FName SessionName, bool isCompleted)
{
	if (!isCompleted) return;
	UEngine * Engine = GetEngine();
	Engine->AddOnScreenDebugMessage(0, 1.5f, FColor::Green, TEXT("Hosting"));

	UWorld* world = GetWorld();
	if (!ensure(world != nullptr)) return;

	world->ServerTravel("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
}

void UMultiplayerGameInstance::OnDestroySessionComplete(FName SessionName, bool isCompleted)
{
	if (!isCompleted) return;
	CreateSession();
}

void UMultiplayerGameInstance::OnFindSessionsComplete(bool isCompleted)
{
	if (!isCompleted) return;

	TArray<FString> ServerNames;
	ServerNames.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Finding sessions completed."));
	auto SearchResults = OnlineSessionSearch->SearchResults;
	if (SearchResults.Num() > 0)
	{
		for (auto &i : SearchResults)
		{
			ServerNames.Emplace(i.GetSessionIdStr());
			UE_LOG(LogTemp, Warning, TEXT("Session: %s"), *i.GetSessionIdStr());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Sessions found."));
	}
	MainMenu->SetServerList(ServerNames);
}

void UMultiplayerGameInstance::JoinServer(uint32 Index)
{
	if (!SessionInterface.IsValid())  return;
	if (!OnlineSessionSearch.IsValid()) return;

	SessionInterface->JoinSession(0, FName("My Session"), OnlineSessionSearch->SearchResults[Index]);
	
}

void UMultiplayerGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UEngine * Engine = GetEngine();
	FString address;
	SessionInterface->GetResolvedConnectString(SessionName, address);
	FString string = "Joining " + address;
	Engine->AddOnScreenDebugMessage(0, 1.5f, FColor::Green, string);
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;
	PlayerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
}