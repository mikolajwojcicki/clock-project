# Ściągawka: pierwsza konsultacja (Teams)

Zmiana OpenSpec `prepare-first-consultation`. Wszystko do pokazania jest w jednym pliku:
[`pakiet.pdf`](pakiet.pdf) (20 stron, wektorowo, można powiększać). Po zmianie diagramów albo
wykresów: `nix-shell -p poppler-utils img2pdf --run notes/consultation/first-consultation/build-pakiet.sh`.

W mailu obiecałeś cztery rzeczy. Każda ma swoje miejsce:

| Obiecane w mailu | Gdzie |
| --- | --- |
| Działający prototyp: alarm z przeglądarki przez BLE, alarm, wyłączenie ruchem | demo na żywo (kamera) |
| Schemat blokowy sprzętu | `pakiet.pdf` s. 1–2 |
| Diagramy oprogramowania: przegląd, moduły, szczegóły, BLE | `pakiet.pdf` s. 3–16 (rezerwa s. 19–20) |
| Pomiary prądu w uśpieniu i przy odświeżaniu | `pakiet.pdf` s. 17–18 |

## A. Co przeczytać przed konsultacją (ok. 1 h)

Czytaj w tej kolejności. Cel: umieć powiedzieć każdą liczbę i każdy brak bez zaglądania.

1. **Mail do promotora** (`tmp.md`), 2 min. Pamiętaj, co obiecałeś.
2. **Notatka przygotowawcza** [`notes/log/2026-11-02_first-consultation.md`](../../log/2026-11-02_first-consultation.md), 15 min.
   Trzy tabele: wynik próby demo, 19 rzeczy niezrobionych lub częściowych, płytka na biurku vs schemat blokowy.
3. **Pomiar** [`notes/log/2026-11-02_refresh-on-demand.md`](../../log/2026-11-02_refresh-on-demand.md), 10 min.
   Liczby są niżej w części C, krok 5.
4. **Cały `pakiet.pdf` od s. 1 do 20**, 20 min. Dla diagramów oprogramowania przeczytaj też tabelę
   „File / Level / Shows” w [`firmware/docs/diagrams/README.md`](../../../firmware/docs/diagrams/README.md)
   (co pokazuje każda część A–F).
5. **Perfboard** [`notes/log/2026-11-03_perfboard-bringup.md`](../../log/2026-11-03_perfboard-bringup.md)
   i [`hardware/docs/perfboard-shield.md`](../../../hardware/docs/perfboard-shield.md), 5 min. To jest plan „co dalej”.
6. **Pytania** [`notes/promotor-questions.md`](../../promotor-questions.md) i [`notes/deadlines.md`](../../deadlines.md), 5 min.
   Odpowiedzi tam zapisane nie pochodzą od promotora; trzeba je u niego potwierdzić (część C, krok 8).

## B. Dzień przed i godzinę przed

Dzień przed:

1. Sprawdź, czy diagramy dalej pasują do kodu. Pusty wynik = pasują:
   `git diff --stat 8c415fe HEAD -- firmware/apps/clock firmware/src firmware/boards web`.
   Jeśli nie jest pusty: popraw w zmianie `add-firmware-architecture-diagrams`, przerenderuj, przebuduj pakiet.
2. Wgraj aplikację `firmware/apps/clock/` (`make flash`) i przejdź całe demo raz (część C, krok 2).

Godzinę przed:

1. DK na USB J-Link, konsola UART 115200 Bd otwarta, strona `web/` na `localhost:8000` w Chromium.
2. **Kamera na płytkę** (telefon jako druga kamera albo kamera laptopa). Przez Teams promotor nie zobaczy
   ekranu e-paper ani potrząsania bez kamery. Sprawdź, czy słychać buzzer w mikrofonie.
3. Otwarte okna: `pakiet.pdf`, konsola UART, Chromium, ta ściągawka (nie udostępniana).
4. Udostępniaj okno, nie cały ekran.

## C. Kolejność pokazywania (ok. 30 min)

### 1. Wstęp (1 min)

„Prototyp działa na nRF52 DK, oprogramowanie bare-metal na nRF5 SDK. Pokażę najpierw demo,
potem schemat blokowy, diagramy oprogramowania, pomiary i to, czego jeszcze nie ma.”

### 2. Demo na żywo (5–7 min)

Kolejność jak w próbie z 2026-11-02 (wszystko poza DCF przeszło):

1. Reset: odświeżenie ekranu przy starcie (`display ok` w konsoli, ok. 3,7 s).
2. Ustawienie czasu z konsoli. Ekran nie odświeża się, dopóki okno DCF jest otwarte; to celowe.
3. `ble adv`, połączenie ze strony, zapis alarmu na najbliższą minutę (`ble: write ok`).
4. Alarm dzwoni (`ALARM ...`), potrząsanie płytką, `ALARM stop (motion)` po kilku sekundach.
5. DCF: **powiedz wprost**, że w pomieszczeniu odbiornik DCF-1060N nie zdekodował ramki
   (9 odrzuconych impulsów, potem timeout). Pokaż log albo, jeśli podłączony, generator na Nucleo.

Jeśli coś nie zadziała: powiedz to i pokaż wynik próby z notatki. Nie udawaj i nie puszczaj nagrania.

### 3. Sprzęt: `pakiet.pdf` s. 1–2 (3 min)

- **s. 1 schemat blokowy**: to docelowe urządzenie (moduł MDBT42Q, MCP73831, AP2112K, Li-Ion, USB-C).
- **s. 2 schemat płytki stykowej** (wersja uproszczona dla promotora, `hardware/kicad/breadboard-prototype-for-promotor.kicad_sch`): to, co jest na biurku. Pięć bloków: DK, e-paper, LIS3DH, DCF-77, buzzer.
- Różnice do powiedzenia: na biurku nie ma całego toru zasilania (USB-C → MCP73831 → Li-Ion → AP2112K),
  zasilanie z USB DK; zamiast modułu jest DK z tym samym układem nRF52832; przyciski i SWD z DK.
  E-paper, LIS3DH, DCF-1060N i buzzer zgodne ze schematem.

### 4. Oprogramowanie: `pakiet.pdf` s. 3–16 (10 min)

| Strona | Diagram | Co powiedzieć |
| --- | --- | --- |
| 3 | L1 przegląd | stany procesora (start, praca, sen WFE, odświeżanie e-paper), tryby DCF / alarm / BLE, źródła wybudzeń |
| 4–8 | L2 moduły a–e | przerwania tylko ustawiają flagi; pętla główna wywołuje kroki `app_poll`; a: przerwania, pętla i sen, b: alarm i ruch, c: zegar i DCF, d: BLE, e: rysowanie i e-paper. Przejdź szybko |
| 9–14 | L3 szczegóły A–F | A start i sen, B UART/zegar/BLE, C alarm z wyłączaniem ruchem, D okno DCF, E dekoder DCF-77, F odświeżanie e-paper |
| 15–16 | BLE przegląd a–b | strona WWW → przeglądarka → SoftDevice → `ble_sched.c` → zapis do flash; a: połączenie i odczyt, b: zapis i rozłączenie |

Jeśli brakuje czasu: s. 3, potem s. 11 (alarm), s. 12–13 (DCF), s. 15. Resztę pomiń.

### 5. Pomiary: `pakiet.pdf` s. 17–18 (5 min)

Pomiar PPK2 2026-11-02, zasilanie 3,0 V z PPK2, panel i generator DCF podłączone, USB i SWD odłączone.
Oś czasu to czas pliku; zasilenie ok. 164 s, więc odświeżanie „436 s” z notatki jest na wykresie ok. 600 s.

- **s. 17 cały przebieg** (skala log.): spoczynek między naciśnięciami, jedno odświeżenie na naciśnięcie, nic pomiędzy.
- **s. 18 jedno pełne odświeżenie** (skala log.): ok. 3,73 s.

Liczby do powiedzenia:

| Wielkość | Wartość |
| --- | --- |
| Prąd spoczynkowy | 74,8 µA |
| Pełne odświeżenie | 3,73 s, ok. 4,5 mA, 16,6 mC (netto) |
| Dobowo przy 20 odświeżeniach (założenie, nie pomiar) | 1,89 mAh |
| Wcześniej, odświeżanie co minutę | 8,5 mAh na dobę, czyli 4,5 raza więcej |

Wniosek: spoczynek to 95 % budżetu. Teraz celem są peryferia na szynie (ok. 72 µA), nie wyświetlacz.

Zastrzeżenia, gdy zapyta:
- Okna DCF i buzzer nie są w tym bilansie.
- Liczba naciśnięć na dobę jest założeniem.
- Część odświeżeń w tym przebiegu była zaburzona przez sondę na `EPD_BUSY`; liczone są tylko 4 pełne.
- Krótkich szpilek do ok. 250 mA na początku odświeżania nie analizowałem; sprawdzę.

### 6. Czego jeszcze nie ma (3 min)

Powiedz sam, zanim zapyta. Pełna lista (19 pozycji) w notatce przygotowawczej. Najważniejsze:

1. Pomiar napięcia baterii (SAADC nieużywany), więc brak wskaźnika słabej baterii na ekranie.
2. Status alarmu nie jest rysowany na ekranie (jest dzień tygodnia, HH:MM, sekundy).
3. Brak trybu System OFF; najgłębszy stan to sen System ON.
4. LIS3DH i e-paper bez odłączania zasilania (tylko uśpienie rejestrem / komendą); odłączany jest tylko DCF.
5. Synchronizacja DCF nieudana w pomieszczeniu.
6. Tor zasilania z baterią nie jest jeszcze na płytce.

### 7. Co dalej (2 min)

- Prototyp na lutowanej płytce uniwersalnej (perfboard) jako nakładka na DK. Rozmieszczenie w VeroRoute w toku;
  wyświetlacz HAT wpinany w złącze 2×20, LIS3DH w dwa rzędy gniazd. Przymiarka złącz zrobiona 2026-11-03.
- Potem tor baterii i pomiar z baterii.
- Pisanie rozdziałów w kolejności 3 → 4 → 5 → 2 → 1 → 6.

### 8. Pytania do promotora (3 min)

1. Czy lutowana płytka uniwersalna (bez własnego PCB) wystarczy jako prototyp do pracy?
2. Czy potrzebne są testy z udziałem innych osób? (Jeśli tak: wniosek do komisji etyki.)
3. Terminy: czy obowiązują daty jak w zeszłym roku (całość do 03.12, ostateczna wersja do 07.12, antyplagiat do 10.12)?
4. Które poziomy diagramów (L1 / L2 / L3) mają wejść do pracy, a które do załącznika?
5. Jak często konsultacje i w jakiej formie przesyłać fragmenty pracy?

Zapisuj odpowiedzi od razu, słowami promotora.

### Rezerwa (tylko na pytanie)

- `pakiet.pdf` s. 19–20: sekwencje BLE (numerowane komunikaty).
- Tabela 19 rozbieżności spec/kod w notatce przygotowawczej.
- Plik pomiaru `notes/measurements/ppk-20261102T174048.ppkz`.

## D. Po konsultacji (tego samego dnia)

1. Uzupełnij sekcje „Consultation” i „Changed requirements” w
   [`notes/log/2026-11-02_first-consultation.md`](../../log/2026-11-02_first-consultation.md)
   (data, co pokazane, uwagi po polsku dosłownie, decyzje, zadania). Jeśli spotkanie było innego dnia, zmień datę w nazwie (`git mv`).
2. Zmienione wymagania przepisz do `notes/deadlines.md`, `notes/promotor-questions.md` albo `openspec/roadmap.md`.
3. Commit. Potem zadania 3.1–3.3 w `openspec/changes/prepare-first-consultation/tasks.md`.
