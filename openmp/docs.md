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
		- [Uniknięcie potrzeby synchronizacji](#uniknięcie-potrzeby-synchronizacji)
		- [Usprawnienie fazy `gather`](#usprawnienie-fazy-gather)
	- [Wyniki eksperymentów](#wyniki-eksperymentów)
- [Kod źródłowy](#kod-źródłowy)

# Zadanie 1

Celem zadania był pomiar czasu i przyśpieszenia wykonania programu wypełniającego
strukturę danych liczbami losowymi w zależności od liczby liczb oraz konfiguracji OpenMP (klauzuli `schedule`).

Zdecydowałem się na skalowanie silne.

# Wyniki

Poniżej zamieszczam serię wykresów prezentujacych pozyskane wyniki.

![Dynamic 1](src/plots/draw/combined-dynamic-1.png)

*Podpis*

![Dynamic 4](src/plots/draw/combined-dynamic-4.png)

*Podpis*

![Dynamic 16](src/plots/draw/combined-dynamic-16.png)

*Podpis*

![Dynamic 256](src/plots/draw/combined-dynamic-256.png)

*Podpis*

![Guided 1](src/plots/draw/combined-guided-1.png)

*Podpis*

![Guided 4](src/plots/draw/combined-guided-4.png)

*Podpis*

![Guided 16](src/plots/draw/combined-guided-16.png)

*Podpis*

![Guided 256](src/plots/draw/combined-guided-256.png)

*Podpis*

![static 1](src/plots/draw/combined-static-1.png)

*Podpis*

![static 4](src/plots/draw/combined-static-4.png)

*Podpis*

![static 16](src/plots/draw/combined-static-16.png)

*Podpis*

![static 256](src/plots/draw/combined-static-256.png)

*Podpis*

# Wnioski

Dla eksperynemnut `dynamic` coś jest zrobione źle.

Wyniki nie są zgodne z oczekiwaniami. Oczekiwane charakterystyki to spadek $1/x$ dla czasu oraz
liniowy wzrost dla przyśpieszenia.

Najprawdopodobniejsza przyczyna to błąd w kodzie / konfiguracji. Nie jest to raczej wina vClustr'a
bo niezgodność jest zbyt duża.

Dla pozostałych eksperymentów wykresy są zgodne z oczekiwanymi charakterystykami.
Widzimy, że ustawienie parametru chunk "na sztywno" nie przynosi dobrych efektów.

`Guided` i `static` na `auto` osiągają podobne wyniki (najlepsze)

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

Wszystkie pomiary / eksperymenty przeprowadzone zostały na komputerze "Ares" (pojedynczy węzeł: `	48 cores, Intel(R) Xeon(R) Platinum 8268 CPU @ 2.90GHz`, `3,85 GB RAM`).
Ze względu na [szczegóły implementacyjne](#uniknięcie-potrzeby-synchronizacji) kod programu sortującego
(napisanego w `C++`) kompilowany był z użyciem `gcc 10.3.0` z flagami `-std=c++11 -fopenmp -O2`.

Wersja `gcc` była podyktowana faktem, że w kodzie korzystam z szczegółu implementacyjnego `OpenMP` i lokalnie,
gdzie program był testowany przed przeprowadzaniem właściwych eksperymentów, korzystam właśnie z `gcc 10.3.0`.

## Szczegóły implementacji

### Założenia

Zaimplementowany algorytm zakłada:

1. Liczba kubełków jest większa bądź równa liczbie wątków.
2. Liczba elementów w sortowanej tablicy jest więszka bądź równa liczbie wątków.

Brak powyższych założeń wprowadzałby niepotrzebne komplikacje do kodu, który musiałby w specjalny
sposób obsługiwać te przypadki.

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

### PRNG

Wykorzystano `erand48`, który jest generatorem kongruencyjnym (nie jest to podejście które daje najlepsze rezultaty) dającym
docelowo rozkład jednostajny.

Przeprowadziłem prostą weryfikację tego generatora, generując ok. 16 mln. liczb z przedziału $[0, 1)$ i rozmieszczając je do 32. równomiernych kubełków.
Wynik tego eksperymentu przedstawiam na poniższym wykresie.

![prng plot](src/plots/sort/prng-distribution-16777216-32.png)

*Rozkład generatora `erand48`*

Widzimy na powyższym wykresie, że przy przy dużej liczbie elementów w tablicy generator faktycznie daje próbki z rozkładu jednostajnego. Charakterystyka ta nie jest zachowana,
jeżeli przeprowadzimy obliczenia dla małej liczby elementów, rzędu 100-1000. Nie stanowi to jednak problemu, gdyż wszystkie eksperymenty były przeprowadzane na znacznie
większej liczbie elementów (ok. 33 mln.).


### Zrównoleglanie pętli `for`

Zgodnie z wynikami Zadania 1, do zrównoleglania pętli `for` wykorzystałem klauzulę `schedule(static)`, która dawała najlepsze rezultaty.

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

![seq](src/plots/sort/seq-256-external.png)
![par-time](src/plots/sort/par-time-256-409-2023-04-22-10-07-50.png)
![par-bar-time](src/plots/sort/par-bar-time-256-409-2023-04-22-10-07-50.png)
![par-sp](src/plots/sort/par-sp-256-409-2023-04-22-10-07-50.png)

# Kod źródłowy

W załączniku...
