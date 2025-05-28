#ifndef __CONFIG_H
#define __CONFIG_H

#include "DebugDataInspector.h"  // Szükséges a debug kiíratáshoz
// #include "MiniAudioFft.h"  // Szükséges a MiniAudioFft::DisplayMode enumhoz
#include "StoreBase.h"

// --------------------------------
// Konfig struktúra típusdefiníció
struct Config_t {
  //-- Band
  uint8_t bandIdx;
  // uint16_t currentFreq;
  // uint8_t currentStep;
  // uint8_t currentMode;

  // BandWidht
  uint8_t bwIdxAM;
  uint8_t bwIdxFM;
  uint8_t bwIdxMW;
  uint8_t bwIdxSSB;

  // Step
  uint8_t ssIdxMW;
  uint8_t ssIdxAM;
  uint8_t ssIdxFM;

  // BFO
  int currentBFO;
  uint8_t currentBFOStep;  // BFO lépésköz (pl. 1, 10, 25 Hz)
  int currentBFOmanu;      // Manuális BFO eltolás (pl. -999 ... +999 Hz)

  // Squelch
  uint8_t currentSquelch;
  bool squelchUsesRSSI;  // A squlech RSSI alapú legyen?

  // FM RDS
  bool rdsEnabled;

  // Hangerő
  uint8_t currVolume;

  // AGC
  uint8_t agcGain;
  uint8_t currentAGCgain;

  //--- TFT
  uint16_t tftCalibrateData[5];       // TFT touch kalibrációs adatok
  uint8_t tftBackgroundBrightness;    // TFT Háttérvilágítás
  bool tftDigitLigth;                 // Inaktív szegmens látszódjon?
  uint8_t screenSaverTimeoutMinutes;  // Képernyővédő ideje percekben (1-30)
  bool beeperEnabled;                 // Hangjelzés engedélyezése

  // MiniAudioFft módok
  uint8_t miniAudioFftModeAm;  // MiniAudioFft módja AM képernyőn
  uint8_t miniAudioFftModeFm;  // MiniAudioFft módja FM képernyőn

  // MiniAudioFft erősítés
  float miniAudioFftConfigAm;  // -1.0f: Disabled, 0.0f: Auto, >0.0f: Manual
                               // Gain Factor
  float miniAudioFftConfigFm;  // -1.0f: Disabled, 0.0f: Auto, >0.0f: Manual
                               // Gain Factor

  float miniAudioFftConfigAnalyzer;  // MiniAudioFft erősítés konfigurációja az
                                     // Analyzerhez
  float miniAudioFftConfigRtty;      // MiniAudioFft erősítés konfigurációja az
                                     // RTTY-hez

  uint16_t cwReceiverOffsetHz;  // CW vételi eltolás Hz-ben
};

// Alapértelmezett konfigurációs adatok (readonly, const)
extern const Config_t DEFAULT_CONFIG;

/**
 * Konfigurációs adatok kezelése
 */
class Config : public StoreBase<Config_t> {
 public:
  // A 'config' változó, alapértelmezett értékeket veszi fel a konstruktorban
  // Szándékosan public, nem kell a sok getter egy embedded rendszerben
  Config_t data;

 protected:
  const char* getClassName() const override {
    return "Config";
  } /**
     * Referencia az adattagra, csak az ős használja
     */
  Config_t& getData() override { return data; };

  /**
   * Const referencia az adattagra, CRC számításhoz
   */
  const Config_t& getData() const override { return data; };

  // Felülírjuk a mentést/betöltést a debug kiíratás hozzáadásához
  uint16_t performSave() override {
    uint16_t savedCrc =
        StoreEepromBase<Config_t>::save(getData(), 0, getClassName());
#ifdef __DEBUG
    if (savedCrc != 0) {
      DebugDataInspector::printConfigData(getData());
    }
#endif
    return savedCrc;
  }

  uint16_t performLoad() override {
    uint16_t loadedCrc =
        StoreEepromBase<Config_t>::load(getData(), 0, getClassName());
#ifdef __DEBUG
    DebugDataInspector::printConfigData(
        getData());  // Akkor is kiírjuk, ha defaultot töltött
#endif
    uint8_t currentTimeout = data.screenSaverTimeoutMinutes;
    if (currentTimeout < SCREEN_SAVER_TIMEOUT_MIN ||
        currentTimeout > SCREEN_SAVER_TIMEOUT_MAX) {
      data.screenSaverTimeoutMinutes = SCREEN_SAVER_TIMEOUT;
      // A 'data' módosítása miatt a checkSave() később észlelni fogja az
      // eltérést a lastCRC-hez képest (amit a loadedCrc alapján állít be a
      // StoreBase), és menteni fogja a javított adatot.
    }
    return loadedCrc;
  }

 public:
  /**
   * Konstruktor
   * @param pData Pointer a konfigurációs adatokhoz
   */
  Config() : StoreBase<Config_t>(), data(DEFAULT_CONFIG) {}

  /**
   * Alapértelmezett adatok betöltése
   */
  void loadDefaults() override {
    memcpy(&data, &DEFAULT_CONFIG, sizeof(Config_t));
    analogWrite(PIN_TFT_BACKGROUND_LED,
                data.tftBackgroundBrightness);  // Háttérvilágítás beállítása
  }
};

// Globális config példány deklaráció
extern Config config;

#endif  // __CONFIG_H
