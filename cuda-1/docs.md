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

# Środowiko i metodologia

Wszystkie pomiary opisane w sekcjach dla poszczególnych zadań zostały wykonane w środowisku Google Colab na karcie graficznej Nvidia Tesla T4. Jej parametry przedstawiam w poniższej tabeli:

|Hardware | Tesla T4 |
|------|------|
| Clock rate (GHz) | 1.59 |
| # cores | 40 |
| # FP64 AUs per core | x |
| # FP32 AUs per core | **64** |
| # FP16 AUs per core | **8** |
| cache per core (KB) | 64 |
| shared cache (MB)| 6 |
| Memory (GB) | 16 |
| Max memory bandwidth (GB/sec) | 300 |
| FP64 TFLOPS | x |
| FP32 TFLOPS | 8.1 |
| FP16 TFLOPS | 65 |

Każdy pomiar został wykonany trzykrotnie, a wyniki prezentowane na wykresach to średnie z tych pomiarów (wraz z wyliczonych odchyleniem standardowym).
Niestetty największy rozmiar bloku w zadaniu drugim na jakim udało się przeprowadzić obliczenia to 64 (jeden wymiar). Dla większych środowisko zwracało błąd o przekroczeniu rozmiaru dostępnej pamięci współdzielonej:

```
ptxas error   : Entry function '_Z23matrix_transpose_sharedPiS_' uses too much shared data (0x10000 bytes, 0xc000 max)
```

# Zadanie 1

## Cel

Celem pierwszego zadania było wykonanie pomiarów czasu wykonania operacji dodawania wektorów, w zależności od ich wymiaru oraz rozmiaru bloków procesorów.

## Wyniki i wnioski

| ![exp-1-wyniki](ex1_vector_addition_gpu/plots/exp-1-time-by-arr-size.png) |
|:--:|
| *Wykres 1 Czas wykonania w zależności od rozmiaru tablicy dla danej liczby wątków i liczby bloków. Liczba bloków to (\<rozmiar tablicy> / <liczba wątków>)* |

Na powyższym wykresie 1 obserwujemy, że:

1. 32 wątki na blok to zbyt mało do osiągnięcia sensownych czasów (jako że liczby wątków w bloku to wielokrotności 32, to jest to prawie minimalna wartość).
2. Dla liczby wątków na blok > 32, przy dostosowanej liczbie bloków (tak, że <liczba bloków> x <liczba wątków na blok> ~= <rozmiar tablicy>), widzimy ustabilizowanie się czasów -- nie zależą one już od ilości wątków.
   Jest to zgodne z oczekiwaniami, ponieważ przy tak dobranych parametrach, niezależnie od testowanej konfiguracji dla każdego wątku przypada pojedyncze wykonanie kernela. Gdyby utrzymać stałą liczbę bloków, przy zmieniającej się liczbie wątków, wtedy spodziewalibyśmy się spadku czasu wykonania w zależności od liczby wątków.


# Zadanie 2

## Cel

Celem zadania drugiego było porównanie efektywności prostych algorytmów transpozycji macierzy, korzystających z różnych typów pamięci: globalen i/lub współdzielonej.
Porównanie miało uzwględniac rozmiar bloku, liczbę bloków i rozmiar macierzy.

## Wyniki i wnioski

| ![exp-2-wyniki-agg](ex2_matrix_transpose_gpu/plots/exp-2-time-by-matrix-size-all-block-sizes.png) |
|:--:|
| *Wykres 2 Czas wykonania w zależności od rozmiaru macierzy dla danej liczby wątków i liczby bloków. Liczba bloków to (\<rozmiar macierzy> / <liczba wątków>)* |

| ![exp-2-wyniki-4096](ex2_matrix_transpose_gpu/plots/exp-2-time-by-matrix-size-4096-block-size.png) |
|:--:|
| *Wykres 3 Czas wykonania w zależności od rozmiaru macierzy dla danej liczby wątków i liczby bloków. Liczba bloków to (\<rozmiar macierzy> / <liczba wątków>)* |


Na powyższych wykresach obserwujemy:

1. Algorytm wykorzystujący pamięć współdzieloną jest szybszy (przy tym samym rozmiarze bloku) od algorytmu wykorzystującego tylko pamięć globalną (wykresy 2, 3). Jest to efekt całkowicie zgodny z oczekiwaniami, jako, że pamięć globalna
   jest znacznie wolniejsza od pamięci współdzielonej. Przez to przeniesienie cześci obliczeń do znacznie wydajniejeszej pamięci pozwala na uzyskanie lepszego czasu wykonania, tym bardziej, że operacja odwracania macierzy sprowadza się prawie wyłącznie do kopiowania / modyfikacji poszczególnych rejonów pamięci, nie ma tam za bardzo żadnych dodatkowych obliczeń.
2. Podobnie jak w zadaniu 1, możemy wywnioskować, że liczba wątków na blok rzędu $32^2$ jest zbyt niska do osiągnięcia sensownej efektywności.
   Obserwujemy znaczą przewagę w czasie wykonania dla konfiguracji operujących na większej liczbie bloków $64^2$.
3. Wnioskiem całkowicie subiektywnym jest także większa stabliność (w sensie wachania się czasu obliczeń) obliczeń prowadzonych na GPU. Opieram to spostrzeżenie na *bardzo* niskich odchyleniach standardowych w porównaniu do obliczeń prowadzonych wyłącznie na CPU podczas wcześniejszych zajęć z tego przedmiotu. Zaznaczam tu jednak, że tam wpływ na wyniki miały mechanizmy synchronizacji (OpenMP, OpenMPI) i przede wszystkim obliczenia prowadzone były dla znacznie większych danych.
