# Podpisy rysunków (szkic agenta)

**SZKIC AGENTA** (Claude Code, 2026-11-02). Autor przepisuje każdy podpis
własnymi słowami, zanim trafi do pracy, i odnotowuje to w `notes/ai-use.md`.
Forma zgodna z `clock-project-latex/thesis-writing-rules.yaml`: etykieta „Rys.”,
podpis pod rysunkiem, pod nim wiersz źródła. Rysunek trzeba przywołać w tekście
przed jego wstawieniem (np. „na rys. 3.2”). Numer rysunku nadaje LaTeX.

| Plik | Podpis (szkic agenta) | Źródło |
|---|---|---|
| `l1-overview` | Stany pracy mikrokontrolera i tryby zasilania układów zegara wraz z przerwaniami budzącymi procesor | Źródło: opracowanie własne |
| `l2-modules-a` | Struktura oprogramowania zegara: źródła przerwań i kroki pętli głównej wraz z wywoływanymi modułami (część A) | Źródło: opracowanie własne |
| `l2-modules-b` | Struktura oprogramowania zegara: zależności między modułami oraz sterowane przez nie układy i SoftDevice (część B) | Źródło: opracowanie własne |
| `l3-detail-a` | Uruchomienie, pętla główna, ustawienie terminu RTC1 i wejście w stan uśpienia (część A) | Źródło: opracowanie własne |
| `l3-detail-b` | Obsługa konsoli UART, upływu czasu i okna Bluetooth w pętli głównej (część B) | Źródło: opracowanie własne |
| `l3-detail-c` | Obsługa alarmu: faza brzęczyka, zliczanie ruchu z akcelerometru LIS3DH i wyłączenie alarmu (część C) | Źródło: opracowanie własne |
| `l3-detail-d` | Planowanie okna synchronizacji DCF-77 i obsługa jego wyniku (część D) | Źródło: opracowanie własne |
| `l3-detail-e` | Automat stanów dekodera ramek DCF-77 i obsługa przerwania od zboczy sygnału odbiornika (część E) | Źródło: opracowanie własne |
| `l3-detail-f` | Odświeżanie wyświetlacza e-papierowego z uśpieniem procesora w czasie oczekiwania na sygnał BUSY (część F) | Źródło: opracowanie własne |
| `ble-overview-a` | Podział zadań między stroną WWW, przeglądarką, stosem SoftDevice i oprogramowaniem zegara: rozgłaszanie, połączenie i odczyt harmonogramu (część A) | Źródło: opracowanie własne |
| `ble-overview-b` | Podział zadań między stroną WWW, przeglądarką, stosem SoftDevice i oprogramowaniem zegara: zapis harmonogramu i rozłączenie (część B) | Źródło: opracowanie własne |
| `ble-sequence-a` | Sekwencja komunikatów Bluetooth Low Energy przy nawiązaniu połączenia i odczycie harmonogramu alarmów (część A) | Źródło: opracowanie własne |
| `ble-sequence-b` | Sekwencja komunikatów Bluetooth Low Energy przy zapisie harmonogramu alarmów i rozłączeniu (część B) | Źródło: opracowanie własne |
