---
Title: Metody programowania równoległego - MPI
Author: Kacper Kafara
---

<script type="text/javascript" src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML"></script>
<script type="text/x-mathjax-config">
  MathJax.Hub.Config({ tex2jax: {inlineMath: [['$', '$']]}, messageStyle: "none" });
</script>

# Metody Programowania Równoległego - MPI

Autor: Kacper Kafara\
Grupa: wtorek 15:00

# Spis treści

- [Metody Programowania Równoległego - MPI](#metody-programowania-równoległego---mpi)
- [Spis treści](#spis-treści)
- [Zadanie 1](#zadanie-1)
- [Wyniki](#wyniki)
- [Wnioski](#wnioski)
- [Zadanie 2 - sortowanie kubełkowe (wersja 1)](#zadanie-2---sortowanie-kubełkowe-wersja-1)
	- [Implementowany algorytm](#implementowany-algorytm)
		- [Analiza złożoności](#analiza-złożoności)
	- [Środowisko i kompilacja](#środowisko-i-kompilacja)
	- [Szczegóły implementacji](#szczegóły-implementacji)
		- [Założenia](#założenia)
		- [Struktury danych](#struktury-danych)
		- [Pomiar czasu](#pomiar-czasu)
		- [PRNG](#prng)
		- [Zrównoleglanie pętli `for`](#zrównoleglanie-pętli-for)
		- [Unikanie równoczesnych odczytów tych samych rejonów pamięci](#unikanie-równoczesnych-odczytów-tych-samych-rejonów-pamięci)
		- [Uniknięcie potrzeby synchronizacji](#uniknięcie-potrzeby-synchronizacji)
		- [Usprawnienie fazy `gather`](#usprawnienie-fazy-gather)
	- [Wyniki eksperymentów](#wyniki-eksperymentów)
		- [Czas wykonania algorytmu sekwencyjnego a oczekiwany rozmiar kubełka](#czas-wykonania-algorytmu-sekwencyjnego-a-oczekiwany-rozmiar-kubełka)
		- [Czas wykonania algorytmu równoległego a liczba wątków](#czas-wykonania-algorytmu-równoległego-a-liczba-wątków)
		- [Porównanie z algorytmem w wersji 2](#porównanie-z-algorytmem-w-wersji-2)
- [Kod źródłowy](#kod-źródłowy)

# Zadanie 1

Celem zadania był pomiar czasu i przyśpieszenia wykonania programu wypełniającego
strukturę danych liczbami losowymi w zależności od liczby liczb oraz konfiguracji OpenMP (klauzuli `schedule`).

Zdecydowałem się na skalowanie silne.

# Wyniki

Poniżej zamieszczam serię wykresów prezentujacych pozyskane wyniki.

| ![draw-comb-1mb](src/plots/draw/combined-131072.png) |
|:--:|
| *Wykres 1.1 Zestawienie wyników dla różnych konfiguracji przy rozmiarze tablicy 1,31+e5* |


| ![draw-comb-4mb](src/plots/draw/combined-524288.png) |
|:--:|
| *Wykres 1.2 Zestawienie wyników dla różnych konfiguracji przy rozmiarze tablicy 5,24+e5* |


| ![draw-comb-16mb](src/plots/draw/combined-2097152.png) |
|:--:|
| *Wykres 1.3 Zestawienie wyników dla różnych konfiguracji przy rozmiarze tablicy 2,1+e6* |


| ![draw-comb-64mb](src/plots/draw/combined-8388608.png) |
|:--:|
| *Wykres 1.4 Zestawienie wyników dla różnych konfiguracji przy rozmiarze tablicy 8,4+e6* |


| ![draw-comb-256mb](src/plots/draw/combined-33554432.png) |
|:--:|
| *Wykres 1.5 Zestawienie wyników dla różnych konfiguracji przy rozmiarze tablicy 33,6+e6* |

# Wnioski

Dla tego typu zadania klauzula `schedule(dynamic)` sprawdza się zdecydowanie najgorzej. Przydziela ona wątkom zadania rozmiaru `chunk_size` (domyślnie $1$) "na żądanie".
Zdaje się, że przy `chunk_size` 1 bądź 2 (z takimi eksperymentowałem) i bardzo małym rozmiarze zadania (wylosowanie pojedynczej liczby pseudolosowej) narzut organizacyjny zakłóca poprawne działanie.
Przy większym `chunk_size` bądź znacząco większym rozmiarze pojedynczego zadania być może ta klauzula sprawdziła by się lepiej (wykresy 1.{1..5}).


Ustawianie `chunk_size` "na sztywno" daje złę efekty. `chunk_size` powinien pozostawać w zależności do rozmiaru pojedynczego zadania (aby narzut organizacyjny stanowił możliwie małą część czasu wykonania) (wykresy 1.{1..5})


Wartości domyślne `chunk_size` sprawdzają się relatywnie dobrze (wykres 1.{4,5})


`guided` i `static` na `auto` osiągają podobne wyniki (najlepsze) (wykresy 1.{4,5}).

# Zadanie 2 - sortowanie kubełkowe (wersja 1)

Cele zadania:

1. Implemenetacja sekwencyjnego algorytmu sortowania kubełkowego
2. Przeprowadznienie pomiarów i znalezienie optymalnego rozmiaru kubełka (dającego najmniejszy czas wykonania, przy stałym rozmiarze zadania)
3. Implementacja algorytmu równoległego sortowania kubełkowego (wersja 1)
4. Przeprowadzanie pomiarów czasu i przyśpieszenia w zależności od liczby wątków (rozmiar tablicy i kubełka stały)
5. Porównanie swojej wersji algorytmu z inną, zaimplementowaną przez członka zespołu

## Implementowany algorytm

Implementowany był wariant 1:

* Wątki dzielą się wypełnianiem tablicy liczbami (każdy wypełnia swój fragment) (faza `draw`)
* Każdy z wątków czyta całą tablicę w celu przydzielenia liczb do odopowiednich kubełków (faza `scatter`)
* Każdy z wątków przydziela tylko te liczby, które powinny znaleźć się w przypisanym mu kubełku (faza `scatter`)
* Wątki dzielą się kubełkami do sortowania (każdy sortuje swoje) (faza `sort`)
* Wątki dzielą się kubełkami do przepisania do tablicy wyjściowej (każdy przepisuje swoje) (faza `gather`)

### Analiza złożoności

Przyjmijmy onaczenia: $n$ - liczba elementów do posortowania, $p$ - liczba wątków, $b$ - liczba kubełków.

Przeanalizujmy najpierw złożoność algorytmu sekwencyjnego w modelu RAM.

## Środowisko i kompilacja

* Wszystkie pomiary / eksperymenty przeprowadzone zostały na komputerze "Ares" (pojedynczy węzeł: `	48 cores, Intel(R) Xeon(R) Platinum 8268 CPU @ 2.90GHz`, `3,85 GB RAM`).
Ze względu na [szczegóły implementacyjne](#uniknięcie-potrzeby-synchronizacji) kod programu sortującego
(napisanego w `C++`) kompilowany był z użyciem `gcc 10.3.0` z flagami `-std=c++11 -fopenmp -O2`.

Wersja `gcc` była podyktowana faktem, że w kodzie korzystam z szczegółu implementacyjnego `OpenMP` i lokalnie,
gdzie program był testowany przed przeprowadzaniem właściwych eksperymentów, korzystam właśnie z `gcc 10.3.0`.

* Wszystkie wynikowe punkty pomiarowe są średnią z 10 pomiarów.

## Szczegóły implementacji

### Założenia

Zaimplementowany algorytm zakłada:

1. Liczba kubełków jest większa bądź równa liczbie wątków.
2. Liczba elementów w sortowanej tablicy jest więszka bądź równa liczbie wątków.

Brak powyższych założeń wprowadzałby niepotrzebne komplikacje do kodu, który musiałby w specjalny
sposób obsługiwać te przypadki.

Powyższe założenia są weryfikowane w czasie działania programu.

### Struktury danych

* Poszczególne elementy są typu `double`.
* Sortowana tablica jest alokowana w ciągłym bloku pamięci na stercie (operator `new`).
* Kubełki reprezentowane są przez wektor wektorów `std::vector<std::vector<double>>` (więc przy implementacji `STL` wykorzystywanej przez `gcc`
	poszczególne kubełki zaalokowane są na stercie).

### Pomiar czasu

Do pomiarów czasu wykorzystana została funkcja `omp_get_wtime()` pochodząca z `OpenMP`.

Do czasu całkowitego został zaliczony cały region `parallel`, a więc wliczone zostały:

1. tworzenie środowiska wykonawczego przez `OpenMP`,
2. inicjalizacja stanu dla generatora liczb losowych,
3. fazy `draw`, `scatter`, `sort`, `gather`

Co istotne, do czasu wykonania nie został wliczony czas alokacji tablicy liczb ani czas alokacji wektora kubełków.
Uznałem, że w przypadku algorytmu równoległego, gdzie w eksperymentach rozmiar tablicy i liczba kubełków są stałe,
jest to część stała, nic nie wnosząca do badanej charakterystyki algorytmu równoległego. Oczywiście jest to istotna część
sekwencyjna algorytmu (alokacja kubełków), mająca duży wpływ na ostateczne wartości przyśpieszenia, jednak nie zalicza się ona do żadnej z badanych faz,
dlatego wykluczyłem ją z analizy.

Również istotny jest fakt, że wynikami są pomiary dla wątku 0, inne dane są odrzucane.

### PRNG

Wykorzystano `erand48`, który jest generatorem kongruencyjnym (nie jest to podejście które daje najlepsze rezultaty) dającym
docelowo rozkład jednostajny. Istotne jest to, że nie korzysta on z wspólnego (globalnego) stanu i dobrze zachowuje się w środowisku wielowątkowym.

Przeprowadziłem prostą weryfikację tego generatora, generując ok. 16 mln. liczb z przedziału $[0, 1)$ i rozmieszczając je do 32. równomiernych kubełków.
Wynik tego eksperymentu przedstawiam na poniższym wykresie.

| ![prng plot](src/plots/sort/prng-distribution-16777216-32.png) |
|:--:|
| *Wykres 2.1 Rozkład generatora `erand48`* |

Widzimy na powyższym wykresie, że przy przy dużej liczbie elementów w tablicy generator faktycznie daje próbki z rozkładu jednostajnego. Charakterystyka ta nie jest zachowana,
jeżeli przeprowadzimy obliczenia dla małej liczby elementów, rzędu 100-1000. Nie stanowi to jednak problemu, gdyż wszystkie eksperymenty były przeprowadzane na znacznie
większej liczbie elementów (ok. 33 mln.).


### Zrównoleglanie pętli `for`

Zgodnie z wynikami Zadania 1, do zrównoleglania pętli `for` wykorzystałem klauzulę `schedule(static)`, która dawała najlepsze rezultaty.

### Unikanie równoczesnych odczytów tych samych rejonów pamięci

Implementacja fazy `scatter` zapewnia, że każdy z wątków zaczyna przeszukiwać tablicę w poszukiwaniu "swoich" liczb od innego indeksu, próbując w ten sposób
minimalizować synchornizację sprzętową w dostępie do tych samych rejonów pamięci.

### Uniknięcie potrzeby synchronizacji

Zauważmy, że jeżeli przypisanie kubełków do wątków w fazie `scatter` nie jest jednakowe z podziałem wproadzanym przez `OpenMP` w fazie `sort`,
to może dojść do sytuacji, gdy do kubełka już sortowanego przez jeden wątek, inny będzie chciał dopisać wartość, bo skoro podziały nie są jednakowe
to zachodzą konflikty.

Jednym z możliwych rozwiązań tego problemu jest wprowadzenie synchronizacji (poprzez barierę) pomiędzy tymi fazami algorytmu.

Natomiast rozwiązaniem wybranym przeze mnie było przydzielenie kubełków do wątków w sposób jednakowy z `OpenMP` (`#pragma omp for schedule(static)`).

Specyfikacja wskzauje, że w przypadku braku jawnego przekazania wartości `chunk` praca dzielona jest spójnymi przedziałami możliwie po równo pomiędzy wątki.
Jednak w przypadku, gdy liczba wątków nie dzieli liczby kubełków dokumentacja nie specyfikuje jak rozdzielone będą kubełki "będące resztą z dzielenia".

Inspekcja implementacji wersji `OpenMP` dostarczanej wraz z `gcc 10.3.0` wykazała, że praca dzielona jest wg rosnących indeksów po równo wątkom w kolejności ich `id` (zdaje się, że to określa jeszcze specyfikacja).
Do tego jeżeli mamy $r$ kubełków będących "resztą z dzielenia", to zakresy zadań dla pierwszych $r$ wątków będą powiększone o $1$.

Jeżeli mamy $n_w$ wątków i $r$ to reszta z dzielenia liczby kubełków $n_k$ przez $n_w$, to kubełki przypisane wątkowi $w$ możemy opisać następująco:

* $r \le w \implies \{ w * floor(\frac{n_k}{n_w}) + r, \ldots, (w + 1) * floor(\frac{n_k}{n_w}) + r - 1 \}$

* $r > w \implies \{ w * floor(\frac{n_k}{n_w}) + w, \ldots, (w + 1) * floor(\frac{n_k}{n_w}) + w \}$

### Usprawnienie fazy `gather`

Zastosowałem prostą optymalizację korzystającą z faktu, że wątek zliczając rozmiary kubełków należących do wątków o niższym `id`, nie potrzebuje
iterować przez wszystkie te kubełki dla każdego analizowanego przez siebie kubełka. Wystarczy, że zrobi to raz, dla pierwszego przepisywanego
przez siebie kubełka i zapamięta wartość.

## Wyniki eksperymentów

### Czas wykonania algorytmu sekwencyjnego a oczekiwany rozmiar kubełka

| ![seq](src/plots/sort/seq-256-external.png) |
|:--:|
| *Wykres 2.2 Czas wykonania algorytmu sekwencyjnego a rozmiar kubełka, argmin: 409* |


Na powyższym wykresie został przedstawiony czas wykonania algorytmu sekwencyjnego w zależności od rozmiaru kubełka. Są to wyniki pomiarów uzyskane przez jednego z członków mojego zespołu.
Wyniki uzyskiwane przeze mnie były fatalne, **chociaż obydwaj uruchamialiśmy tą samą wersję algorytmu** (implementowaną przeze mnie).

Przedstawiam je poniżej:

| ![bad-seq](src/plots/sort/seq-256-2023-04-19-15-00-00.png) |
|:--:|
| *Wykres 2.3 Czas wykonania algorytmu sekwencyjnego a rozmiar kubełka (moje pomiary), argmin: 3976* |


Jedyne różnica to obecność flagi `-O2` przy kompilacji (która powinna raczej suprawnić pracę algorytmu). Takie sytuacje zdażają się jeszcze w kilku miejscach (jednakowy kod daje różne rezultaty w zależności od tego kto mierzy...) -- np. przy skalowaniu sortowania, o czym niżej.
Naturalnym wnioskiem jest to, że słabo kontrolujemy środowisko eksperymentu, tzn. są jakieś zmienne których nie bierzemy pod uwagę i to one wpływają na różnice w wynikach -- brak sugestii.

Wobec powyższego zdecydowaliśmy w grupie, że będziemy przeprowadzać obliczenia dla rozmiaru kubełka 500 i 409, a ja we własnym zakresie przeprowadziłem jeszcze pomiary dla swojego minimum: 3976 (wyniki były jednak znacznie gorsze, dlatego dalej nie prezentuję wykresów dla tego rozmiaru kubełka).

Wyniki znajdują sie na kolejnych wykresach.

### Czas wykonania algorytmu równoległego a liczba wątków


| ![par-time](src/plots/sort/par-time-256-409-2023-04-23-10-47-57.png) |
|:--:|
| *Wykres 2.4 Czas wykonania algorytmu równoległego a liczba wątków* |

Na powyższym wykresie czasu wykonania algorytmu równoległego w zależności od liczby wątków widzimy, że ogólna charakterystyka wykresu jest zgodna z oczekiwaniami teoretycznymi (zachowanie jak $1/x$), jednak ogólna
wydajność (niebieska linia) jest niższa od idealnej (pomarańczowa, przerywana linia). Powody tego są lepiej widoczne na kolejnych wykresach.


| ![par-bar-time](src/plots/sort/par-bar-time-256-409-2023-04-23-10-47-57.png) |
|:--:|
| *Wykres 2.5 Czas wykonania algorytmu równoległego a liczba wątków, zestawienie poszczególnych faz* |



| ![par-sp](src/plots/sort/par-sp-256-409-2023-04-23-10-47-57.png) |
|:--:|
| *Wykres 2.6 Przyśpieszenie algorytmu równoległego a liczba wątków, wraz z przyśpieszeniem poszczególnych faz* |

Z powyższych wykresów widzimy, że fazy `draw` i `scatter` skalują się bardzo dobrze, natomiast `sort` i `gather` znacznie gorzej (są wąskim gardłem algorytmu).

W przypadku fazy `sort` oczekiwałem przyśpieszenia liniowego (i takie też otrzymali moi koledzy) -- jednak jest ono gorsze. Zastosowałem `std::sort` z biblioteki standardowej, który jest zaimplementowany
jako miks `insertion sort`, `quick sort` i `heapsort` (wydajna implementacja), z tym, że nie jest ona realizowana w miejscu -- i to jest przypuszczalny powód takiego wyniku -- większa ilość kopi mogła negatywnie wpłynąć na wydajność.

W przypadku `gather` wynika to z pewnością (przynajmniej częściowo) z sposobu pomiaru czasu -- wyniki brane są tylko z wątku zerowego, który przy użyciu klauzuli `#pragma for schedule(static)` będzie
zajmował się początkowymi kubełkami -- co w połączeniu z optymalizacją [`Usprawnienie fazy gather`](#usprawnienie-fazy-gather) sprawia, że nie potrzebuje on w ogóle iterować przez kubełki
w celu zliczenia ich rozmiarów (a kubełków potencjalnie może być dużo, u mnie to $\frac{1}{400}$ liczby elementów).

Po przeprowadzeniu dodatkowych eksperymentów okazało się, że kluczowa jest tutaj kompilacja z flagą `-O2`! Oto wykresy czasu i przyśpieszenia algorytmu równoległego uzyskanego dla programu skompilowanego bez tej flagi:


| ![par-sp-no-o2](src/plots/sort/par-sp-256-409-2023-04-23-12-36-08.png) |
|:--:|
| *Wykres 2.7 Przyśpieszenie algorytmu równoległego skompilowanego bez `-O2`* |



| ![cmp-o2-time](src/plots/sort/cmp-time-409-2023-04-23-10-47-57-2023-04-23-12-36-08.png)
|:--:|
| *Wykres 2.8 Czas wykonania algorytmu równoległego a liczba wątków (program skompilowany z i bez `-O2`)* |



| ![cmp-o2-sp](src/plots/sort/cmp-sp-409-2023-04-23-10-47-57-2023-04-23-12-36-08.png) |
|:--:|
| *Wykres 2.9 Przyśpieszenie równoległego a liczba wątków, zestawienie poszczególnych faz (program skompilowany z i bez `-O2`)* |

Proszę zauważyć, że:

1. Ogólne czasy wykonania algorymu wzrosły (1.5x - 2x krotnie)
2. Nie ma już superskalowania dla fazy `scatter`
3. Faza `gather` skaluje się w końcu zgodnie z oczekiwaniami!
4. Algorytm skompilowany z flagą `-O2` zyskuje szczególnie w fazach `scatter` i `gather`.

Wnioskuję z tego następujące fakty:

1. Superskalowanie w fazie `scatter` wynikało z optymalizacji dokonanych przez kompiltor, przypuszczalnie w organizacji tego jakie dane znajdują się w jakim momencie w pamięci cache aplikacji -- dzięki czemu
	wątki wykonywały znacznie mniej odczytów z pamięci RAM.
2. Faza `gather` mogła zostać przypuszczalnie **znacznie** zoptymalizowana i zredukowana do pojedynczych wowołań `std::memcpy` - kopiującej przy jednym wywołaniu całe regiony pamięci, a nie tak jak jest to zapisane w algorytmie: element po elemencie.
   W takim wypadku pozostaje tam bardzo mało do skalowania (pracy do rozbijania pomiędzy wątki) (wraz ze wzrostem liczby kubełków zwiększa się tylko liczba potrzebnych wywołań). Zwracam uwagę także na to, że bez flagi `-O2` faza `gather` przyśpiesza lepiej
	 (zgodnie z oczekiwaniami) ale pomimo przyśpieszenia ciągle działa wolniej niż w programie skompilowanym z tą opcją.

### Porównanie z algorytmem w wersji 2

Autorem drugiej implementacji jest Adam Niemiec. Jest ona pochodną implementacji algorytmu 1 i korzysta z takich samych typów i struktur danych, tak samo mierzony jest czas itd.

Jako, że wyniki dla algorytmu drugiego zostały pozyskane dla programu skompilowanego bez flagi `-O2` to też z taką wersją swoich wyników go porównuję.

| ![cmp-ext-time](src/plots/sort/cmp-time-409-2023-04-23-12-36-08-external.png)
|:--:|
| *Wykres 2.10 Porównanie czsau wykonania algorytmów w wersjach 1 i 2 (zgodnie z oznaczeniami z UPEL)*|


| ![cmp-ext-sp](src/plots/sort/cmp-sp-409-2023-04-23-12-36-08-external.png)
|:--:|
| *Wykres 2.11 Porównanie przyśpieszenia algorytmów w wersjach 1 i 2 (zgodnie z oznaczeniami z UPEL)* |


Na podstawie wykresu 2.10 widzimy, że w ogólności implementacja algorytmu 1 jest znacznie szybsza (1.2x-2x) od implementacji algorytmu 2. Na części wykresu 2.10 poświęconej poszczególnym fazom, możemy zauważyć, że miejscem gdzie
algorytm 1 uzyskuje przewagę jest faza `scatter`. Faktycznie, implementacja algorytmu 2 tylko tam się różni -- w odróżnieniu od algorytmu 1, gdzie każdy wątek wpisuje liczby tylko do przypisanych sobie kubełków, tutaj każdy wątek może potencjalnie
wpisywać do każdego z kubełków z czego wynika potrzeba synchronizacji. Najprawdopodobniej to właśnie synchronizacja jest powodem słabsezj wydajności algorytmu 2 w tej fazie. Obserwujemy także, że w faze `sort` algorytm 1 radzi sobie lepiej (do 8 wątków, potem różnica mieści się w błędzie pomiarowym).
Jest to trudne do wytłumaczenia, jako, że implementacja tej fazy w obu algorytmach jest dokładnie taka sama. Pozostałe dwie fazy `draw` i `gather` dają właściwie jednakowe wyniki (ich implementacja też jest jednakowa).

Na wykresie przyśpieszenia 2.11 obserwujemy że algorytm drugi ma je średnio większe. Jest to głównie konsekwencja fazy `sort`, która prezentuje lepsze przyśpieszenie dla algorytmu 2., jednak trzeba tutaj zaznaczyć, że to w algorytmie 1 we wszystkich punktach pomiarowych
osiągnięty czas jest lepszy.

# Kod źródłowy

W załączniku...
