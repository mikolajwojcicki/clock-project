# SoftDevice S112: rola w oprogramowaniu zegara

**SZKIC AGENTA** (Claude Code, 2026-11-02). Uzupełnia rysunki `ble-overview-a`,
`ble-overview-b`, `ble-sequence-a`, `ble-sequence-b` i `l2-modules-b`. Autor
przepisuje tekst własnymi słowami, zanim trafi do pracy (wpis w `notes/ai-use.md`).
Fakty pochodzą z kodu w commicie `8c415fe` i z nagłówków nRF5 SDK 17.1.0.

## Czym jest

SoftDevice S112 w wersji 7.2.0 to gotowy, binarny stos Bluetooth Low Energy firmy
Nordic Semiconductor. Obsługuje tylko rolę urządzenia peryferyjnego (peripheral).
Zajmuje dolną część pamięci flash (poniżej adresu 0x19000) i początek pamięci RAM
(aplikacja zaczyna się od 0x200022C8). Programuje się go raz, osobno od aplikacji
(`make flash_softdevice`); plik `.hex` aplikacji go nie zawiera.

## Za co odpowiada

- radio i zależności czasowe warstwy łącza (rozgłaszanie, zdarzenia połączenia),
- protokoły GAP, GATT i ATT, w tym samodzielne odpowiedzi na odkrywanie usług
  i charakterystyk oraz wymianę MTU,
- bloki POWER i CLOCK: uruchamia zegar LFCLK z kwarcu 32,768 kHz (dokładność
  20 ppm) i przetwornicę DC/DC na żądanie aplikacji,
- kolejkowanie kasowania i zapisu pamięci flash, tak aby nie kolidowały z radiem,
- zarezerwowane zasoby, m.in. RADIO, RTC0 i TIMER0 (pełna lista: specyfikacja S112).
  Dlatego zegar aplikacji korzysta z RTC1.

## Jak aplikacja z nim rozmawia

- **Aplikacja → SoftDevice:** funkcje `sd_*` wywoływane instrukcją SVC, np.
  `sd_ble_gap_adv_set_configure`, `sd_ble_gap_adv_start`, `sd_ble_gap_adv_stop`,
  `sd_ble_gatts_rw_authorize_reply`, `sd_ble_gap_disconnect`, `sd_power_dcdc_mode_set`.
  Zapis flash idzie przez `nrf_fstorage_sd`.
- **SoftDevice → aplikacja:** zdarzenia przez przerwanie programowe SWI2
  (`SD_EVT_IRQn`, tryb przerwaniowy `NRF_SDH_DISPATCH_MODEL 0`). Moduł SDK `nrf_sdh`
  rozsyła je do obserwatorów: `nrf_ble_gatt` (priorytet 1), `nrf_ble_qwr` (2)
  i `on_ble_evt` w `ble_sched.c` (3). Zdarzenia SoC (koniec operacji flash) trafiają
  do `nrf_fstorage_sd`.

## Zdarzenia obsługiwane przez zegar

| Zdarzenie | Reakcja (`ble_sched.c`) |
|---|---|
| `BLE_GAP_EVT_CONNECTED` | zapamiętanie uchwytu łącza, flaga dla pętli głównej |
| `BLE_GAP_EVT_DISCONNECTED` | zwolnienie uchwytu, flaga dla pętli głównej |
| `BLE_GAP_EVT_ADV_SET_TERMINATED` | koniec 60-sekundowego okna rozgłaszania |
| `BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST` | odczyt: zakodowanie bieżącej tabeli alarmów; zapis: walidacja (`sched_decode`) i odpowiedź ze statusem |
| `BLE_GAP_EVT_PHY_UPDATE_REQUEST` | zgoda na automatyczny wybór PHY |
| `BLE_GAP_EVT_SEC_PARAMS_REQUEST` | odmowa parowania (łącze otwarte) |
| `BLE_GATTS_EVT_SYS_ATTR_MISSING` | ustawienie pustych atrybutów systemowych |

Funkcje obsługi zdarzeń działają w przerwaniu i tylko ustawiają flagi. Zapis
tabeli i jej utrwalenie we flash wykonuje pętla główna (`ble_sched_poll`).

## Gdzie w kodzie

| Temat | Plik |
|---|---|
| włączenie stosu, LFCLK, DC/DC | `firmware/src/board.c:clocks_start` |
| usługa GATT, rozgłaszanie, zdarzenia | `firmware/src/ble_sched.c` |
| zapis flash przez SoftDevice | `firmware/src/sched_store.c` |
| konfiguracja (`NRF_SDH_*`) | `firmware/apps/clock/config/sdk_config.h` |
| mapa pamięci | `firmware/apps/clock/armgcc/clock_gcc_nrf52.ld` |
