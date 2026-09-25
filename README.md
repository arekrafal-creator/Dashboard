# Dashboard Magazynowy (C++ / Qt)

Aplikacja desktopowa (Windows/Linux/macOS) w C++ i Qt, która:
- łączy się bezpośrednio z Twoją bazą danych SQL,
- wykonuje dowolną liczbę kwerend SQL, które **wklejasz bezpośrednio w programie**,
- automatycznie odświeża wyniki co 5 minut (konfigurowalne),
- wyświetla je jako czytelne tabele w ciemnym motywie, dobrym na duży ekran/TV w hali.

**Cała konfiguracja odbywa się w oknach programu** — nie musisz edytować żadnych
plików ręcznie. Dane logowania do bazy i lista kwerend są zapisywane automatycznie
w tle (w folderze `config/`), ale to tylko wewnętrzny magazyn programu.

## 1. Czego potrzebujesz do zbudowania programu

- **Qt 6** (moduły: Widgets, Sql, Core, Gui) — https://www.qt.io/download-open-source
- **CMake** ≥ 3.16
- Kompilator C++ (Visual Studio na Windows, gcc/clang na Linux/macOS)
- **Sterownik do Twojej bazy danych**:
  - SQL Server → sterownik ODBC ("ODBC Driver 17/18 for SQL Server") + moduł Qt `QODBC`
  - MySQL/MariaDB → wtyczka Qt `QMYSQL` (może wymagać osobnej instalacji `libmysqlclient`)
  - PostgreSQL → wtyczka Qt `QPSQL`

## 2. Budowanie

```bash
cd dashboard
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="ŚCIEŻKA_DO_QT/6.x.x/kompilator"
cmake --build . --config Release
```

Albo skorzystaj z automatycznego budowania w chmurze (patrz punkt 7 niżej) — nie
wymaga niczego instalować lokalnie.

## 3. Pierwsze uruchomienie — konfiguracja w programie

Przy pierwszym starcie (albo gdy program nie może się połączyć z bazą) automatycznie
pojawi się okno **"Ustawienia połączenia z bazą danych"**:

1. Wybierz typ bazy (SQL Server / MySQL / PostgreSQL).
2. Wpisz dane połączenia (connection string dla SQL Server, albo host/port/login/hasło
   dla MySQL/PostgreSQL).
3. Kliknij **"Testuj połączenie"**, żeby od razu sprawdzić, czy dane są poprawne.
4. Kliknij **"Zapisz"**.

To okno możesz otworzyć ponownie w każdej chwili z menu **Połączenie → Ustawienia
połączenia...** — np. gdy zmieni się hasło do bazy.

## 4. Dodawanie i edycja kwerend — też w programie

Menu **Kwerendy → Zarządzaj kwerendami...** otwiera listę wszystkich paneli
(kafelków) na dashboardzie:

- **Dodaj panel...** — otwiera formularz: tytuł panelu + duże pole tekstowe,
  w które **wklejasz swoje zapytanie SQL**. Zatwierdzasz — panel od razu pojawia
  się na dashboardzie.
- **Edytuj...** (albo dwuklik na pozycji) — poprawiasz tytuł/SQL istniejącego panelu.
- **Usuń** — usuwa panel.
- **Przesuń w górę / w dół** — zmienia kolejność. Układ siatki (2 kolumny, tyle
  wierszy ile potrzeba) liczy się automatycznie na podstawie tej kolejności —
  nie musisz podawać żadnych współrzędnych.
- Każdy panel może mieć własny czas odświeżania (checkbox "Własny czas odświeżania"),
  albo korzystać z globalnego (domyślnie 300 s = 5 minut, ustawiasz go w oknie
  Ustawienia połączenia).

Program startuje z pustą listą paneli (albo z 4 przykładowymi kwerendami, jeśli
korzystasz z paczki tak jak ją dostałeś) — dodajesz własne przez ten formularz,
bez dotykania jakiegokolwiek pliku.

Przykładowe zapytania (SQL Server), które możesz wkleić jako punkt wyjścia:

```sql
-- Pick taski wg statusu
SELECT status AS Status, COUNT(*) AS Ilosc
FROM pick_tasks
WHERE status IN ('RELEASED', 'HOLD')
GROUP BY status ORDER BY status

-- Replenishmenty wg typu lokacji
SELECT src.location_type AS Skad, dst.location_type AS Dokad, COUNT(*) AS Ilosc
FROM replenishments r
JOIN locations src ON r.source_location_id = src.location_id
JOIN locations dst ON r.destination_location_id = dst.location_id
GROUP BY src.location_type, dst.location_type
ORDER BY Ilosc DESC

-- Srednia produktywnosc wg godzin (zmien daty na swoj okres)
SELECT DATEPART(HOUR, completed_at) AS Godzina,
       ROUND(AVG(lines_per_hour), 1) AS SredniaProduktywnosc
FROM picker_activity
WHERE completed_at >= '2026-09-01' AND completed_at < '2026-09-26'
GROUP BY DATEPART(HOUR, completed_at) ORDER BY Godzina

-- 3 najlepsi i 3 najslabsi pickerzy
WITH ranked AS (
  SELECT employee_name AS Picker, ROUND(AVG(lines_per_hour), 1) AS Produktywnosc,
         RANK() OVER (ORDER BY AVG(lines_per_hour) DESC) AS rnk_top,
         RANK() OVER (ORDER BY AVG(lines_per_hour) ASC) AS rnk_bottom
  FROM picker_activity
  WHERE completed_at >= '2026-09-01' AND completed_at < '2026-09-26'
  GROUP BY employee_name
)
SELECT Picker, Produktywnosc,
       CASE WHEN rnk_top <= 3 THEN 'TOP 3' ELSE 'DOL 3' END AS Grupa
FROM ranked WHERE rnk_top <= 3 OR rnk_bottom <= 3
ORDER BY Produktywnosc DESC
```

> Powyższe zapytania są w składni SQL Server (`DATEPART`, `RANK() OVER`). Dla MySQL/PostgreSQL
> podmień funkcje daty (`HOUR(x)` w MySQL, `EXTRACT(HOUR FROM x)` w PostgreSQL) i oczywiście
> nazwy Twoich własnych tabel/kolumn.

Kolorowanie wierszy (RELEASED na zielono / HOLD na czerwono, TOP 3 na zielono / DOL 3
na czerwono) działa automatycznie, jeśli panel ma `id` odpowiednio `pick_status` albo
`top_bottom_pickers` — id nadawane jest automatycznie z tytułu panelu, więc jeśli
chcesz zachować to kolorowanie, tytuł panelu powinien zaczynać się podobnie jak
w przykładach powyżej. To jedyne miejsce, gdzie trzeba by zajrzeć do kodu C++
(`QueryPanel.cpp`, funkcja `applyRowColoring()`), gdyby ktoś chciał inne reguły kolorowania.

## 5. Wygląd (kolory, czcionki)

`resources/style.qss` to zwykły plik tekstowy (składnia podobna do CSS) — jedyny
element, który nadal edytuje się w pliku, bo dotyczy całego programu, a nie
pojedynczej kwerendy. Zmień kolory (`#RRGGBB`), zapisz, zrestartuj program.

## 6. Obsługa na dużym ekranie

- Program startuje zmaksymalizowany.
- **F11** — przełącza pełny ekran (bez paska okna) — wygodne na telewizorze w hali.
- **Esc** — wychodzi z pełnego ekranu.
- Menu **Widok → Odśwież wszystko teraz** — ręczne odświeżenie, bez czekania na timer.

## 7. Automatyczne budowanie .exe w chmurze (bez instalowania Qt lokalnie)

W folderze `.github/workflows/build.yml` jest gotowa konfiguracja GitHub Actions,
która sama pobiera Qt, buduje program i przygotowuje gotowy `.exe` do pobrania.

1. Załóż darmowe konto: https://github.com/signup
2. Utwórz nowe repozytorium: https://github.com/new (Public, żeby budowanie było bez limitów)
3. Na stronie repozytorium: **"uploading an existing file"** → wrzuć całą zawartość
   folderu `dashboard/` (wszystko naraz) → **Commit changes**.
4. Zakładka **Actions** — workflow uruchamia się sam.
5. Po zakończeniu (zielony ✔) → wejdź w przebieg → sekcja **Artifacts** →
   pobierz `DashboardMagazynowy-Windows.zip`, rozpakuj, uruchom `WarehouseDashboard.exe`.
6. Skoro dane logowania wpisujesz teraz **w programie** (a nie w pliku wrzucanym
   na GitHub), możesz spokojnie trzymać to repozytorium jako publiczne — hasło do
   bazy nigdy nie trafia do kodu ani do repozytorium, wpisujesz je dopiero
   przy pierwszym uruchomieniu gotowego `.exe` na swoim komputerze.

**Zmiana czegoś w kodzie C++ później:** edytujesz plik `.cpp`/`.h` w tym samym
repozytorium na GitHub (przycisk ✏️ przy pliku), **Commit changes** — Actions
automatycznie zbuduje nową wersję `.exe`. To wciąż ten sam projekt/repozytorium,
nigdy nie zakładasz nowego.

## 8. Struktura kodu

```
dashboard/
├── config/
│   └── queries.json          <- wewnetrzny zapis (tworzony/aktualizowany automatycznie)
├── resources/
│   └── style.qss              <- kolory / czcionki (EDYTUJ recznie, opcjonalnie)
├── src/
│   ├── main.cpp                          <- start programu
│   ├── ConfigManager.*                    <- wczytuje/zapisuje pliki .json
│   ├── DatabaseManager.*                  <- laczy sie z baza i wykonuje SQL
│   ├── QueryPanel.*                       <- pojedynczy kafelek z tabela + kolorowanie wierszy
│   ├── ClockWidget.*                      <- zegar w pasku gornym
│   ├── ConnectionSettingsDialog.*         <- okno "Ustawienia polaczenia"
│   ├── PanelEditDialog.*                  <- okno "Dodaj/Edytuj panel" (tu wklejasz SQL)
│   ├── PanelManagerDialog.*               <- lista panelow (dodaj/usun/kolejnosc)
│   └── MainWindow.*                       <- skleja wszystko w jedno okno, menu, siatke
└── CMakeLists.txt
```

## 9. Częste problemy

- **"Driver not loaded" / brak sterownika QMYSQL lub QPSQL** — te wtyczki trzeba czasem doinstalować
  osobno (Qt Maintenance Tool → Additional Libraries) albo skompilować samemu z odpowiednią
  biblioteką klienta (libmysqlclient / libpq) w PATH.
- **Program się łączy, ale panel pokazuje błąd SQL** — sprawdź nazwy tabel/kolumn w swoim
  zapytaniu; przetestuj je najpierw wprost w swoim kliencie SQL (SSMS / DBeaver itp.).
- **Za bardzo obciąża bazę** — zwiększ czas odświeżania w oknie "Ustawienia połączenia"
  (albo dla pojedynczego panelu w oknie edycji panelu).
- **Zgubiłem hasło do bazy** — po prostu otwórz menu Połączenie → Ustawienia połączenia
  i wpisz je ponownie; program samodzielnie zapisze nową wersję.
