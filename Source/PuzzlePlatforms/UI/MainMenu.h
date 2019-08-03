#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class UButton;
class IMenuInterface;

UCLASS()
class PUZZLEPLATFORMS_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetMenuInterface(IMenuInterface* MenuInterface);

protected:

	virtual bool Initialize() override;

private:

	UFUNCTION()
	void HostButtonClick();

	UFUNCTION()
	void JoinButtonClick();

	UPROPERTY(meta = (BindWidget))
	UButton* Host;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Join;

	IMenuInterface* MenuInterface;

};