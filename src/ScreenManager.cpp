#include "ScreenManager.h"
#include "FMSceen.h"

void ScreenManager::registerDefaultScreenFactories() {
    // MainScreen factory
    registerScreenFactory(FMScreen::SCREEN_NAME, [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<FMScreen>(tft); });

    // // MenuScreen factory
    // registerScreenFactory("MenuScreen", [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<MenuScreen>(tft, "Main Menu sanyi"); });

    // // InfoScreen factory
    // registerScreenFactory("InfoScreen", [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<InfoScreen>(tft); }); // VolumeScreen factory
    // registerScreenFactory("VolumeScreen", [](TFT_eSPI &tft) -> std::shared_ptr<UIScreen> { return std::make_shared<VolumeScreen>(tft); });
}