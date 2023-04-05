# Co na następny raz:

wykres - czas sortowania sekwencyjnego od wielkości kubełka dla danej wielkości tablicy (duża wielkość tablicy)
spodziewamy się jakiejś paraboli

Dzięki temu znajdujemy w miarę sensowny rozmiar kubełka i taką wersję potem wykorzystujemy do zrównoleglania.

Można oczywiście rozmiarem kubełka się jeszcze potem pobawić (przy programie równoległym).

Drugi wykres to speedup od ilości wątków.

Speedup dla całości programu równoległego + speedupy wyliczone dla poszczególnych części algorytmu równoległego:

1. losowanie liczb
2. rozkład z tablicy do kubełków
3. sortowanie kubełków
4. przepisanie z kubełków to tablicy wynikowej
5. całość (zmierzyć raz, nie sumować tych czasów)

Chodzi o sprawdzenie, które części zrównoleglają się dobrze, a której są wąskim gardłem.

Trzeci wykres czas od liczby wątków w formie słupkowej: czas dla danego wątku składa się z losowania liczb, rozkład, sortowanie, przepisanie, ...
Chodzi o pokazanie tego, która składowa zajmuje najwięcej.


Na następny raz mamy mieć jeszcze gotowy algorytm równoległy (nie musi być idealny)

Czyli 3 wykresy i kod.


Trzeba koniecznie jeszcze znaleźć parę, ogarnąć dane od kogoś i przeprowadzić analizę porównawczą.


Szkice algorytmów są opisane na upelu