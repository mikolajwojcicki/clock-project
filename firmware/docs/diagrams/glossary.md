# Słownik pojęć z rysunków

**SZKIC AGENTA** (Claude Code, 2026-11-02). Objaśnia skróty i nazwy użyte
w rysunkach z tego katalogu. Może posłużyć jako podstawa wykazu skrótów w pracy;
autor przepisuje go własnymi słowami (wpis w `notes/ai-use.md`). Nazwy funkcji
i plików źródłowych opisuje `README.md` (tabela faktów).

| Pojęcie | Znaczenie |
|---|---|
| ADV_IND, SCAN_RSP | Pakiet rozgłoszeniowy BLE i odpowiedź na zapytanie skanera. Zegar wysyła nazwę „ShakeClock” w pierwszym, a UUID usługi w drugim. |
| ATT | Attribute Protocol: protokół odczytu i zapisu atrybutów (wartości) urządzenia BLE. |
| ATT MTU | Największy rozmiar pakietu ATT uzgodniony po połączeniu; zegar dopuszcza do 247 B. |
| autoryzacja (RW_AUTHORIZE_REQUEST) | Mechanizm SoftDevice: przed odczytem lub zapisem charakterystyki pyta aplikację o zgodę i wartość. Odmiany zapisu: WRITE_REQ (jeden pakiet), PREP_WRITE i EXEC_WRITE (zapis długi). Zegar tak podaje bieżącą tabelę i sprawdza nowy harmonogram. |
| BCD | Binary-Coded Decimal: zapis cyfr dziesiętnych po 4 bity; tak są kodowane pola czasu w ramce DCF-77. |
| BLE | Bluetooth Low Energy: energooszczędna odmiana Bluetooth, używana tylko do konfiguracji harmonogramu. |
| BUSY | Wyjście wyświetlacza e-Paper: stan wysoki, gdy sterownik pracuje. |
| BUTTON, TIMEOUT, MOTION | Przyczyny zakończenia alarmu: przycisk, upływ 60 s, wykryty ruch. |
| CC0, CC1, COMPARE0, COMPARE1 | Rejestry porównania licznika RTC1 i ich zdarzenia; zrównanie z licznikiem budzi procesor. CC0: termin aplikacji, CC1: oczekiwanie w sterownikach. |
| charakterystyka GATT | Pojedyncza wartość usługi GATT; zegar ma jedną, 113-bajtową, z tabelą alarmów. |
| CONNECT_IND | Pakiet, którym przeglądarka (urządzenie centralne) otwiera połączenie BLE. |
| CPU | Procesor (rdzeń Cortex-M4F układu nRF52832). |
| CR, LF | Znaki końca linii w konsoli UART. |
| CRC16 | Suma kontrolna rekordu harmonogramu w pamięci flash. |
| DC/DC | Przetwornica impulsowa zasilania rdzenia nRF52; sprawniejsza niż stabilizator liniowy. |
| DCF-77 | Długofalowy (77,5 kHz) nadajnik sygnału czasu z Mainflingen; jedna ramka na minutę, jeden bit na sekundę. |
| DCF_OUT, PON (DCF_PON) | Wyjście sygnału odbiornika DCF-77 i jego wejście wyłączenia zasilania (aktywne niskim stanem). |
| e-Paper | Wyświetlacz elektroforetyczny (moduł Waveshare 2,13″, sterownik SSD1680); obraz zostaje bez zasilania. |
| FIFO | Bufor kolejkowy; konsola UART zbiera w nim odebrane bajty. |
| flash | Nieulotna pamięć mikrokontrolera; strona 0x7F000 przechowuje harmonogram alarmów. |
| FPU | Koprocesor zmiennoprzecinkowy; jego flagi wyjątków są czyszczone przed snem (błąd układu nRF52 nr 87). |
| GAP | Generic Access Profile: część BLE odpowiedzialna za rozgłaszanie i nawiązywanie połączeń. |
| GAP_EVT_CONNECTED, GAP_EVT_DISCONNECTED, ADV_SET_TERMINATED | Zdarzenia SoftDevice: połączenie, rozłączenie, koniec okna rozgłaszania (pełne nazwy z przedrostkiem `BLE_`; opis w `softdevice.md`). |
| GATT | Generic Attribute Profile: organizacja danych urządzenia BLE w usługi i charakterystyki. |
| GG:MM, HH:MM, RRRR-MM-DD | Formaty godziny i daty (godzina, minuta; rok, miesiąc, dzień) w poleceniach konsoli. |
| GPIOTE | Blok nRF52 zamieniający zmiany stanu pinów na zdarzenia i przerwania. Kanał IN: dokładne wykrywanie zbocza, większy prąd; zdarzenie PORT (SENSE): wspólne dla wielu pinów, prawie bez prądu. |
| HP (filtr) | Filtr górnoprzepustowy akcelerometru; usuwa stałe przyspieszenie ziemskie, zostaje sam ruch. |
| INT1, INT1_SRC | Wyjście przerwania akcelerometru LIS3DH (ruch powyżej progu) i rejestr, którego odczyt zwalnia zatrzaśnięte przerwanie. |
| ISR | Interrupt Service Routine: funkcja obsługi przerwania. W zegarze tylko ustawia flagi, resztę robi pętla główna. |
| kandydat (ramka) | Pierwsza poprawna ramka DCF-77; czas jest przyjmowany dopiero, gdy następna ramka jest dokładnie minutę późniejsza. |
| LFCLK, LFXO | Zegar niskiej częstotliwości 32,768 kHz i jego źródło kwarcowe; taktuje RTC1 także w czasie snu. |
| LIS3DH | Trzyosiowy akcelerometr ST; wykrywa potrząsanie wyłączające alarm. |
| LL_TERMINATE_IND | Komunikat warstwy łącza BLE kończący połączenie. |
| nazwy pinów (P0.nn) | Piny portu P0 układu nRF52832, np. P0.25. W rysunkach także nazwy z `dk_breadboard_pins.h`: EPD_CS, EPD_DC, EPD_RST, EPD_BUSY (wyświetlacz e-Paper), SENSOR_CS (LIS3DH), BUZZER_EN (brzęczyk), CLOCK_SEC_MARK (znacznik sekundy do pomiarów). |
| nrf_ble_gatt, nrf_ble_qwr, nrf_fstorage | Moduły nRF5 SDK: wymiana MTU, buforowanie zapisu długiego, zapis pamięci flash przez SoftDevice. |
| nrfx | Biblioteka sterowników peryferiów nRF z nRF5 SDK, używana zamiast bezpośredniego dostępu do rejestrów. |
| OFF, WAIT_MARKER, RECEIVE, WAIT_START | Stany dekodera DCF-77: okno zamknięte, oczekiwanie na początek minuty, zbieranie 59 bitów, oczekiwanie na początek minuty po dwóch zgodnych ramkach. |
| okno DCF | Przedział do 5 min, w którym odbiornik DCF-77 jest zasilany i dekodowane są ramki. |
| okno rozgłaszania | 60 s po przytrzymaniu przycisku, w których zegar jest widoczny dla przeglądarki. |
| OS | System operacyjny komputera lub telefonu z przeglądarką; jego stos BLE obsługuje radio po stronie użytkownika. |
| OVERFLOW (RTC1) | Przepełnienie 24-bitowego licznika RTC1 (co 512 s); liczone, aby czas biegł dalej. |
| overrun | Utrata impulsu DCF-77: nowy impuls nadszedł, zanim pętla główna odebrała poprzedni. |
| pętla główna | Funkcja `main()`: wywołuje `app_poll()` (kroki 1–6), potem odświeża ekran albo usypia procesor. |
| PHY | Warstwa fizyczna radia BLE (szybkość transmisji, np. 1 Mb/s). |
| przerwa minutowa | Brak impulsu w 59. sekundzie ramki DCF-77; wyznacza początek minuty. |
| PWM, PWM0 | Modulacja szerokości impulsów; blok PWM0 generuje ton 2,7 kHz brzęczyka. |
| RAM | Pamięć operacyjna; tu trzymana jest bieżąca tabela alarmów. |
| REJECTED (pulse, timeout, mismatch …) | Odrzucenie ramki DCF-77 z podaną przyczyną; czas zegara pozostaje bez zmian. |
| RST | Wejście resetu (sterownika wyświetlacza e-Paper). |
| RTC1 | Licznik czasu rzeczywistego nRF52 taktowany z LFCLK; podstawa czasu zegara i źródło wybudzeń. |
| SCK, MOSI, MISO | Linie magistrali SPI: zegar, dane do układu, dane z układu. |
| SoftDevice (S112) | Binarny stos BLE firmy Nordic; opis w `softdevice.md`. |
| SPI, SPIM0 | Szeregowy interfejs do wyświetlacza e-Paper i akcelerometru; SPIM0 to jego blok w nRF52, włączany tylko na czas transmisji. |
| SSD1680 | Sterownik wyświetlacza e-Paper. |
| SVC | Instrukcja procesora Cortex-M wywołująca funkcję SoftDevice. |
| SWI2 | Przerwanie programowe, którym SoftDevice przekazuje zdarzenia aplikacji. |
| System ON, WFE, SEV | Tryb uśpienia nRF52 z działającym RTC; WFE usypia procesor do zdarzenia, SEV ustawia zdarzenie (para SEV+WFE czyści jego rejestr). |
| UARTE0 | Port szeregowy z DMA; konsola diagnostyczna, tylko w wersji z `SLEEP_UART=1`. |
| UUID | Identyfikator usługi lub charakterystyki BLE; zegar używa `e1430001-…` i `e1430002-…`. |
| Web Bluetooth | Interfejs przeglądarki (Chrome, Edge) pozwalający stronie WWW łączyć się z urządzeniem BLE. |
| Write Request; Prepare + Execute Write | Zapis ATT jednym pakietem albo zapis długi w częściach, zatwierdzany na końcu; wybiera go przeglądarka zależnie od MTU. |
