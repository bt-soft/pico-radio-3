#ifndef __MENU_SCREEN_H
#define __MENU_SCREEN_H

#include "ScreenManager.h"
#include "UIComponents.h"

// Menü elemek típusai
enum class MenuItemType {
    ACTION,  // Egyszerű művelet
    TOGGLE,  // Be/ki kapcsoló
    VALUE,   // Érték beállítás
    SUBMENU, // Almenü
    BACK     // Vissza gomb
};

// Menü elem struktúra
struct MenuItem {
    String text;
    MenuItemType type;
    std::function<void()> action;
    bool *toggleValue = nullptr;
    int *intValue = nullptr;
    int minValue = 0;
    int maxValue = 100;
    String subMenuName = "";

    MenuItem(const String &text, MenuItemType type, std::function<void()> action = nullptr) : text(text), type(type), action(action) {}
};

class MenuScreen : public Screen {
  private:
    std::vector<MenuItem> menuItems;
    std::shared_ptr<Panel> titlePanel;
    std::shared_ptr<Panel> menuPanel;
    std::shared_ptr<Label> titleLabel;
    std::vector<std::shared_ptr<Button>> menuButtons;
    int selectedIndex = 0;
    int scrollOffset = 0;
    static const int VISIBLE_ITEMS = 8;     // Increased to show all menu items including Back
    static const uint16_t ITEM_HEIGHT = 30; // Slightly smaller to fit more items
    static const uint16_t TITLE_HEIGHT = 40;
    static const uint16_t MARGIN = 5;

  public:
    MenuScreen(TFT_eSPI &tft, const String &title = "Menu") : Screen(tft, "MenuScreen") {
        createComponents(title);
        setupDefaultMenu();
    }
    virtual ~MenuScreen() = default;

    // Rotary encoder eseménykezelés felülírása
    virtual bool handleRotary(const RotaryEvent &event) override {
        DEBUG("MenuScreen handleRotary: direction=%d, button=%d\n", (int)event.direction, (int)event.buttonState);

        // Navigáció forgatással
        if (event.direction == RotaryEvent::Direction::Down) {
            navigateDown();
            return true;
        } else if (event.direction == RotaryEvent::Direction::Up) {
            navigateUp();
            return true;
        }

        // Kiválasztott elem aktiválása kattintással
        if (event.buttonState == RotaryEvent::ButtonState::Clicked) {
            activateSelected();
            return true;
        }

        // Ha nem kezeltük, továbbítjuk a szülő implementációnak (gyerekkomponenseknek)
        return Screen::handleRotary(event);
    } // Rotary encoder támogatás
    void navigateUp() {
        if (selectedIndex > 0) {
            int oldIndex = selectedIndex;
            selectedIndex--;
            updateSelectionColors(oldIndex, selectedIndex);
        }
    }

    void navigateDown() {
        if (selectedIndex < (int)menuItems.size() - 1) {
            int oldIndex = selectedIndex;
            selectedIndex++;
            updateSelectionColors(oldIndex, selectedIndex);
        }
    }

    void activateSelected() {
        if (selectedIndex >= 0 && selectedIndex < (int)menuItems.size()) {
            // Vizuális visszajelzés: gomb "megnyomása" rövid időre
            simulateButtonPress(selectedIndex);
            onMenuItemClicked(selectedIndex);
        }
    }

    // Menü elemek hozzáadása
    void addMenuItem(const MenuItem &item) {
        menuItems.push_back(item);
        updateMenuButtons();
    }

    void clearMenu() {
        menuItems.clear();
        updateMenuButtons();
    }

  protected:
    void createComponents(const String &title) {
        // Színsémák
        ColorScheme titleColors = ColorScheme::defaultScheme();
        titleColors.background = TFT_NAVY;
        titleColors.foreground = TFT_WHITE;
        titleColors.border = TFT_BLUE;

        ColorScheme panelColors = ColorScheme::defaultScheme();
        panelColors.background = TFT_BLACK;
        panelColors.border = TFT_DARKGREY;

        // Cím panel
        titlePanel = std::make_shared<Panel>(tft, Rect(0, 0, tft.width(), TITLE_HEIGHT), titleColors);

        // Cím label
        titleLabel = std::make_shared<Label>(tft, Rect(MARGIN, MARGIN, tft.width() - 2 * MARGIN, TITLE_HEIGHT - 2 * MARGIN), title, titleColors);
        titleLabel->setTextSize(2);
        titleLabel->setTextDatum(MC_DATUM);

        // Menü panel
        menuPanel = std::make_shared<Panel>(tft, Rect(0, TITLE_HEIGHT, tft.width(), tft.height() - TITLE_HEIGHT), panelColors);

        // Komponensek összekapcsolása
        titlePanel->addChild(titleLabel);
        addChild(titlePanel);
        addChild(menuPanel);
    }
    void setupDefaultMenu() {
        // Radio specific menu items (English labels for proper display)
        addMenuItem(MenuItem("FM/AM Switch", MenuItemType::ACTION, [this]() { onBandSwitch(); }));
        addMenuItem(MenuItem("Station Search", MenuItemType::ACTION, [this]() { onSeekStation(); }));
        addMenuItem(MenuItem("Frequency Settings", MenuItemType::ACTION, [this]() { onFrequencySettings(); }));
        addMenuItem(MenuItem("Volume", MenuItemType::ACTION, [this]() { onVolumeSettings(); }));
        addMenuItem(MenuItem("Station List", MenuItemType::ACTION, [this]() { onStationList(); }));
        addMenuItem(MenuItem("Configuration", MenuItemType::ACTION, [this]() { onConfiguration(); }));
        addMenuItem(MenuItem("Information", MenuItemType::ACTION, [this]() { onInformation(); }));
        addMenuItem(MenuItem("Back", MenuItemType::BACK, [this]() { onBack(); }));
    }
    void updateMenuButtons() {
        // Töröljük a régi gombokat
        for (auto &button : menuButtons) {
            menuPanel->removeChild(button);
        }
        menuButtons.clear();

        // Új gombok létrehozása
        for (int i = 0; i < std::min((int)menuItems.size(), VISIBLE_ITEMS); i++) {
            int itemIndex = i + scrollOffset;
            if (itemIndex >= menuItems.size())
                break;

            ColorScheme buttonColors = getButtonColors(itemIndex);

            auto button = std::make_shared<Button>(tft, Rect(MARGIN, TITLE_HEIGHT + i * ITEM_HEIGHT + MARGIN, tft.width() - 2 * MARGIN, ITEM_HEIGHT - MARGIN),
                                                   menuItems[itemIndex].text, buttonColors);

            button->setClickCallback([this, itemIndex]() { onMenuItemClicked(itemIndex); });

            menuButtons.push_back(button);
            menuPanel->addChild(button);
        }
    } // Optimalizált kiválasztás frissítés - csak a változott gombokat rajzolja újra
    void updateSelectionColors(int oldIndex, int newIndex) {
        // Scroll kezelése
        updateScrollIfNeeded(newIndex);

        // Ha mindkét index látható, csak azokat frissítjük
        int startVisible = scrollOffset;
        int endVisible = scrollOffset + VISIBLE_ITEMS - 1;

        // Régi kiválasztott gomb frissítése
        if (oldIndex >= startVisible && oldIndex <= endVisible && oldIndex >= 0) {
            int buttonIndex = oldIndex - scrollOffset;
            if (buttonIndex >= 0 && buttonIndex < menuButtons.size()) {
                ColorScheme oldColors = getButtonColors(oldIndex);
                menuButtons[buttonIndex]->setColorScheme(oldColors);
                menuButtons[buttonIndex]->markForRedraw();
            }
        }

        // Új kiválasztott gomb frissítése
        if (newIndex >= startVisible && newIndex <= endVisible) {
            int buttonIndex = newIndex - scrollOffset;
            if (buttonIndex >= 0 && buttonIndex < menuButtons.size()) {
                ColorScheme newColors = getButtonColors(newIndex);
                menuButtons[buttonIndex]->setColorScheme(newColors);
                menuButtons[buttonIndex]->markForRedraw();
            }
        }
    }

    // Scroll pozíció frissítése, ha szükséges
    void updateScrollIfNeeded(int selectedIndex) {
        int maxOffset = std::max(0, (int)menuItems.size() - VISIBLE_ITEMS);
        int oldScrollOffset = scrollOffset;

        // Ha a kiválasztott elem a látható terület felett van
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
        // Ha a kiválasztott elem a látható terület alatt van
        else if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
            scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
        }

        // Limitálás
        scrollOffset = std::max(0, std::min(scrollOffset, maxOffset));

        // Ha változott a scroll pozíció, újra kell építeni a menüt
        if (oldScrollOffset != scrollOffset) {
            updateMenuButtons();
        }
    }

    // Egyes gomb színének frissítése (pl. toggle állapot változáskor)
    void updateSingleButtonColor(int index) {
        int startVisible = scrollOffset;
        int endVisible = scrollOffset + VISIBLE_ITEMS - 1;

        if (index >= startVisible && index <= endVisible) {
            int buttonIndex = index - scrollOffset;
            if (buttonIndex >= 0 && buttonIndex < menuButtons.size()) {
                ColorScheme newColors = getButtonColors(index);
                menuButtons[buttonIndex]->setColorScheme(newColors);
                menuButtons[buttonIndex]->markForRedraw();
            }
        }
    }

    // Gomb megnyomás szimulálása rotary kattintáskor
    void simulateButtonPress(int index) {
        int startVisible = scrollOffset;
        int endVisible = scrollOffset + VISIBLE_ITEMS - 1;

        if (index >= startVisible && index <= endVisible) {
            int buttonIndex = index - scrollOffset;
            if (buttonIndex >= 0 && buttonIndex < menuButtons.size()) {
                auto button = menuButtons[buttonIndex];

                // Pressed állapot beállítása
                ColorScheme pressedColors = getButtonColors(index);
                pressedColors.background = pressedColors.pressedBackground;
                pressedColors.foreground = pressedColors.pressedForeground;

                button->setColorScheme(pressedColors);
                button->markForRedraw();

                // Azonnali rajzolás
                button->draw();

                // Rövid késleltetés (vizuális visszajelzés)
                delay(100);

                // Vissza normál állapotra
                ColorScheme normalColors = getButtonColors(index);
                button->setColorScheme(normalColors);
                button->markForRedraw();
            }
        }
    }
    ColorScheme getButtonColors(int itemIndex) {
        ColorScheme colors = ColorScheme::defaultScheme();

        // Normál elemek típus szerint
        const MenuItem &item = menuItems[itemIndex];
        switch (item.type) {
        case MenuItemType::BACK:
            colors.background = TFT_RED;
            colors.foreground = TFT_WHITE;
            colors.pressedBackground = TFT_MAROON;
            break;
        case MenuItemType::TOGGLE:
            if (item.toggleValue && *item.toggleValue) {
                colors.background = TFT_GREEN;
                colors.foreground = TFT_WHITE;
                colors.pressedBackground = TFT_DARKGREEN;
            } else {
                colors.background = TFT_DARKGREY;
                colors.foreground = TFT_LIGHTGREY;
                colors.pressedBackground = TFT_BLUE;
            }
            break;
        default:
            // ACTION és egyéb típusok
            colors.background = TFT_DARKGREY;
            colors.foreground = TFT_WHITE;
            colors.pressedBackground = TFT_BLUE;
            break;
        }

        // Kiválasztott elem jelölése csak finoman - világosabb keret
        if (itemIndex == selectedIndex) {
            colors.border = TFT_CYAN;
        } else {
            colors.border = TFT_DARKGREY;
        }

        return colors;
    }
    virtual void onActivate() override {
        DEBUG("MenuScreen activated\n");
        // Csak akkor építjük újra a menüt, ha még nincs létrehozva
        if (menuButtons.empty()) {
            updateMenuButtons();
        }
        // Csak a kiválasztott elem színét frissítjük
        updateSelectionColors(-1, selectedIndex);
        // Jelöljük az egész képernyőt újrarajzolásra, de ne töröljük a gombokat
        markForRedraw();
    }

    virtual void handleOwnLoop() override {
        // Menü animációk vagy időzített frissítések itt
    }

    virtual void drawSelf() override {
        if (needsRedraw) {
            tft.fillScreen(TFT_BLACK);
            needsRedraw = false;
            // Force all child components to redraw after clearing screen
            forceChildRedraw();
        }
    }

    // Helper method to force all child components to redraw
    void forceChildRedraw() {
        // Force the main panels to redraw and all their children
        if (titlePanel) {
            titlePanel->markForRedraw();
            if (titleLabel)
                titleLabel->markForRedraw();
        }
        if (menuPanel) {
            menuPanel->markForRedraw();
            // Force all menu buttons to redraw
            for (auto &button : menuButtons) {
                if (button)
                    button->markForRedraw();
            }
        }
    }

  private:
    void onMenuItemClicked(int index) {
        selectedIndex = index;

        if (index < menuItems.size()) {
            const MenuItem &item = menuItems[index];

            switch (item.type) {
            case MenuItemType::ACTION:
            case MenuItemType::BACK:
                if (item.action) {
                    item.action();
                }
                break;
            case MenuItemType::TOGGLE:
                if (item.toggleValue) {
                    *item.toggleValue = !(*item.toggleValue);
                    // Csak az adott gomb színét frissítjük
                    updateSingleButtonColor(index);
                }
                break;
            case MenuItemType::SUBMENU:
                if (screenManager && !item.subMenuName.isEmpty()) {
                    screenManager->switchToScreen(item.subMenuName);
                }
                break;

            default:
                break;
            }
        }
    } // Menü műveletek
    void onBandSwitch() {
        // FM/AM váltás
        // Itt implementálandó a si4735 sáv váltás logika
        // Példa: si4735.setAM() vagy si4735.setFM()
    }

    void onSeekStation() {
        // Automatikus állomás keresés
        // Itt implementálandó a si4735 seek funkció
        // Példa: si4735.seekStationProgress()
    }

    void onStationList() {
        // Előre beállított állomások listája
        // Itt lehetne egy állomáslista képernyőre váltani
        // screenManagerRef->switchToScreen("StationListScreen");
    }

    void onFrequencySettings() {
        // Itt lehetne egy frekvencia beállító képernyőre váltani
        // screenManagerRef->switchToScreen("FrequencyScreen");
    }
    void onVolumeSettings() {
        // Hangerő beállítások képernyőre váltás
        DEBUG("MenuScreen: Switching to VolumeScreen\n");
        if (screenManager) {
            screenManager->switchToScreen("VolumeScreen");
        }
    }

    void onConfiguration() {
        // Konfiguráció képernyő
        // screenManagerRef->switchToScreen("ConfigScreen");
    }
    void onInformation() {
        // Információ képernyőre váltás
        if (screenManager) {
            screenManager->switchToScreen("InfoScreen");
        }
    }
    void onBack() {
        DEBUG("MenuScreen: Back pressed, switching to MainScreen\n");
        if (screenManager) {
            screenManager->switchToScreen("MainScreen");
        }
    }
};

#endif // __MENU_SCREEN_H
