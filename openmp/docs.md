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

# Zadanie

Celem zadania był pomiar czasu i przyśpieszenia wykonania programu wypełniającego
strukturę danych liczbami losowymi w zależności od liczby liczb oraz konfiguracji OpenMP (klauzuli `schedule`).

Zdecydowałem się na skalowanie silne.

# Wyniki

Poniżej zamieszczam serię wykresów prezentujacych pozyskane wyniki.

![Dynamic 1](src/data/plots/combined-dynamic-1.png)

*Podpis*

![Dynamic 4](src/data/plots/combined-dynamic-4.png)

*Podpis*

![Dynamic 16](src/data/plots/combined-dynamic-16.png)

*Podpis*

![Dynamic 256](src/data/plots/combined-dynamic-256.png)

*Podpis*

![Guided 1](src/data/plots/combined-guided-1.png)

*Podpis*

![Guided 4](src/data/plots/combined-guided-4.png)

*Podpis*

![Guided 16](src/data/plots/combined-guided-16.png)

*Podpis*

![Guided 256](src/data/plots/combined-guided-256.png)

*Podpis*

![static 1](src/data/plots/combined-static-1.png)

*Podpis*

![static 4](src/data/plots/combined-static-4.png)

*Podpis*

![static 16](src/data/plots/combined-static-16.png)

*Podpis*

![static 256](src/data/plots/combined-static-256.png)

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

# Kod źródłowy

W załączniku...

## Program

## Automatyzacja

## Wykresy