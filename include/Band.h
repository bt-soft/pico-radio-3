#ifndef __BAND_H
#define __BAND_H

#include <SI4735.h>

#include "Config.h"
#include "defines.h"
#include "rtVars.h"

// Band index
#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

// DeModulation types
#define FM 0
#define LSB 1
#define USB 2
#define AM 3
#define CW 4

// BandTable állandó része (PROGMEM-ben tároljuk)
struct BandTableConst {
    const char *bandName;  // Sáv neve (PROGMEM mutató)
    uint8_t bandType;      // Sáv típusa (FM, MW, LW vagy SW)
    uint16_t prefMod;      // Preferált moduláció (AM, FM, USB, LSB, CW)
    uint16_t minimumFreq;  // A sáv minimum frekvenciája
    uint16_t maximumFreq;  // A sáv maximum frekvenciája
    uint16_t defFreq;      // Alapértelmezett frekvencia vagy aktuális frekvencia
    uint8_t defStep;       // Alapértelmezett lépésköz (növelés/csökkentés)
    bool isHam;            // HAM sáv-e?
};

// BandTable változó része (RAM-ban tároljuk)
struct BandTableVar {
    uint16_t currFreq;    // Aktuális frekvencia
    uint8_t currStep;     // Aktuális lépésköz (növelés/csökkentés)
    uint8_t currMod;      // Aktuális mód/modulációs típus (FM, AM, LSB, USB, CW)
    uint16_t antCap;      // Antenna Tuning Capacitor
    int16_t lastBFO;      // Utolsó BFO érték
    int16_t lastmanuBFO;  // Utolsó manuális BFO érték X-Tal segítségével
};

// A kombinált struktúra az állandó és változó adatok összekapcsolásával
struct BandTable {
    const BandTableConst *pConstData;  // PROGMEM-beli állandó adatok
    BandTableVar varData;              // RAM-ban tárolt változó adatok
};

// Sávszélesség struktúra (Címke és Érték)
struct BandWidth {
    const char *label;  // Megjelenítendő felirat
    uint8_t index;      // Az si4735-nek átadandó index
};

// Lépésköz struktúra (Címke és Érték)
struct FrequencyStep {
    const char *label;  // Megjelenítendő felirat (pl. "1kHz")
    uint8_t value;      // A lépésköz értéke (pl. 1, 5, 9, 10, 100)
};

/**
 * Band class
 */
class Band {
   private:
    // Si4735 referencia
    SI4735 &si4735;

    // Config referencia
    Config &configRef;

    // SSB betöltve?
    bool ssbLoaded = false;

    void setBandWidth();
    void loadSSB();

   public:
    // BandMode description
    static const char *bandModeDesc[5];

    // Sávszélesség struktúrák tömbjei
    static const BandWidth bandWidthFM[5];
    static const BandWidth bandWidthAM[7];
    static const BandWidth bandWidthSSB[6];

    // Lépésköz konfigurációk (érték beállításához)
    static const FrequencyStep stepSizeAM[4];
    static const FrequencyStep stepSizeFM[3];

    static const FrequencyStep stepSizeBFO[4];

    Band(SI4735 &si4735, Config &configRef);
    virtual ~Band() = default;

    /**
     *
     */
    void bandInit(bool sysStart = false);

    /**
     * Band beállítása
     * @param useDefaults default adatok betültése?
     */
    void bandSet(bool useDefaults = false);

    /**
     * A Default Antenna Tuning Capacitor értékének lekérdezése
     * @return Az alapértelmezett antenna tuning capacitor értéke
     */
    inline uint16_t getDefaultAntCapValue() {

        // Kikeressük az aktuális Band rekordot
        switch (getCurrentBandType()) {

            case SW_BAND_TYPE:
                return 1;  // SW band esetén antenna tuning capacitor szükséges

            case FM_BAND_TYPE:
            case MW_BAND_TYPE:
            case LW_BAND_TYPE:
            default:
                return 0;  // FM és sima AM esetén nem kell antenna tuning capacitor
        }
    }

    /**
     * Band beállítása
     */
    void useBand();

    /**
     * A Band egy rekordjának elkérése az index alapján
     * @param bandIdx a band indexe
     * @return A BandTableVar rekord referenciája, vagy egy üres rekord, ha nem található
     */
    BandTable &getBandByIdx(uint8_t bandIdx);

    /**
     *
     */
    inline BandTable &getCurrentBand() { return getBandByIdx(configRef.data.bandIdx); }

    /**
     * A Band indexének elkérése a bandName alapján
     *
     * @param bandName A keresett sáv neve
     * @return A BandTable rekord indexe, vagy -1, ha nem található
     */
    int8_t getBandIdxByBandName(const char *bandName);

    /**
     * Demodulációs mód index szerinti elkérése (FM, AM, LSB, USB, CW)
     * @param demodIndex A demodulációs mód indexe
     */
    inline const char *getBandModeDescByIndex(uint8_t demodIndex) { return bandModeDesc[demodIndex]; }

    /**
     * Aktuális mód/modulációs típus (FM, AM, LSB, USB, CW)
     */
    inline const char *getCurrentBandModeDesc() { return bandModeDesc[getCurrentBand().varData.currMod]; }

    /**
     * A lehetséges AM demodulációs módok kigyűjtése
     */
    inline const char **getAmDemodulationModes(uint8_t &count) {
        // count = sizeof(bandModeDesc) / sizeof(bandModeDesc[0]) - 1;
        count = ARRAY_ITEM_COUNT(bandModeDesc) - 1;
        return &bandModeDesc[1];
    }

    /**
     * Az aktuális sávszélesség labeljének lekérdezése
     * @return A sávszélesség labelje, vagy nullptr, ha nem található
     */
    const char *getCurrentBandWidthLabel() {
        const char *p;
        uint8_t currMod = getCurrentBand().varData.currMod;
        if (currMod == AM) p = getCurrentBandWidthLabelByIndex(bandWidthAM, configRef.data.bwIdxAM);
        if (currMod == LSB or currMod == USB or currMod == CW) p = getCurrentBandWidthLabelByIndex(bandWidthSSB, configRef.data.bwIdxSSB);
        if (currMod == FM) p = getCurrentBandWidthLabelByIndex(bandWidthFM, configRef.data.bwIdxFM);

        return p;
    }

    /**
     * Sávszélesség tömb labeljeinek visszaadása
     * @param bandWidth A sávszélesség tömbje
     * @param count A tömb elemeinek száma
     * @return A label-ek tömbje
     */
    template <size_t N>
    const char **getBandWidthLabels(const BandWidth (&bandWidth)[N], size_t &count) {
        count = N;  // A tömb mérete
        static const char *labels[N];
        for (size_t i = 0; i < N; i++) {
            labels[i] = bandWidth[i].label;
        }
        return labels;
    }

    /**
     * A sávszélesség labeljének lekérdezése az index alapján
     * @param bandWidth A sávszélesség tömbje
     * @param index A keresett sávszélesség indexe
     * @return A sávszélesség labelje, vagy nullptr, ha nem található
     */
    template <size_t N>
    const char *getCurrentBandWidthLabelByIndex(const BandWidth (&bandWidth)[N], uint8_t index) {
        for (size_t i = 0; i < N; i++) {
            if (bandWidth[i].index == index) {
                return bandWidth[i].label;  // Megtaláltuk a labelt
            }
        }
        return nullptr;  // Ha nem található
    }

    /**
     * A sávszélesség indexének lekérdezése a label alapján
     * @param bandWidth A sávszélesség tömbje
     * @param label A keresett sávszélesség labelje
     * @return A sávszélesség indexe, vagy -1, ha nem található
     */
    template <size_t N>
    int8_t getBandWidthIndexByLabel(const BandWidth (&bandWidth)[N], const char *label) {
        for (size_t i = 0; i < N; i++) {
            if (strcmp(label, bandWidth[i].label) == 0) {
                return bandWidth[i].index;  // Megtaláltuk az indexet
            }
        }
        return -1;  // Ha nem található
    }

    /**
     * Lépésköz tömb labeljeinek visszaadása
     * @param bandWidth A lépésköz tömbje
     * @param count A tömb elemeinek száma
     * @return A label-ek tömbje
     */
    template <size_t N>
    const char **getStepSizeLabels(const FrequencyStep (&stepSizeTable)[N], size_t &count) {
        count = N;  // A tömb mérete
        static const char *labels[N];
        for (size_t i = 0; i < N; i++) {
            labels[i] = stepSizeTable[i].label;
        }
        return labels;
    }

    /**
     * A lépésköz értékének lekérdezése az index alapján
     * @param bandWidth A lépésköz tömbje
     * @param index A keresett lépésköz indexe
     * @return A lépésköz labelje, vagy nullptr, ha nem található
     */
    template <size_t N>
    const uint16_t getStepSizeByIndex(const FrequencyStep (&stepSizeTable)[N], uint8_t index) {
        // Ellenőrizzük, hogy az index érvényes-e a tömbhöz
        if (index < N) {
            return stepSizeTable[index].value;  // Közvetlenül visszaadjuk a labelt az index alapján
        }

        return 0;  // Ha az index érvénytelen
    }

    /**
     * A lépésköz labeljének lekérdezése az index alapján
     * @param bandWidth A lépésköz tömbje
     * @param index A keresett lépésköz indexe
     * @return A lépésköz labelje, vagy nullptr, ha nem található
     */
    template <size_t N>
    const char *getStepSizeLabelByIndex(const FrequencyStep (&stepSizeTable)[N], uint8_t index) {
        // Ellenőrizzük, hogy az index érvényes-e a tömbhöz
        if (index < N) {
            return stepSizeTable[index].label;  // Közvetlenül visszaadjuk a labelt az index alapján
        }
        return nullptr;  // Ha az index érvénytelen
    }

    /**
     * Aktuális frekvencia lépésköz felirat megszerzése
     */
    const char *currentStepSizeStr() {

        // Statikus buffer a formázott string tárolására
        static char formattedStepStr[10];  // Elég nagynak kell lennie (pl. "100Hz" + '\0')

        // BFO esetén az érték az érték :')
        if (rtv::bfoOn) {
            snprintf(formattedStepStr, sizeof(formattedStepStr), "%dHz", configRef.data.currentBFOStep);
            return formattedStepStr;  // Visszaadjuk a buffer pointerét
        }

        const char *currentStepStr = nullptr;
        BandTable &currentBand = getCurrentBand();
        uint8_t currentBandType = currentBand.pConstData->bandType;  // Kikeressük az aktuális Band típust

        if (currentBandType == FM_BAND_TYPE) {
            currentStepStr = getStepSizeLabelByIndex(Band::stepSizeFM, configRef.data.ssIdxFM);

        } else {  // Nem FM

            // Ha SSB vagy CW, akkor a lépésköz a BFO-val van megoldva
            if (currentBand.varData.currMod == LSB or currentBand.varData.currMod == USB or currentBand.varData.currMod == CW) {
                switch (rtv::freqstepnr) {
                    default:
                    case 0:
                        currentStepStr = "1kHz";
                        break;
                    case 1:
                        currentStepStr = "100Hz";
                        break;
                    case 2:
                        currentStepStr = "10Hz";
                        break;
                }

            } else {  // AM/LW/MW

                uint8_t index = (currentBandType == MW_BAND_TYPE or currentBandType == LW_BAND_TYPE) ? configRef.data.ssIdxMW : configRef.data.ssIdxAM;
                currentStepStr = getStepSizeLabelByIndex(Band::stepSizeAM, index);
            }
        }

        return currentStepStr;
    }

    // Current band utils metódusok
    inline const char *getCurrentBandName() { return (const char *)pgm_read_ptr(&getCurrentBand().pConstData->bandName); }

    inline uint8_t getCurrentBandType() { return getCurrentBand().pConstData->bandType; }

    inline uint16_t getCurrentBandMinimumFreq() { return getCurrentBand().pConstData->minimumFreq; }

    inline uint16_t getCurrentBandMaximumFreq() { return getCurrentBand().pConstData->maximumFreq; }

    inline uint16_t getCurrentBandDefaultFreq() { return getCurrentBand().pConstData->defFreq; }

    inline uint8_t getCurrentBandDefaultStep() { return getCurrentBand().pConstData->defStep; }

    inline bool getCurrentBandIsHam() { return getCurrentBand().pConstData->isHam; }

    /**
     * Band nevek lekérdezése
     */
    const char **getBandNames(uint8_t &count, bool isHamFilter);

    void tuneMemoryStation(uint16_t frequency, int16_t bfoOffset, uint8_t bandIndex, uint8_t demodModIndex, uint8_t bandwidthIndex);
};

#endif  // __BAND_H