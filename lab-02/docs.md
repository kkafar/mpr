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
- [Komunikacja PP](#komunikacja-pp)
	- [Zadanie](#zadanie)
	- [Realizacja](#realizacja)
		- [Tech stack](#tech-stack)
		- [Przeprowadzone eksperymenty](#przeprowadzone-eksperymenty)
		- [Testowanie opóźnienia](#testowanie-opóźnienia)
		- [Testowanie przepustowości](#testowanie-przepustowości)
	- [Wnioski](#wnioski)
- [Badanie efektywności programu równoległego](#badanie-efektywności-programu-równoległego)
	- [Zadanie](#zadanie-1)
	- [Realizacja](#realizacja-1)
		- [Tech stack](#tech-stack-1)
		- [Przeprowadzone eksperymenty](#przeprowadzone-eksperymenty-1)
		- [Wyniki](#wyniki)
			- [vCluster](#vcluster)
			- [Ares](#ares)
	- [Wnioski](#wnioski-1)
- [Komunikacja PP -- kod źródłowy](#komunikacja-pp----kod-źródłowy)
	- [Kod programu testowego](#kod-programu-testowego)
	- [Makefile do automatyzacji + pliki maszynowe](#makefile-do-automatyzacji--pliki-maszynowe)
	- [Single node](#single-node)
	- [Two nodes](#two-nodes)
	- [Rysowanie wykresów](#rysowanie-wykresów)
- [Badanie efektywności programu równoległego -- kod źródłowy](#badanie-efektywności-programu-równoległego----kod-źródłowy)
- [Eksperyment](#eksperyment)
	- [Skrypt automatyzujący (uruchamianie programu i agregowanie danych)](#skrypt-automatyzujący-uruchamianie-programu-i-agregowanie-danych)
	- [Rysowanie wykresów](#rysowanie-wykresów-1)
- [Dane - PP](#dane---pp)
	- [Przepustowość -- single node](#przepustowość----single-node)
	- [Przepustowość -- two nodes](#przepustowość----two-nodes)
	- [Opóźnienie -- single node](#opóźnienie----single-node)
	- [Opóźnienie -- two nodes](#opóźnienie----two-nodes)
- [Dane - Badanie ...](#dane---badanie-)
	- [Ares](#ares-1)
	- [vCluster](#vcluster-1)

<!-- <div style="page-break-after: always;"></div> -->

# Komunikacja PP

## Zadanie

Celem pierwszego zadania było przetestowanie dwóch równych typów komunikacji `point to point` w `MPI`, poprzez
zmierzenie przepustowości oraz opóźnienia komunikacyjnego (narzut) w dwóch konfiguracjach:

1. komunikacja pomiędzy dwoma procesorami tego samego węzła (nazywana dalej `single node`)
2. komunikacja pomiędzy dwoma procesorami z różnych węzłów (nazywana dalej `two nodes`)

## Realizacja

### Tech stack

* Kod do przeprowadzania testów został zaimplementowany w `C`
* Rysowanie wykresów i analiza w `Python 3.11` z wykorzystaniem pakietów
  `numpy`, `pandas`, `matplotlib`
* Implementacja MPI -- taka jak na `vCluster` -- prawdopodobnie `OpenMPI` ale bez pewności

### Przeprowadzone eksperymenty

Eksperymenty zostały przeprowadzone w obydwu konfiguracjach opisanych w poleceniu.

Testowane typy komunikacji:

1. Wysyłanie `MPI_Send`, odbieranie `MPI_Recv` (dalej nazywany `std`)
2. Wysyłanie buferowane `MPI_Bsend`, odbieranie `MPI_Recv` (dalej nazywany `buff`)

W każdej z konfiguracji rozłożenie węzłów `x` typ komunikacji testowane były przepustowość i opóźnienie.

### Testowanie opóźnienia

W celu oszacowania opóźnienia przygotowano eksperyment z dwoma komunikującymi się procesami.
Przebieg komunikacji został zilustrowany poniżej.

![Eksperyment - opóźnienie](playground/src/data/lab1/experiment_delay.png)

*Rysunek 1 - Komunikacja procesów przy pomiarach opóźnienia*

Proces A wysyła wiadomość do procesu B, a następnie czeka na odpowiedź.
Wykonując pomiar czasu trwania segmentu wydzielonego czerwonymi, przerywanymi liniami w procesie A
otrzymujemy dwukrotność oszacowania kosztu czasowego przesyłu wiadomości.

Istotne jest również to, że przesyłamy możliwie mały komunikat -- jeden bajt -- dzięki czemu mierzymy czas
możliwie bliski samemu narzutowi komunikacyjnemu.

Wyniki zaprezentowane w poniższej tabeli, to wyniki uśrednione z `N = 10000` pomiarów.

| Typ komunikacji | Konfiguracja | Czas [us] |
| ----------------- |-------------- | ------ |
| `std` | `single node` | 0.28 |
| `buff` | `single node` | 2.25 |
| `std` | `two nodes` | 429.87 |
| `buff` | `two nodes` | 483.27 |


### Testowanie przepustowości

Pomiary przeprowadzono w sposób zupełnie analogiczny do testowania opóźnienia, z tą różnicą,
że zostały wykonane dla różnych rozmiarów wiadomości (nie tylko 1B).

Testowany zakres to `1 KB` - `1000 KB` dla `single node` oraz `1 KB` - `100 KB` dla `two nodes`
z rodzielczością `1 KB`. Różnica w przedziałach wynika z faktu czasu trwania obliczeń dla `two nodes`
(prawdopodobnie proces był ubijany -- timeout, stacktrace nie sugerował błędu wykonania).

Przepustowość została wyliczona następująco:

`<przepustowość> = <rozmiar wiadomosci> * <liczba przesłanych wiadomości (proces A)> * 2.0 / <całkowity czas>`

Licznik został przemnożony przez 2, ponieważ każda przesłana przez proces A wiadomość miała towarzyszącą jej odpowiedź z procesu B.

**Uwaga** Przy komunikacji buferowanej, rozmiar bufora został arbitralnie ustawiony na czterokrotność największej zaplanowanej wiadomości -
tj. odpowiednio `4000 KB` i `400 KB`. Chodziło o to, żeby bufor zawsze mieścił więcej niż jedną wiadomość.

Nie zamieszczam tabel, ponieważ danych jest zbyt wiele. Poniżej przygotowane wykresy.
`
![przepustowość - single node](playground/src/data/lab1/single_node_Mb_s_low.png)

*Rysunek 2 - Przepustowość w zależności od rozmiaru przesyłanej wiadomości -- konfiguracja `single node`*


![przepustowość - two nodes](playground/src/data/lab1/two_nodes_Mb_s_low.png)

*Rysunek 3 -Przepustowość w zależności od rozmiaru przesyłanej wiadomości -- konfiguracja `two nodes`*

## Wnioski

1. Wbrew moim oczekiwaniom znacznie szybsza jest komunikacja poprzez `MPI_Send` -- [**prawdopodobnie**](https://www.mcs.anl.gov/research/projects/mpi/sendmode.html)
synchroniczna. Być może moment `flush` bufora powoduje duże opóźnienia (dużo danych do przesyłu na raz?) -- ta hipoteza wymagałaby dalszych pomiarów.

2. Jest ogromna przepaść pomiędzy (~18x) pomiędzy komunikacją w ramach jednego węzła, a różnymi węzłami
3. Na wykresie 2 widzimy (wypłaszczenie wykresu od rozmiaru wiadomości ~400 KB), że przepustowość systemu wynosi ~63 Gb/s dla komunikacji niebuforowanej
   ~41 Gb/s dla komunikacji buferowanej. Ciężko określić jak dobrym przybliżeniem fizycznej przepustowości łącza jest ~63 Gb/s.

	 Początkowy wzrost przepustowości powodowany jest faktem, że system nie napotkał jeszcze ograniczeń i "na bieżąco" obsługiwał wiadomości.

4. W przypadku komunikacji pomiędzy dwoma węzłami (wykres 3) widzimy większą niestabliność przepustowości (policzyć odchylenie). Tutaj również lepiej
   działa komunikacja synchroniczna. Niestabilność jest najprawdopodobniej kwestią tego jak działa vcluster -- próbowałem liczyć dla różnych rozmiarów wiadomości, różnych rozdzielczości
	 (odległość pomiędzy rozmiarami wiadomości). Próbowałem nawet czekania pomiędzy wysyłaniem pojedynczych batchy wiadomości (np. 15 wiadomości i sleep na 300 ms) -- nie przyniosło to żadnej poprawy.
	 Generalnie oczekiwana charakterystyka jest jedankowa z tą z rysunku 2, z tym, że dla niższych wartości przepustowości.

5. Na wykresie 2 pomiędzy rozmiarem wiadomości `70 KB` a `71 KB` (`std`) widzimy spadek przepustowości -- jest on najprawdopodobniej spowodowany sposobem implementacji
  operacji `MPI_Send`, która w zależności od rozmiaru wiadomośći stosuje różne rozwiązania.

6. Zakłócenia na wykresie 2 (`buf`) w okolicach `200 KB` i `400 KB` prawdopodobnie są spowodowane zmianami w obiążeniu na vclustrze (inny użytkownik przestał / zaczął wykonywać obliczenia).
   Podobnie dla `std` przy `800 KB`. Pozostałe skoki to kwestia "stabilności" vclustra -- nie udało mi się ich zniwelować pomimo testowania różnych rozmiarów wiadomości, różnych kroków, itd.

7. Charakterystyka z rysunku 2 jest całkowicie zgodna z oczekiwaniami teoretycznymi -- początkowo z powodu
   małych rozmiarów wydajności nie wykorzystujemy całej potencjalnej przepustowości. Gdy już ją osiągamy -- sytuacja się stablizuje.


**Note**: Nie filtrowałem danych pod względem eliminacji "elementów odstających" -- nie zakłucają ostatecznych wniosków (w przypadku komunikacji wewnątrz pojedynczego węzła).

# Badanie efektywności programu równoległego

## Zadanie

Celem zadania było zbadanie charakterystki problemu równoległego na przykładzie naturalnie równoległego
problemu obliczania liczby $\pi$ metodą Monte Carlo, gdzie część sekwencyjna występuje jedynie
przy końcowej agregacji wyników obliczeń.

## Realizacja

### Tech stack

Podobnie jak przy pierwszym zadaniu: `C`, `Python@3.11`, `pyplot` i zamiast `pandas` -- wspaniały `polars`.

Przydział technologii do zadań jak przy zadaniu 1.

### Przeprowadzone eksperymenty

Zgodnie z opisem zadania wykonane zostały pomiary czasu w zależności od rozmiaru problemu -- w dwóch konfiguracjach skalowania:
`słabego` (rozamiar problemu skaluje się proporcjonalnie do liczby procesorów), `slinego` (rozmiar problemu jest stały ze względu na liczbę procesorów).

Na vClustrze wykonano `S = 5` serii obliczeń dla liczby punktów `n = 1e9` i liczby procesorów `p = 1, 2, ..., 12`.

Na Aresie wykonano `S = 10` serii obliczeń dla liczby punktów `n = 1e3, 1e7, 1e11` i liczby procesorów `p = 1, 2, ..., 12`.

### Wyniki

Poniżej zamieszczam sporządzone wykresy. Surowe dane znajdują się na samym końcu sprawozdania.

#### vCluster

![Wynik vCluster strong](playground/src/data/plots/vcluster-combined-s-1e+09.png)
*Rysunek 4 Wyniki obliczeń na vClustrze - skalowanie silne, rozmiar problemu bazowego: 1e9 punktów*


![Wynik vCluster weak](playground/src/data/plots/vcluster-combined-w-1e+09.png)
*Rysunek 5 Wyniki obliczeń na vClustrze - skalowanie słabe, rozmiar problemu bazowego: 1e9 punktów*


#### Ares

![Wyniki Ares strong 1e3](playground/src/data/plots/ares-combined-s-1e+03.png)
*Rysunek 6 Wyniki obliczeń na Aresie - skalowanie silne, rozmiar problemu bazowego: 1e3 punktów*

![Wyniki Ares strong 1e7](playground/src/data/plots/ares-combined-s-1e+07.png)
*Rysunek 7 Wyniki obliczeń na Aresie - skalowanie silne, rozmiar problemu bazowego: 1e7 punktów*

![Wyniki Ares strong 1e11](playground/src/data/plots/ares-combined-s-1e+11.png)
*Rysunek 8 Wyniki obliczeń na Aresie - skalowanie silne, rozmiar problemu bazowego: 1e11 punktów*

![Wyniki Ares weak 1e3](playground/src/data/plots/ares-combined-w-1e+03.png)
*Rysunek 9 Wyniki obliczeń na Aresie - skalowanie słabe, rozmiar problemu bazowego: 1e3 punktów*

![Wyniki Ares weak 1e7](playground/src/data/plots/ares-combined-w-1e+07.png)
*Rysunek 10 Wyniki obliczeń na Aresie - skalowanie słabe, rozmiar problemu bazowego: 1e7 punktów*

![Wyniki Ares weak 1e11](playground/src/data/plots/ares-combined-w-1e+11.png)
*Rysunek 11 Wyniki obliczeń na Aresie - skalowanie słabe, rozmiar problemu bazowego: 1e11 punktów*

## Wnioski

Omówię najpierw jakie są oczekiwane charakterystyki. Notacja intuicyjna: `T(<liczba procesorów>, <rozmiar_problemu>)` -- funkcja czasu.

W przypadku skalowania silnego przeprowadzamy pomiary dla stałego rozmiaru problemu i zmiennej liczby procesorów.
Z prawa Amdahla, gdy zaniedbamy część sekwencyjną (w naszym problemie jest znikoma) natychmiast otrzymamy, że
funkcja czasu powinna się zachowywać $1/x$, ponieważ oczekujemy, że przy małej części sekwencyjnej
$T(p, n) = T(1, n) / p$.
Oczywście część sekwencyjna w prawie Amdahla, nawet "zaniedbywalna" - przy skali (liczba procesorów) poważnie
wpływa na to jakie finalnie przyśpieszenie jesteśmy w stanie uzyskać.

Przyśpieszenie definiujemy jako $S_n = T(1, n) / T(p, n)$, co idąc za wcześniejszym założeniem daje
$S_p = T(1, n) / ( T(1, n) / p ) = p$ -- spodziewamy się charakterystki liniowej.

Dalej już łatwo otrzymujemy efektywność definiowana jako $S_p / p$ powinna być stała -- $y = 1$
oraz część sekwencyjna $SF_p = (1 / S_p - 1 / p) / (1 - 1 / p)$ powinna być stała -- $y = 0$.

Dwie ostatnie zależności określamy tylko dla $p > 1$.

Natomiast w przypadku skalowania słabego rozmiar problemu skaluje się proporcjonalnie do liczby procesorów.
Spodziewamy się zatem, że czas wykonania pozostaje stały i nie zależy od liczby procesorów. Idąc teraz analogicznym rozumowaniem
otrzymamy, że pozostałe charakterystyki powinny być identyczne jak wcześniej.

Wyniki pomiarów uzyskane na vClustrze w pełni realizują oczekiwania teoretyczne (rysunki 4 i 5).
Obserwujemy jedynie małe odchylenia spowodobane "stabilnością" vClustra.
Na wykresie przyśpierszenia (skalowanie silne) momentami wydaje się, że mamy nawet superskalowanie (które potem przenosi się na pozostałe wykresy).
Powody mogą być różne: albo sam vCluster "postanowił" znacząco przyśpieszyć, albo pomiar czasu przez
funkcję `MPI_Wtime` jest prowadzony z zbyt małą rozdzielczością + błędy numeryczne (wątpliwe ?).
Linia teoretyczna (pomarańczowym kolorem) na wykresie czasu (rysunek 4) jest wyżej niż większość punktów pomiarowych,
ponieważ akurat przy pierwszym pomiarze vCluster "postanowił" zwolnić.

Wyniki pomiarów uzyskane na Aresie dla liczb punktów `1e7, 1e11` właściwie idealnie pokrywają
się z oczekiwaniami teoretycznymi (przestrzenie pomiędzy seriami zdają się być duże na wykresach, ale to kwestia bardzi drobnej rozdziałki).
Podobnie jak w przypadku vClustra miejscami widzimy superskalowanie, ale wydaje mi się, że powody mogą być podobne -- obwiniałbym raczej
jakoś pomiarów a nie sądził że faktycznie mamy superskalowanie. Na rysunku 11, wykresie przyśpieszenia (na innych też) widzimy, że
przyśpieszenie zaczyna maleć przy wzroście liczby procesorów -- co także jest oczekiwane -- prawdopopodobnie jest to
już wpływ części sekwencyjnej.

W przypadku `1e3` punktów -- wydaje się, że ta liczba była zdecydowanie za mała i zmierzony czas wykonania programu
(obydwa typy skalowania) jest w większości szumem komunikacyjnym, niedokładnością pomiaru *i **relatywnie** dużym wpływem części sekwencyjnej na całość wykonania*.
Wydaje się, że przy takim rozmiarze problemu zrównoleglanie w modelu skalowania silnego nie ma sensu -- sam narzut komunikacyjny jest zbyt duży.
Skalowanie jset wyraźnie gorsze, co przekłada się rozjechanie się pozostałych, pochodnych metryk.

Wykresy o których nie wspominam wprost, po prostu są zgodne z oczekiwaniami.


# Komunikacja PP -- kod źródłowy

## Kod programu testowego

```c
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#ifndef ITERATION_COUNT
#define ITERATION_COUNT 10
#endif

// in bytes
#ifndef MSG_MIN_SIZE
#define MSG_MIN_SIZE 1024
#endif

#ifndef MSG_MAX_SIZE
#define MSG_MAX_SIZE 1024
#endif

#ifndef MSG_SIZE_STEP
#define MSG_SIZE_STEP 1024
#endif

#define S_TO_MS_FACTOR 1e6

typedef int (*FnSendHandle)(const void *, int, MPI_Datatype, int, int, MPI_Comm);
typedef int (*FnRecvHanlde)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);

typedef struct ExperimentConfig {
  char *name; // non owning
  char *description; // non owning
  FnSendHandle sendhandle;
  FnRecvHanlde recvhandle;
  char *logfilename; // non owning
  void *params; // non owning
} ExperimentConfig;

char g_hostname[MPI_MAX_PROCESSOR_NAME];
int g_rank, g_size, g_hostname_len;

bool init_global_state(void) {
  assert((MPI_Comm_rank(MPI_COMM_WORLD, &g_rank) == MPI_SUCCESS) && "Valid rank returned");
  assert((MPI_Comm_size(MPI_COMM_WORLD, &g_size) == MPI_SUCCESS) && "Valid size returned");

  char *mem = (char *) malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME);
  if (mem == NULL) {
    printf("Failed to allocate memory for processor name in process %d\n", g_rank);
    return false;
  }
  assert((MPI_Get_processor_name(g_hostname, &g_hostname_len) == MPI_SUCCESS) && "Valid processor name");
  return true;
}

bool teardown_global_state(void) {
  return true;
}

void log_info(char * info) {
  if (info == NULL) info = "";
  printf("[%s][%d/%d] I %s\n", g_hostname, g_rank, g_size, info);
  fflush(stdout);
}

void experiment_throughput(ExperimentConfig cfg) {
  if (g_rank == 0) log_info(cfg.description);

  FILE *logfile;
  if (g_rank == 0) {
    logfile = fopen(cfg.logfilename, "w");
    if (logfile == NULL) {
      log_info("Failed to open logfile");
      return;
    }
    fprintf(logfile, "type,msgsize,time,throughput\n");
  }

  // it should round down
  int msg_sizes_arr_size = (MSG_MAX_SIZE - MSG_MIN_SIZE) / MSG_SIZE_STEP + 1;
  int msg_sizes_arr[msg_sizes_arr_size];

  for (int i = 0; i < msg_sizes_arr_size; ++i) {
    msg_sizes_arr[i] = MSG_MIN_SIZE + i * MSG_SIZE_STEP;
  }

  if (g_rank == 0) printf("Message sizes array size %d\n", msg_sizes_arr_size);


  // optimistic allocation on stack
  // I'm using single buffer here, as we specify message size when calling send/recv
  // so whole array should not be copied
  char buffer[MSG_MAX_SIZE];
  const int cping = 0;
  const int cpong = 1;
  double start_time, end_time;

  // Just arbitrarily big buffer
  const int comm_buffer_size = MSG_MAX_SIZE * 4 + MPI_BSEND_OVERHEAD;
  char comm_buffer[comm_buffer_size];

  // buffered communication
  if (cfg.name[0] == 'b') {
    MPI_Buffer_attach(comm_buffer, comm_buffer_size);
  }

  for (int i = 0; i < msg_sizes_arr_size; ++i) {
    MPI_Barrier(MPI_COMM_WORLD);
    int msg_size = msg_sizes_arr[i];
    if (g_rank == 0) printf("Doing computation for msg_size: %d\n", msg_size);

    start_time = MPI_Wtime() * 1e6;
    for (int message_id = 0; message_id < ITERATION_COUNT; ++message_id) {
      if (g_rank == cping) {
        cfg.sendhandle(buffer, msg_size, MPI_BYTE, cpong, 0, MPI_COMM_WORLD);
        cfg.recvhandle(buffer, msg_size, MPI_BYTE, cpong, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else if (g_rank == cpong) {
        cfg.recvhandle(buffer, msg_size, MPI_BYTE, cping, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cfg.sendhandle(buffer, msg_size, MPI_BYTE, cping, 0, MPI_COMM_WORLD);
      } else {
        printf("Dunno what happened\n");
      }
    }
    end_time = MPI_Wtime() * 1e6;

    if (g_rank == cping) {
      double elapsed_time = end_time - start_time;

      double throughput = msg_size * 8.0 * ITERATION_COUNT * 2.0 / (elapsed_time);
      fprintf(logfile, "%s,%d,%lf,%lf\n", cfg.name, msg_size, elapsed_time, throughput);
    }
  }

  if (cfg.name[0] == 'b') {
    int size;
    MPI_Buffer_detach(comm_buffer, &size);
  }

  if (g_rank == 0) {
    fclose(logfile);
  }
}

void experiment_delay(ExperimentConfig cfg) {
  if (g_rank == 0) log_info(cfg.description);

  FILE *logfile;
  if (g_rank == 0) {
    logfile = fopen(cfg.logfilename, "w");
    if (logfile == NULL) {
      log_info("Failed to open logfile");
      return;
    }
  }


  const int cping = 0;
  const int cpong = 1;

  char payload = 'a';

  double start_time, end_time;

  // Just arbitrarily big buffer
  const int comm_buffer_size = 32 + MPI_BSEND_OVERHEAD;
  char comm_buffer[comm_buffer_size];

  // buffered communication
  if (cfg.name[0] == 'b') {
    MPI_Buffer_attach(comm_buffer, comm_buffer_size);
  }

  // Synchronizujemy wszystkie procesy tego komunkatora
  MPI_Barrier(MPI_COMM_WORLD);

  start_time = MPI_Wtime() * S_TO_MS_FACTOR;
  for (int i = 0; i < ITERATION_COUNT; ++i) {
    if (g_rank == cping) {
      cfg.sendhandle(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD);
      cfg.recvhandle(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (g_rank == cpong) {
      cfg.recvhandle(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      cfg.sendhandle(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD);
    } else {
      printf("Dunno what happened\n");
    }
  }
  end_time = MPI_Wtime() * S_TO_MS_FACTOR;

  if (g_rank == cping) {
    double elapsed_time = end_time - start_time;
    double single_send_time  = elapsed_time / (ITERATION_COUNT * 2);
    printf("Single send time: %lf [ms]\n", single_send_time );
    fprintf(logfile, "type,time\n");
    fprintf(logfile, "%s,%lf\n", cfg.name, single_send_time );
  }

  if (g_rank == 0) {
    fclose(logfile);
  }

  if (cfg.name[0] == 'b') {
    int size;
    MPI_Buffer_detach(comm_buffer, &size);
  }
}


int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);
  assert((init_global_state() == true) && "Global state initialized");
  assert((MSG_MIN_SIZE <= MSG_MAX_SIZE && MSG_SIZE_STEP > 0) && "Proper ranges");
  log_info("Initialized");

  ExperimentConfig throughput_cfg = {
    .name = "std",
    .description = "Throughput std",
    .sendhandle = MPI_Send,
    .recvhandle = MPI_Recv,
    .logfilename = "throughput-std.csv",
    .params = NULL
  };

  ExperimentConfig delay_cfg = {
    .name = "std",
    .description = "Delay std",
    .sendhandle = MPI_Send,
    .recvhandle = MPI_Recv,
    .logfilename = "delay-std.csv",
    .params = NULL
  };

  ExperimentConfig throughput_buf_cfg = {
    .name = "buff",
    .description = "Throughput buff",
    .sendhandle = MPI_Bsend,
    .recvhandle = MPI_Recv,
    .logfilename = "throughput-buff.csv",
    .params = NULL
  };

  ExperimentConfig delay_buf_cfg = {
    .name = "buff",
    .description = "Delay buff",
    .sendhandle = MPI_Bsend,
    .recvhandle = MPI_Recv,
    .logfilename = "delay-buff.csv",
    .params = NULL
  };


  MPI_Barrier(MPI_COMM_WORLD);
  experiment_throughput(throughput_cfg);

  MPI_Barrier(MPI_COMM_WORLD);
  experiment_throughput(throughput_buf_cfg);

  MPI_Barrier(MPI_COMM_WORLD);
  experiment_delay(delay_cfg);

  MPI_Barrier(MPI_COMM_WORLD);
  experiment_delay(delay_buf_cfg);

  // experiment_delay(MPI_Bsend, MPI_Recv, "Delay buff");

  teardown_global_state();
  MPI_Finalize();
  return 0;
}
```

## Makefile do automatyzacji + pliki maszynowe

```makefile
build-main: main.c
	mpicc -o main -std=gnu99 main.c -DITERATION_COUNT=3000 -DMSG_MIN_SIZE=1024 -DMSG_MAX_SIZE=102400 -DMSG_SIZE_STEP=1024

run-main: build-main
	mpiexec -machinefile ./allnodes -np 2 ./main

run-main-single-node: build-main
	mpiexec -machinefile ./single-node -np 2 ./main
	mv delay-std.csv delay-std-single-node.csv
	mv throughput-std.csv throughput-std-single-node.csv
	mv delay-buff.csv delay-buff-single-node.csv
	mv throughput-buff.csv throughput-buff-single-node.csv

run-main-two-nodes: build-main
	mpiexec -machinefile ./two-nodes -np 2 ./main
	mv delay-std.csv delay-std-two-nodes.csv
	mv throughput-std.csv throughput-std-two-nodes.csv
	mv delay-buff.csv delay-buff-two-nodes.csv
	mv throughput-buff.csv throughput-buff-two-nodes.csv

prepare-data:
	cat throughput-std-single-node.csv > throughput-single-node-all.csv
	cat throughput-buff-single-node.csv | tail -n +2 >> throughput-single-node-all.csv
	cat throughput-std-two-nodes.csv > throughput-two-nodes-all.csv
	cat throughput-buff-two-nodes.csv | tail -n +2 >> throughput-two-nodes-all.csv

	cat delay-std-single-node.csv > delay-single-node-all.csv
	cat delay-buff-single-node.csv | tail -n +2 >> delay-single-node-all.csv
	cat delay-std-two-nodes.csv > delay-two-nodes-all.csv
	cat delay-buff-two-nodes.csv | tail -n +2 >> delay-two-nodes-all.csv

	mkdir -p data
	mv *-all.csv data/
```

## Single node

```machinefile
vnode-04.dydaktyka.icsr.agh.edu.pl:2
```

## Two nodes

```machinefile
vnode-09.dydaktyka.icsr.agh.edu.pl:1
vnode-10.dydaktyka.icsr.agh.edu.pl:1
```

## Rysowanie wykresów

```python
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

if __name__ == "__main__":
    t_sn = pd.read_csv('./throughput-single-node-all.csv')
    t_tn = pd.read_csv('./throughput-two-nodes-all.csv')

    d_sn = pd.read_csv('./delay-single-node-all.csv')
    d_tn = pd.read_csv('./delay-two-nodes-all.csv')

    print(t_sn)
    print(t_tn)
    print(d_sn)
    print(d_tn)

    unit_factor = 1_000_000 / 2 ** 20
    t_sn_std_x = t_sn[t_sn['type'] == 'std']['throughput'] * unit_factor
    t_tn_std_x = t_tn[t_tn['type'] == 'std']['throughput'] * unit_factor
    t_sn_buff_x = t_sn[t_sn['type'] == 'buff']['throughput'] * unit_factor
    t_tn_buff_x = t_tn[t_tn['type'] == 'buff']['throughput'] * unit_factor
    t_y = t_tn[t_tn['type'] == 'std']['msgsize'] / 1024
    t_sn_y = t_sn[t_sn['type'] == 'std']['msgsize'] / 1024

    fig, ax = plt.subplots(figsize=(12, 7))
    ax.plot(t_sn_y, t_sn_std_x, label='std single node')
    ax.plot(t_sn_y, t_sn_buff_x, label='buff single node')

    ax.set_title('throughput(message_size)')
    ax.set_ylabel('Throughput [Mb / s]')
    ax.set_xlabel('Message size [KB]')
    ax.legend()

    fig, ax = plt.subplots(figsize=(12, 7))
    ax.plot(t_y, t_tn_std_x, label='std two nodes')
    ax.plot(t_y, t_tn_buff_x, label='buff two nodes')

    ax.set_title('throughput(message_size)')
    ax.set_ylabel('Throughput [Mb / s]')
    ax.set_xlabel('Message size [B]')
    ax.legend()

    plt.show()
```

# Badanie efektywności programu równoległego -- kod źródłowy

# Eksperyment

```c
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

typedef uintmax_t Size_t;

typedef struct ProcessArgs {
  Size_t total_point_count;
  Size_t point_count;
} ProcessArgs;

char g_hostname[MPI_MAX_PROCESSOR_NAME];
int g_rank, g_size, g_hostname_len;
ProcessArgs g_pargs;

inline bool init_global_state(void) {
  assert((MPI_Comm_rank(MPI_COMM_WORLD, &g_rank) == MPI_SUCCESS) && "Valid rank returned");
  assert((MPI_Comm_size(MPI_COMM_WORLD, &g_size) == MPI_SUCCESS) && "Valid size returned");
  assert((MPI_Get_processor_name(g_hostname, &g_hostname_len) == MPI_SUCCESS) && "Valid processor name");
  return true;
}

inline bool teardown_global_state(void) {
  return true;
}

double estimate_pi(Size_t point_count) {
  double x, y;
  Size_t hit_count = 0;

  for (Size_t i = 0; i < point_count; ++i) {
    x = drand48();
    y = drand48();
    if (x * x + y * y <= 1) ++hit_count;
  }

  return (double)hit_count / point_count * 4;
}

bool parse_args(int argc, char *argv[], ProcessArgs *output) {
  if (argc != 2) {
    output->point_count = 1e5;
    return false;
  }
  Size_t total_point_count = strtoll(argv[1], NULL, 10);
  assert((total_point_count > 0) && "Point count must be > 0");

  output->total_point_count = total_point_count;
  output->point_count = total_point_count / g_size;

  return true;
}

int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);
  init_global_state();
  srand48(time(NULL) + g_rank * 31);

  parse_args(argc, argv, &g_pargs);

  double start_time, elapsed_time;
  double reduce_buffer;

  MPI_Barrier(MPI_COMM_WORLD);

  if (g_rank == 0) {
    start_time = MPI_Wtime() * 1e6;
  }

  parse_args(argc, argv, &g_pargs);

  double pi_estimate = estimate_pi(g_pargs.point_count);

  MPI_Reduce(&pi_estimate, &reduce_buffer, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (g_rank == 0) {
    double average = reduce_buffer / g_size;
    elapsed_time = MPI_Wtime() * 1e6 - start_time;
    printf("proc_count,total_point_count,point_count,avg_pi,time\n");
    printf("%d,%ld,%ld,%lf,%lf\n", g_size, g_pargs.total_point_count,g_pargs.point_count, average, elapsed_time);
  }

  teardown_global_state();
  MPI_Finalize();
  return 0;
}
```

## Skrypt automatyzujący (uruchamianie programu i agregowanie danych)

```bash
#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=10:00:00
#SBATCH --partition=plgrid
#SBATCH --account=plgmpr23-cpu

rootdir="$(pwd)"
echo "Running run.sh script from dir: ${rootdir}"

print_help ()
{
  echo -e "Available params:\n\t-h -- show this help\n\t-b BINARY -- specify program to run (defaults to 'main')\n\t-c/-C -- whether to compile (uses make)\n\t-d/-D -- whether to process data\n\t-r/-R -- whether to run\n\t-a/-A -- run all stages / none; specify it always first\n\t-m MACHINEFILE\n\t-x/-X -- ares / not ares execution context\n\t-t -- run with test params\n\t-s/-S -- scale weak / strong (only for Ares)\n\t-z -- archive final data"
}

# $1 -- binary name
assert_binary_exists ()
{
  if ! command -v "$1" &> /dev/null
  then
    echo "Look like $1 binary is missing. Aborting"
    exit 1
  fi
}

run_vc_strong ()
{
  # Constant problem size -- splitted over various numbers of processes
  local problem_size=${vc_strong_point_count}
  local exp_type="strong"

  for (( series_id = 1 ; series_id <= ${vc_repeats} ; series_id++ ))
  do
    for n_processes in ${proc_count}
    do
      echo "[${execution_context}] Point count: ${problem_size}, process count: ${n_processes}"
      mpiexec -machinefile "./${machinefilename}" -np ${n_processes} "./${progname}" "${problem_size}" | tee "${outdir_raw}/type_${exp_type}_series_${series_id}_points_${problem_size}_procs_${n_processes}.csv"
    done
  done
}

run_vc_weak ()
{
  local base_problem_size=${vc_weak_point_count_base}
  local exp_type="weak"
  local problem_size=${base_problem_size}

  for (( series_id = 1 ; series_id <= ${vc_repeats} ; series_id++ ))
  do
    for n_processes in ${proc_count}
    do
      problem_size=$(( ${n_processes} * ${base_problem_size} ))
      echo "[${execution_context}] Point count: ${problem_size}, process count: ${n_processes}"
      mpiexec -machinefile "./${machinefilename}" -np ${n_processes} "./${progname}" "${problem_size}" | tee "${outdir_raw}/type_${exp_type}_series_${series_id}_points_${problem_size}_procs_${n_processes}.csv"
    done
  done
}

run_ares_strong ()
{
  for series_id in ${ares_series}
  do
    for problem_size in "${ares_point_counts[@]}"
    do
      for n_procs in ${proc_count}
      do
        echo "[ares] scaling: strong, sid: ${series_id}, points: ${problem_size}, procs: ${n_procs}"
        mpiexec -np ${n_procs} "./main" "${problem_size}" | tee "${outdir_raw}/type_strong_series_${series_id}_points_${problem_size}_procs_${n_procs}.csv"
      done
    done
  done
}

run_ares_weak ()
{
  local total_problem_size=0
  for series_id in ${ares_series}
  do
    for problem_size in "${ares_point_counts[@]}"
    do
      for n_procs in ${proc_count}
      do
        total_problem_size=$(( ${problem_size} * ${n_procs} ))
        echo "[ares] scaling: weak, sid: ${series_id}, points: ${total_problem_size}, procs: ${n_procs}"
        mpiexec -np ${n_procs} "./main" "${total_problem_size}" | tee "${outdir_raw}/type_weak_series_${series_id}_points_${total_problem_size}_procs_${n_procs}.csv"
      done
    done
  done
}

# possible params
progname="main"
machinefilename="allnodes"
scaling="all"

# Shared configuration
proc_count=$(seq 1 1 12)

# vcluster configurations
vc_repeats=3
vc_series=""

# weak scaling configuration
vc_weak_point_count_base=100000000 # 1e8

# strong scaling configuration
vc_strong_point_count=100000000 # 1e8

# Ares configurations
ares_repeats=10
ares_series=""
ares_point_counts=( 1000 1000000 1000000000 ) # 1e3, 1e6, 1e9

# Actions to execute
should_process_data=1 # 1 means 'yes'
should_compile=1
should_run=1
should_archive=1
is_test=0

# Execution context
username="$(whoami)"

if [[ "${username}" != "kafara" ]]
then
  is_ares=1
  execution_context="ares"
else
  is_ares=0
  execution_context="vcluster"
fi


# https://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
OPTIND=1
opt_str="haAb:dDm:cCrRxXts:S:zZ"

while getopts "${opt_str}" opt
do
  case "${opt}" in
    h)
      print_help
      exit 0
      ;;
    a)
      should_compile=1
      should_run=1
      should_process_data=1
      should_archive=1
      ;;
    A)
      should_compile=0
      should_run=0
      should_process_data=0
      should_archive=0
      ;;
    b) progname="${OPTARG}"
      ;;
    d) should_process_data=1
      ;;
    D) should_process_data=0
      ;;
    m) machinefilename="${OPTARG}"
      ;;
    c) should_compile=1
      ;;
    C) should_compile=0
      ;;
    r) should_run=1
      ;;
    R) should_run=0
      ;;
    x) is_ares=1
      ;;
    X) is_ares=0
      ;;
    t)
      is_test=1
      ;;
    z)
      should_archive=1
      ;;
    Z)
      should_archive=0
      ;;
    s)
      scaling="weak"
      ares_series="${OPTARG}"
	  vc_series="${OPTARG}"
      ;;
    S)
      scaling="strong"
      ares_series="${OPTARG}"
	  vc_series="${OPTARG}"
      ;;
  esac
done

shift $((OPTIND-1))

if [[ ${should_compile} -eq 1 ]]
then
  echo "Compiling..."
  assert_binary_exists "make"
  make
fi

echo "Creating data directories"
mkdir -p data/{test,full}/{ares,vcluster}/{raw,processed}

if [[ ${is_test} -eq 1 ]]
then
  modedir="test"
else
  modedir="full"
fi

outdir="data/${modedir}/${execution_context}"
outdir_raw="${outdir}/raw"
outdir_processed="${outdir}/processed"

echo "Resolved outdir: ${outdir}"

if [[ ${should_run} -eq 1 ]]
then
  echo "Running..."
  assert_binary_exists "mpiexec"
  assert_binary_exists "tee"

  if [[ ${is_ares} -eq 0 && ! -z "${MACHINEFILE}" ]]
  then
    machinefilename="${MACHINEFILE}"
    echo "Running with machinefile: ${machinefilename}"
  fi

  # Main loop dispatch
  if [[ ${is_ares} -eq 1 ]]
  then
    if [[ ${scaling} == "weak" ]]
    then
      run_ares_weak
    else
      run_ares_strong
    fi
  else
    run_vc_strong
    run_vc_weak
  fi
fi

if [[ ${should_process_data} -eq 1 ]]
then
  echo "Processing raw data..."
  assert_binary_exists "xargs"
  assert_binary_exists "awk"

  cd "${outdir_raw}"

  outfile="final.csv"
  echo "type,series,proc_count,total_point_count,point_count,avg_pi,time" > "../processed/${outfile}"
  series_count=${ares_repeats}

  if [[ ${is_ares} -eq 0 ]]
  then
    series_count=${vc_repeats}
  fi

  for exptype in "strong" "weak"
  do
    echo "Processing for experiment type: ${exptype}"
    for (( series_id = 1 ; series_id <= ${series_count} ; series_id++ ))
    do
      ls . | grep "^type_${exptype}_series_${series_id}_" | xargs -n 1 tail -n 1 | awk -v type="${exptype}" -v sid="${series_id}" -F ',' '/.+/ {print type "," sid "," $0}' >> "../processed/${outfile}"
    done
  done

  cd "${rootdir}"
fi


if [[ ${should_archive} -eq 1 ]]
then
  cd "${rootdir}"
  echo "Archiving final data..."
  timestamp=$(date +%Y-%m-%d-%H:%M:%S)
  archivedir="data-archive/${execution_context}/${timestamp}"
  mkdir -p "${archivedir}"
  cp "${outdir_processed}/final.csv" "${archivedir}/"
fi

exit 0
```

## Rysowanie wykresów

```python
import pathlib as path
import polars as pl
import matplotlib.pyplot as plt
import numpy as np

pl.Config.set_tbl_rows(100)
plt.rcParams['errorbar.capsize'] = 5
plt.rcParams['figure.figsize'] = (16, 9)
largemarkersize=100
smallmarkersize=50
primarymarkerstyle='o'
secondarymarkerstyle='^'

plotdir = path.Path('data', 'plots')
assert plotdir.is_dir(), "[ares] Plotdir exists"

# exctx = "ares"
exctx = "vcluster"

if exctx == 'ares':
    datadir = path.Path('data', 'ares')
    datafile = datadir.joinpath('ares-final-23-03-21-06-43.csv')
else:
    datadir = path.Path('data', 'vcluster')
    datafile = datadir.joinpath('vcluster-final-23-03-21-09-29.csv')


# vCluster
assert datadir.is_dir(), f"[{exctx}] Datadir exists"
assert datafile.is_file(), f"[{exctx}] main data file exists"


data_raw = pl.read_csv(datafile, has_header=True)
data_raw = (
    data_raw.replace('time', data_raw.get_column('time') / 1e6) # conversion from microseconds
    .drop('avg_pi') # don't need this column, as long as it looks ok -- it does!
    .rename({"proc_count": "procs", "total_point_count": "tpts", "point_count": "pts"})
)

data_s, data_w = data_raw.partition_by('type')

# Aggregated by series id
dtime_s = (
    data_s
    .groupby(['procs', 'tpts']) # key is series x procs x tpts
    .agg([pl.mean('time'), pl.std('time').alias('time_std')])
    .sort(pl.col(['procs', 'tpts']))
)

dtime_w = (
    data_w
    .groupby(['procs', 'pts'])
    .agg([pl.mean('time'), pl.std('time').alias('time_std')])
    .sort(pl.col(['procs', 'pts']))
)

point_counts = dtime_w.get_column('pts').unique().sort()
series = data_w.get_column('series').unique().sort()
xdata_time = dtime_w.get_column('procs').unique().sort()
xdata = xdata_time.clone().tail(-1)
xdata_bounds = xdata.head(1), xdata.tail(1)

dspeedup_s = (
    data_s
    .groupby(['procs', 'tpts'])
    .agg([pl.mean('time')])
    .with_columns((1 / pl.col('time')).alias('speedup'))
    .sort(pl.col(['procs', 'tpts']))
)

t1s = dspeedup_s.filter(pl.col('procs') == 1).get_column('time').sort()

for i, points in enumerate(point_counts):
    fig, [[plot_time, plot_sd], [plot_eff, plot_sf]] = plt.subplots(nrows=2, ncols=2)

    data_time = dtime_s.filter(pl.col('tpts') == points)
    plot_time.errorbar(xdata_time, data_time.get_column('time'), data_time.get_column('time_std'), linestyle=':')
    plot_time.scatter(xdata_time, data_time.get_column('time'), label='Średnia')
    plot_time.plot(xdata_time, data_time.get_column('time').max() / xdata_time, linestyle='--', label='$y=t_1 / x$')

    data_sd = dspeedup_s.filter(pl.col('tpts') == points).get_column('speedup').tail(-1) * t1s[i]
    plot_sd.scatter(xdata, data_sd, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sd.plot(xdata, data_sd, linestyle=':')
    plot_sd.plot(xdata_bounds, xdata_bounds, linestyle='--', label="y=x")

    # Serial Fraction
    data_sf = ((1 / data_sd) - (1 / xdata)) / (1 - 1 / xdata)
    plot_sf.scatter(xdata, data_sf, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sf.plot(xdata, data_sf, linestyle=':')
    plot_sf.plot([xdata.head(1), xdata.tail(1)], [0, 0], linestyle='--', label="y=0")

    # Efficiency
    data_eff = data_sd / xdata
    plot_eff.scatter(xdata, data_eff, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_eff.plot(xdata, data_eff, linestyle=':')
    plot_eff.plot(xdata_bounds, [1, 1], linestyle='--', label='y=x')

    for sid in series:
        data_sd = (
            data_s
            .filter((pl.col('series') == sid) & (pl.col('tpts') == points))
            .sort(pl.col('procs'))
        )
        ydata = data_sd.get_column('time')[0] / data_sd.get_column('time').tail(-1)
        plot_sd.scatter(xdata, ydata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_eff.scatter(xdata, ydata / xdata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_sf.scatter(xdata, ((1 / ydata) - (1 / xdata)) / (1 - 1 / xdata), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')

        data_time = data_sd
        plot_time.scatter(xdata_time, data_time.get_column('time'), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')

    plot_sd.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie sline)", xlabel="Liczba procesów", ylabel="Przyśpieszenie")
    plot_sd.grid()
    plot_sd.legend()

    plot_eff.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Efektywność")
    plot_eff.grid()
    plot_eff.legend()

    plot_sf.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Serial Fraction")
    plot_sf.grid()
    plot_sf.legend()

    plot_time.set(title=f"{exctx}, {points:.0e} punktów do podziału (skalowanie silne)", xlabel="Liczba procesów", ylabel="Czas [s]")
    plot_time.grid()
    plot_time.legend()

    fig.tight_layout()
    fig.savefig(plotdir.joinpath(f'{exctx}-combined-s-{points:.0e}.png'))


dspeedup_w = (
    data_w
    .groupby(['procs', 'pts'])
    .agg([pl.mean('time')])
    .with_columns((1 / pl.col('time') * pl.col('procs')).alias('speedup')) # ACHTUNG - scaling
    .sort(pl.col(['procs', 'pts']))
)

t1s = dspeedup_w.filter(pl.col('procs') == 1).get_column('time').sort()
for i, points in enumerate(point_counts):
    fig, [[plot_time, plot_sd], [plot_eff, plot_sf]] = plt.subplots(nrows=2, ncols=2)

    data_time = dtime_w.filter(pl.col('pts') == points)
    plot_time.errorbar(xdata_time, data_time.get_column('time'), data_time.get_column('time_std'), linestyle=':')
    plot_time.scatter(xdata_time, data_time.get_column('time'), label='Średnia')
    plot_time.plot(xdata_time, [data_time.get_column('time').max() for _ in range(len(xdata_time))], linestyle='--', label='$y=t_1$')

    data_sd = dspeedup_w.filter(pl.col('pts') == points).get_column('speedup').tail(-1) * t1s[i]
    plot_sd.scatter(xdata, data_sd, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sd.plot(xdata, data_sd, linestyle=':')
    plot_sd.plot([2, 12], [2, 12], linestyle='--', label="y=x")

    # Serial Fraction
    data_sf = ((1 / data_sd) - (1 / xdata)) / (1 - 1 / xdata)
    plot_sf.scatter(xdata, data_sf, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_sf.plot(xdata, data_sf, linestyle=':')
    plot_sf.plot([xdata.head(1), xdata.tail(1)], [0, 0], linestyle='--', label="y=0")

    # Efficiency
    data_eff = data_sd / xdata
    plot_eff.scatter(xdata, data_eff, s=largemarkersize, marker=primarymarkerstyle, label='Średnia')
    plot_eff.plot(xdata, data_eff, markersize=smallmarkersize, linestyle=':')
    plot_eff.plot([2, 12], [1, 1], linestyle='--', label='y=x')

    for sid in series:
        data_sd = (
            data_w
            .filter((pl.col('series') == sid) & (pl.col('pts') == points))
            .sort(pl.col('procs'))
        )
        ydata = (data_sd.get_column('time')[0] * data_sd.get_column('procs') / data_sd.get_column('time')).tail(-1)
        plot_sd.scatter(xdata, ydata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_eff.scatter(xdata, ydata / xdata, s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')
        plot_sf.scatter(xdata, ((1 / ydata) - (1 / xdata)) / (1 - 1 / xdata), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria {sid}')
        plot_time.scatter(xdata_time, data_sd.get_column('time'), s=smallmarkersize, marker=secondarymarkerstyle, label=f'Seria: {sid}')

    plot_sd.set(title=f'{exctx}, {points:.0e} punktów na proces (skalowanie słabe)', xlabel="Liczba procesów", ylabel="Przyśpieszenie")
    plot_sd.grid()
    plot_sd.legend()

    plot_eff.set(title=f"{exctx}, {points:.0e} punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Efektywność")
    plot_eff.grid()
    plot_eff.legend()

    plot_sf.set(title=f"{exctx}, {points:.0e} punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Serial Fraction")
    plot_sf.grid()
    plot_sf.legend()
    plot_time.set(title=f"{exctx}, {points:.0e} punktów na proces (skalowanie słabe)", xlabel="Liczba procesów", ylabel="Czas [s]")
    plot_time.grid()
    plot_time.legend()

    fig.tight_layout()
    fig.savefig(plotdir.joinpath(f'{exctx}-combined-w-{points:.0e}.png'))

plt.show()
plt.close()
```

# Dane - PP

## Przepustowość -- single node

```csv
type,msgsize,time,throughput
std,1024,393.000000,12506.870229
std,2048,484.000000,20310.743802
std,3072,628.500000,23461.575179
std,4096,766.500000,25650.097847
std,5120,837.000000,29362.007168
std,6144,936.250000,31499.279039
std,7168,1111.750000,30947.964920
std,8192,1215.500000,32350.143974
std,9216,1296.250000,34126.750241
std,10240,1392.000000,35310.344828
std,11264,1541.500000,35074.408044
std,12288,1653.750000,35665.850340
std,13312,1708.000000,37410.772834
std,14336,1821.250000,37783.280714
std,15360,1984.000000,37161.290323
std,16384,2221.500000,35400.945307
std,17408,2396.000000,34874.123539
std,18432,2561.500000,34539.761858
std,19456,2661.750000,35085.488870
std,20480,2735.500000,35936.391884
std,21504,2784.750000,37065.876650
std,22528,2976.250000,36332.431751
std,23552,3060.000000,36944.313725
std,24576,3167.250000,37245.181151
std,25600,3282.250000,37437.733262
std,26624,3432.750000,37228.228097
std,27648,3528.750000,37608.331562
std,28672,3672.250000,37477.187011
std,29696,3760.250000,37907.266804
std,30720,3798.500000,38819.534027
std,31744,4053.500000,37590.033305
std,32768,3935.500000,39966.052598
std,33792,4085.750000,39699.345285
std,34816,4199.750000,39792.082862
std,35840,4364.250000,39418.456780
std,36864,4506.000000,39269.241012
std,37888,4562.500000,39860.252055
std,38912,4706.500000,39685.031340
std,39936,4880.750000,39275.275316
std,40960,4992.500000,39380.671007
std,41984,5075.500000,39705.093094
std,43008,5191.750000,39762.777484
std,44032,5247.000000,40280.846198
std,45056,5450.250000,39680.528416
std,46080,5576.750000,39661.810194
std,47104,5681.000000,39799.190283
std,48128,5762.250000,40091.006117
std,49152,5863.500000,40236.991558
std,50176,6010.000000,40074.009983
std,51200,6195.500000,39667.500605
std,52224,6259.750000,40045.560925
std,53248,6446.000000,39651.008377
std,54272,6702.000000,38869.829902
std,55296,6844.750000,38777.281858
std,56320,7028.500000,38462.829907
std,57344,7189.750000,38283.834626
std,58368,7312.500000,38313.353846
std,59392,7441.250000,38310.982698
std,60416,7600.000000,38157.473684
std,61440,7649.750000,38551.848100
std,62464,7606.000000,39419.826453
std,63488,7634.750000,39915.177314
std,64512,7698.250000,40224.414640
std,65536,7463.250000,42149.572907
std,66560,6641.750000,48102.984906
std,67584,6811.250000,47627.557350
std,68608,6890.250000,47794.840536
std,69632,6960.250000,48020.344097
std,70656,6946.250000,48824.732769
std,71680,7180.500000,47916.440359
std,72704,7116.250000,49039.761110
std,73728,7217.500000,49032.823000
std,74752,7271.750000,49342.950459
std,75776,7370.500000,49348.728037
std,76800,7453.250000,49460.302553
std,77824,7546.750000,49498.817372
std,78848,7602.500000,49782.361065
std,79872,7730.250000,49595.498205
std,80896,7751.500000,50093.633490
std,81920,7838.250000,50166.299876
std,82944,7958.250000,50027.480916
std,83968,8188.000000,49224.035173
std,84992,8111.250000,50295.774387
std,86016,8228.750000,50174.911135
std,87040,8303.500000,50315.168303
std,88064,8481.250000,49840.200442
std,89088,8489.750000,50369.257045
std,90112,8603.750000,50273.148336
std,91136,8664.000000,50490.858726
std,92160,8853.000000,49968.146391
std,93184,8874.750000,50399.526747
std,94208,8978.250000,50365.984462
std,95232,8991.500000,50838.414058
std,96256,9118.250000,50670.775642
std,97280,9160.250000,50975.027974
std,98304,9304.000000,50715.735168
std,99328,9431.500000,50551.280284
std,100352,9558.500000,50393.848407
std,101376,9585.000000,50767.323944
std,102400,9548.500000,51476.148086
std,103424,9784.750000,50735.603873
std,104448,9855.000000,50872.694064
std,105472,9982.250000,50716.581933
std,106496,9943.250000,51409.830790
std,107520,10082.500000,51187.304736
std,108544,10184.000000,51159.780047
std,109568,10329.250000,50916.223346
std,110592,10346.000000,51308.872994
std,111616,10419.250000,51419.900665
std,112640,10490.750000,51537.973929
std,113664,10563.750000,51647.113951
std,114688,10610.750000,51881.572933
std,115712,10802.250000,51416.843713
std,116736,10850.500000,51641.196258
std,117760,10914.750000,51787.535216
std,118784,10985.000000,51903.796086
std,119808,11080.250000,51901.211615
std,120832,11123.000000,52143.630316
std,121856,11277.500000,51865.111949
std,122880,11475.250000,51399.664495
std,123904,11452.000000,51933.216905
std,124928,11529.000000,52012.698413
std,125952,11634.750000,51962.405724
std,126976,11719.500000,52006.041213
std,128000,11865.000000,51782.553729
std,129024,11960.000000,51782.207358
std,130048,12158.500000,51341.070033
std,131072,12127.500000,51877.600495
std,132096,12903.000000,49140.571960
std,133120,12764.250000,50059.815500
std,134144,12735.500000,50558.768796
std,135168,12781.500000,50761.366037
std,136192,12884.500000,50737.056153
std,137216,12952.000000,50852.130945
std,138240,13001.500000,51036.572703
std,139264,13308.750000,50227.647225
std,140288,13298.750000,50635.014569
std,141312,13410.750000,50578.647727
std,142336,13394.750000,51006.013550
std,143360,13647.000000,50423.389756
std,144384,13824.250000,50132.426714
std,145408,13724.750000,50853.997341
std,146432,13846.000000,50763.657374
std,147456,13799.500000,51290.901844
std,148480,13961.750000,51046.895984
std,149504,14064.500000,51023.441999
std,150528,14016.000000,51550.684932
std,151552,14529.250000,50067.938813
std,152576,14201.750000,51568.630627
std,153600,14384.000000,51256.952169
std,154624,14373.000000,51638.154874
std,155648,14653.250000,50985.986044
std,156672,14594.500000,51528.013978
std,157696,14777.750000,51221.654176
std,158720,14884.500000,51184.520810
std,159744,14954.250000,51274.467125
std,160768,14987.250000,51489.526097
std,161792,15123.500000,51350.652957
std,162816,15262.000000,51206.709475
std,163840,15332.250000,51292.667417
std,164864,15427.500000,51294.584346
std,165888,15491.500000,51399.954814
std,166912,15574.500000,51441.625734
std,167936,15836.250000,50901.747573
std,168960,15916.250000,50954.716092
std,169984,16145.750000,50534.859019
std,171008,16132.250000,50881.829875
std,172032,16334.250000,50553.505671
std,173056,16314.000000,50917.543214
std,174080,16595.750000,50349.276170
std,175104,16710.500000,50297.669130
std,176128,16644.000000,50793.943764
std,177152,16632.250000,51125.349847
std,178176,16743.750000,51078.450168
std,179200,16945.000000,50761.876660
std,180224,16832.000000,51394.676806
std,181248,17056.500000,51006.384663
std,182272,17175.750000,50938.421903
std,183296,17233.500000,51052.937592
std,184320,17218.250000,51383.619125
std,185344,17280.750000,51482.209974
std,186368,17416.000000,51364.630225
std,187392,17486.750000,51437.894406
std,188416,17514.500000,51637.032173
std,189440,17688.750000,51406.232775
std,190464,17690.750000,51678.261238
std,191488,17826.250000,51561.175233
std,192512,17810.250000,51883.471596
std,193536,18080.000000,51381.238938
std,194560,18057.500000,51717.458120
std,195584,18402.750000,51014.288625
std,196608,18497.500000,51018.699824
std,197632,18598.250000,51006.605460
std,198656,18709.750000,50965.341600
std,199680,18919.250000,50660.782008
std,200704,18992.500000,50724.191128
std,201728,18988.500000,50993.727783
std,202752,19131.000000,50870.816999
std,203776,19398.500000,50422.702786
std,204800,19232.500000,51113.479787
std,205824,19498.750000,50667.617155
std,206848,19422.000000,51120.914427
std,207872,19695.250000,50661.230500
std,208896,19711.500000,50868.822768
std,209920,19747.750000,51024.344546
std,210944,20159.750000,50225.384739
std,211968,19992.750000,50890.767903
std,212992,20008.250000,51097.002486
std,214016,20141.000000,51004.259967
std,215040,20262.250000,50941.627904
std,216064,20216.750000,51299.402723
std,217088,20626.500000,50518.624100
std,218112,20601.750000,50817.896538
std,219136,20884.250000,50365.840286
std,220160,20689.500000,51077.503081
std,221184,20934.500000,50714.523872
std,222208,21001.000000,50787.981525
std,223232,21113.250000,50750.765515
std,224256,20865.500000,51588.929093
std,225280,21026.750000,51427.063146
std,226304,21034.750000,51641.174723
std,227328,21226.500000,51406.232775
std,228352,21272.000000,51527.341106
std,229376,21561.250000,51064.052409
std,230400,21588.000000,51228.460256
std,231424,21922.500000,50671.009237
std,232448,21774.000000,51242.325710
std,233472,21965.250000,51019.933759
std,234496,22194.500000,50714.402217
std,235520,22072.750000,51216.817116
std,236544,22123.000000,51322.659675
std,237568,22173.250000,51428.022505
std,238592,22389.000000,51151.976417
std,239616,22406.500000,51331.390445
std,240640,22412.250000,51537.529699
std,241664,22728.750000,51036.119452
std,242688,22729.500000,51250.683033
std,243712,22705.750000,51520.764564
std,244736,23156.000000,50731.248920
std,245760,23018.750000,51247.265816
std,246784,23220.750000,51013.132651
std,247808,22965.750000,51793.579570
std,248832,23032.000000,51858.006252
std,249856,23087.500000,51946.239307
std,250880,23329.500000,51618.080113
std,251904,23359.750000,51761.649846
std,252928,23365.500000,51959.273288
std,253952,23475.250000,51925.734550
std,254976,23591.500000,51878.210372
std,256000,23687.500000,51875.461741
std,257024,23830.000000,51771.514897
std,258048,23895.750000,51834.757227
std,259072,23981.500000,51854.371078
std,260096,24219.000000,51548.817044
std,261120,24249.750000,51686.141094
std,262144,24698.250000,50946.573138
std,263168,24644.500000,51257.132423
std,264192,24710.250000,51319.658846
std,265216,24862.750000,51202.574132
std,266240,25043.000000,51030.307870
std,267264,24916.750000,51486.136836
std,268288,24898.750000,51720.765099
std,269312,25005.500000,51696.530763
std,270336,25253.750000,51382.974806
std,271360,25123.750000,51844.489776
std,272384,25306.000000,51665.344187
std,273408,25234.500000,52006.514890
std,274432,25566.250000,51523.927052
std,275456,25483.000000,51885.131264
std,276480,25689.000000,51660.399393
std,277504,25767.500000,51693.769283
std,278528,25598.250000,52227.570244
std,279552,25652.500000,52308.726245
std,280576,25718.500000,52365.604526
std,281600,25814.250000,52361.776926
std,282624,26217.000000,51744.867834
std,283648,25993.000000,52379.886893
std,284672,26060.000000,52433.829624
std,285696,26482.500000,51782.905692
std,286720,26108.750000,52712.443146
std,287744,26292.750000,52530.496049
std,288768,26198.000000,52908.099855
std,289792,26279.500000,52931.052722
std,290816,26211.750000,53255.383559
std,291840,26371.500000,53119.162732
std,292864,26350.500000,53348.027552
std,293888,26756.250000,52722.724597
std,294912,26720.250000,52977.707918
std,295936,27001.250000,52608.408870
std,296960,27076.750000,52643.245589
std,297984,27137.500000,52706.520497
std,299008,27290.000000,52592.099670
std,300032,27458.750000,52447.893659
std,301056,27446.000000,52651.344458
std,302080,27438.250000,52845.352747
std,303104,27743.500000,52441.083497
std,304128,27830.500000,52453.761161
std,305152,28013.250000,52287.028460
std,306176,28071.250000,52354.091820
std,307200,28241.500000,52212.524122
std,308224,28257.500000,52356.903477
std,309248,28296.500000,52458.445391
std,310272,28306.750000,52613.090517
std,311296,28399.750000,52613.871601
std,312320,28355.750000,52868.853760
std,313344,28775.000000,52269.372719
std,314368,28873.000000,52262.196516
std,315392,28631.000000,52875.610352
std,316416,28743.250000,52840.120724
std,317440,28657.000000,53170.673832
std,318464,28629.500000,53393.429854
std,319488,28742.250000,53354.987866
std,320512,28955.500000,53131.791888
std,321536,28917.250000,53372.046097
std,322560,29255.000000,52923.876260
std,323584,29243.250000,53113.221000
std,324608,29433.500000,52936.905227
std,325632,29334.500000,53283.117149
std,326656,29522.500000,53110.298925
std,327680,29624.500000,53093.351787
std,328704,29805.500000,52935.840701
std,329728,29911.750000,52912.129849
std,330752,30098.750000,52746.695461
std,331776,30157.500000,52806.923651
std,332800,30098.250000,53074.182054
std,333824,31448.750000,50951.316030
std,334848,33409.500000,48108.184798
std,335872,30493.250000,52870.245054
std,336896,30620.500000,52811.051420
std,337920,30469.500000,53234.086546
std,338944,30651.000000,53079.220906
std,339968,30893.500000,52821.674462
std,340992,30786.000000,53165.776652
std,342016,30943.750000,53053.582711
std,343040,31088.250000,52965.091313
std,344064,30816.250000,53592.088590
std,345088,30819.500000,53745.920602
std,346112,31034.250000,53532.390826
std,347136,31176.000000,53446.651270
std,348160,31190.750000,53578.961711
std,349184,31227.750000,53672.877489
std,350208,31210.750000,53859.596453
std,351232,31419.250000,53658.620113
std,352256,31385.500000,53872.928582
std,353280,31407.000000,53992.549432
std,354304,31492.000000,54002.895974
std,355328,31843.750000,53560.726987
std,356352,31788.000000,53809.286523
std,357376,31778.500000,53980.043111
std,358400,32086.500000,53615.071759
std,359424,32100.250000,53745.226283
std,360448,32208.750000,53716.781930
std,361472,32384.000000,53577.865613
std,362496,32461.250000,53601.780585
std,363520,32643.750000,53452.682366
std,364544,32699.250000,53512.273217
std,365568,32647.250000,53748.061475
std,366592,32918.000000,53455.301051
std,367616,32944.000000,53562.311802
std,368640,33095.750000,53465.233451
std,369664,33009.000000,53754.648732
std,370688,33390.750000,53287.284652
std,371712,33539.000000,53198.294523
std,372736,33374.500000,53607.778394
std,373760,33575.250000,53433.645319
std,374784,33634.000000,53486.448237
std,375808,33625.500000,53646.143552
std,376832,33349.500000,54237.502811
std,377856,33546.250000,54065.917949
std,378880,33578.000000,54161.176961
std,379904,33652.000000,54188.137406
std,380928,33772.750000,54139.932342
std,381952,33933.500000,54028.308309
std,382976,34057.000000,53976.709634
std,384000,34136.500000,53994.990699
std,385024,34201.500000,54036.086137
std,386048,34402.000000,53864.031161
std,387072,34353.500000,54083.153099
std,388096,34426.000000,54112.031604
std,389120,34743.000000,53759.778948
std,390144,34676.500000,54004.619843
std,391168,34602.250000,54262.552291
std,392192,34782.500000,54122.665133
std,393216,34805.500000,54228.119119
std,394240,35097.000000,53917.770750
std,395264,35118.250000,54025.106604
std,396288,35150.000000,54116.142248
std,397312,35363.000000,53929.180217
std,398336,35425.500000,53972.782318
std,399360,35467.750000,54047.070931
std,400384,35493.750000,54145.960909
std,401408,35713.250000,53950.799773
std,402432,35611.000000,54243.733678
std,403456,35726.250000,54206.327280
std,404480,35893.250000,54091.061690
std,405504,35941.500000,54155.202204
std,406528,35957.500000,54267.799486
std,407552,36149.000000,54116.285374
std,408576,36331.500000,53979.736592
std,409600,36203.500000,54306.351596
std,410624,36289.750000,54312.724667
std,411648,36395.250000,54290.337338
std,412672,36535.750000,54216.092457
std,413696,36634.250000,54204.488969
std,414720,36821.750000,54061.960662
std,415744,36774.500000,54265.080423
std,416768,36892.250000,54225.112320
std,417792,36766.500000,54544.261760
std,418816,36768.250000,54675.346257
std,419840,36864.000000,54666.666667
std,420864,37162.250000,54360.196167
std,421888,37177.250000,54470.473206
std,422912,37460.750000,54189.454295
std,423936,37437.750000,54354.035699
std,424960,37546.250000,54327.875620
std,425984,37574.250000,54418.203956
std,427008,37693.250000,54376.802213
std,428032,37800.000000,54353.269841
std,429056,37807.750000,54472.133359
std,430080,37970.500000,54368.101553
std,431104,38042.250000,54394.763717
std,432128,38193.000000,54308.758149
std,433152,37984.500000,54736.263476
std,434176,38230.500000,54512.622121
std,435200,38235.750000,54633.687060
std,436224,38952.000000,53755.268022
std,437248,38729.000000,54191.701309
std,438272,38888.500000,54095.827815
std,439296,38995.250000,54073.785910
std,440320,39035.500000,54143.945895
std,441344,39153.250000,54106.650150
std,442368,39066.000000,54353.309783
std,443392,39217.750000,54268.324929
std,444416,39221.500000,54388.455312
std,445440,39345.000000,54342.661075
std,446464,39126.750000,54771.408308
std,447488,39432.750000,54471.027255
std,448512,39574.500000,54400.121290
std,449536,39572.500000,54527.078148
std,450560,39731.000000,54433.263698
std,451584,39618.000000,54712.585189
std,452608,39635.000000,54813.129810
std,453632,39709.750000,54833.727233
std,454656,39872.250000,54733.525196
std,455680,39930.250000,54777.117599
std,456704,40048.500000,54738.110042
std,457728,40191.500000,54665.648209
std,458752,40472.500000,54407.550806
std,459776,40266.250000,54808.302238
std,460800,40431.250000,54706.198794
std,461824,40500.500000,54734.020568
std,462848,40552.000000,54785.717104
std,463872,40660.250000,54760.745445
std,464896,40868.250000,54602.308638
std,465920,40930.250000,54639.685807
std,466944,40980.750000,54692.293333
std,467968,41281.750000,54412.576986
std,468992,41244.500000,54580.891998
std,470016,41373.750000,54529.183359
std,471040,41459.000000,54535.613498
std,472064,41426.500000,54697.046576
std,473088,41638.750000,54536.276906
std,474112,41881.250000,54337.862409
std,475136,41867.750000,54472.781556
std,476160,41810.250000,54665.255529
std,477184,42088.000000,54421.288728
std,478208,41926.750000,54747.825672
std,479232,41861.000000,54951.233845
std,480256,41778.500000,55177.395072
std,481280,42183.250000,54764.485904
std,482304,42269.000000,54769.670444
std,483328,42221.000000,54948.352715
std,484352,42152.750000,55153.924714
std,485376,42671.000000,54599.254763
std,486400,42568.250000,54846.511191
std,487424,42992.250000,54419.929173
std,488448,43155.750000,54327.648112
std,489472,42735.750000,54976.585178
std,490496,42765.500000,55053.274251
std,491520,42727.000000,55217.918412
std,492544,43209.500000,54715.078860
std,493568,43329.750000,54676.669032
std,494592,43329.250000,54790.738358
std,495616,43473.000000,54722.627838
std,496640,43580.250000,54700.741735
std,497664,43694.250000,54670.516143
std,498688,43990.750000,54413.766530
std,499712,44084.750000,54409.236754
std,500736,44122.000000,54474.701963
std,501760,43972.250000,54771.998249
std,502784,44006.000000,54841.685225
std,503808,44210.500000,54699.186845
std,504832,44089.500000,54960.786582
std,505856,44272.750000,54844.318458
std,506880,44266.750000,54962.788097
std,507904,44813.750000,54401.588798
std,508928,44572.500000,54806.313310
std,509952,44801.750000,54635.580083
std,510976,45000.000000,54504.106667
std,512000,44817.250000,54836.028538
std,513024,44986.250000,54739.285893
std,514048,44876.750000,54982.377289
std,515072,45388.000000,54471.349255
std,516096,45084.000000,54947.671014
std,517120,45074.500000,55068.298040
std,518144,45211.000000,55010.754020
std,519168,45286.000000,55028.185311
std,520192,45319.000000,55096.573181
std,521216,45529.000000,54950.400843
std,522240,45783.250000,54752.600569
std,523264,45720.250000,54935.552627
std,524288,45777.500000,54974.220960
std,525312,45841.500000,55004.692255
std,526336,46199.750000,54684.555652
std,527360,46443.750000,54503.092451
std,528384,46450.250000,54601.282017
std,529408,46335.500000,54842.580743
std,530432,46297.250000,54994.056882
std,531456,46453.750000,54914.593547
std,532480,46327.500000,55170.341590
std,533504,46633.500000,54913.725112
std,534528,46530.250000,55141.212437
std,535552,46626.750000,55132.506555
std,536576,46837.250000,54989.667412
std,537600,46949.250000,54963.178326
std,538624,46904.750000,55120.114701
std,539648,46947.250000,55174.912269
std,540672,46919.500000,55312.302987
std,541696,47047.000000,55266.877803
std,542720,47113.750000,55292.902815
std,543744,47093.000000,55421.638035
std,544768,47243.250000,55349.418171
std,545792,47253.000000,55442.016380
std,546816,47215.250000,55590.445883
std,547840,47405.000000,55471.616918
std,548864,47582.500000,55367.986129
std,549888,47510.500000,55555.348818
std,550912,47581.250000,55576.043084
std,551936,47705.000000,55534.908291
std,552960,47807.000000,55519.233585
std,553984,47843.750000,55579.322273
std,555008,47848.250000,55676.819946
std,556032,48014.500000,55586.408273
std,557056,48121.500000,55564.951217
std,558080,48454.750000,55284.239419
std,559104,48366.500000,55486.735654
std,560128,48553.750000,55373.980383
std,561152,49134.250000,54819.796781
std,562176,49263.500000,54775.742690
std,563200,49097.000000,55061.612726
std,564224,49234.750000,55007.392137
std,565248,49379.500000,54945.683938
std,566272,49214.000000,55230.332832
std,567296,49555.250000,54949.189036
std,568320,49506.250000,55102.860750
std,569344,49472.000000,55240.362225
std,570368,49372.750000,55450.960297
std,571392,49721.000000,55161.432795
std,572416,49633.000000,55358.265670
std,573440,49825.500000,55243.038203
std,574464,49857.250000,55306.443897
std,575488,49769.250000,55502.994319
std,576512,50005.250000,55339.341369
std,577536,50047.500000,55390.834707
std,578560,50116.250000,55412.924949
std,579584,50049.500000,55585.034816
std,580608,50121.000000,55603.806788
std,581632,50371.750000,55424.590172
std,582656,50438.250000,55448.965815
std,583680,50372.500000,55618.919053
std,584704,50433.250000,55649.382104
std,585728,50772.250000,55374.626888
std,586752,50841.750000,55395.606957
std,587776,50771.750000,55568.791700
std,588800,50881.500000,55545.532266
std,589824,50794.000000,55737.984801
std,590848,50590.750000,56059.070087
std,591872,51178.500000,55511.310414
std,592896,51176.500000,55609.523903
std,593920,51299.750000,55571.732806
std,594944,51548.750000,55398.650792
std,595968,51521.500000,55523.352387
std,596992,51492.750000,55649.807012
std,598016,51621.250000,55606.495387
std,599040,51330.750000,56016.948905
std,600064,51499.000000,55929.381153
std,601088,52119.000000,55358.360675
std,602112,51932.750000,55651.541657
std,603136,52114.250000,55552.038070
std,604160,52249.000000,55502.842160
std,605184,52482.500000,55349.558424
std,606208,52087.250000,55863.928313
std,607232,52298.750000,55731.993594
std,608256,52312.250000,55811.569948
std,609280,52210.500000,56014.479846
std,610304,52455.000000,55847.091793
std,611328,52629.750000,55755.051088
std,612352,52966.750000,55493.108412
std,613376,52776.750000,55786.019412
std,614400,52880.250000,55769.781724
std,615424,53068.500000,55664.569377
std,616448,53195.750000,55623.812053
std,617472,53111.750000,55804.329550
std,618496,53200.750000,55803.363674
std,619520,53095.500000,56006.554228
std,620544,52994.000000,56206.574329
std,621568,53023.500000,56268.001924
std,622592,53181.000000,56193.783494
std,623616,53718.500000,55723.015349
std,624640,53706.500000,55826.985560
std,625664,53844.500000,55775.189667
std,626688,53944.500000,55762.911882
std,627712,53958.000000,55840.053375
std,628736,53753.250000,56144.192212
std,629760,54130.500000,55843.711032
std,630784,53856.000000,56219.607843
std,631808,53752.750000,56419.037166
std,632832,53924.500000,56330.491706
std,633856,54198.250000,56136.661239
std,634880,54264.000000,56159.221583
std,635904,54340.000000,56171.129923
std,636928,54536.500000,56058.866997
std,637952,54945.000000,55731.542452
std,638976,54671.750000,56099.993141
std,640000,54749.000000,56110.613892
std,641024,54848.000000,56098.949825
std,642048,54864.750000,56171.410605
std,643072,54950.750000,56172.947594
std,644096,54939.000000,56274.428002
std,645120,54858.250000,56446.860773
std,646144,54767.000000,56630.657148
std,647168,55168.250000,56307.865484
std,648192,55433.250000,56127.353168
std,649216,55310.250000,56341.036246
std,650240,55369.500000,56369.517514
std,651264,55893.000000,55929.493854
std,652288,55823.500000,56087.174756
std,653312,55774.000000,56225.079786
std,654336,55989.750000,56096.210467
std,655360,56213.750000,55960.116520
std,656384,56635.250000,55630.428046
std,657408,56361.000000,55988.332357
std,658432,56499.000000,55938.575904
std,659456,56149.250000,56374.551753
std,660480,56484.000000,56127.469726
std,661504,56746.000000,55954.943080
std,662528,56593.750000,56192.325124
std,663552,56721.750000,56152.174430
std,664576,56757.500000,56203.405717
std,665600,57248.250000,55807.470097
std,666624,56961.250000,56174.947003
std,667648,57277.000000,55951.086824
std,668672,57068.750000,56241.386048
std,669696,57174.000000,56223.822017
std,670720,57423.500000,56065.130130
std,671744,58574.250000,55047.588317
std,672768,58211.250000,55475.297301
std,673792,57797.500000,55957.465288
std,674816,57764.500000,56074.523280
std,675840,57722.750000,56200.233010
std,676864,57716.250000,56291.723735
std,677888,57903.750000,56194.329383
std,678912,58134.250000,56056.070217
std,679936,57931.500000,56337.101577
std,680960,58048.750000,56307.982515
std,681984,58190.750000,56255.043972
std,683008,58037.750000,56488.034081
std,684032,58381.750000,56239.383026
std,685056,58374.250000,56330.810246
std,686080,58655.750000,56144.265481
std,687104,58596.750000,56284.677904
std,688128,58837.750000,56137.673517
std,689152,58998.500000,56068.028848
std,690176,59261.500000,55902.142200
std,691200,58958.750000,56272.563445
std,692224,58918.000000,56394.908177
std,693248,58982.250000,56416.810142
std,694272,59047.250000,56437.947576
std,695296,58920.500000,56642.777980
std,696320,59047.500000,56604.191541
std,697344,59436.000000,56316.898849
std,698368,59476.250000,56361.428301
std,699392,59563.000000,56361.862230
std,700416,59493.750000,56510.083832
std,701440,59411.500000,56671.048534
std,702464,59483.000000,56685.560580
std,703488,59680.750000,56580.093246
std,704512,60422.750000,55966.628464
std,705536,60241.250000,56216.841450
std,706560,60400.250000,56150.231166
std,707584,60448.750000,56186.491863
std,708608,60386.500000,56325.807921
std,709632,60538.500000,56265.576451
std,710656,60925.000000,55989.311449
std,711680,60750.250000,56231.274768
std,712704,60805.000000,56261.478497
std,713728,60607.000000,56526.381441
std,714752,60823.250000,56406.219661
std,715776,61121.000000,56211.855173
std,716800,61148.000000,56267.416759
std,717824,60998.250000,56486.131979
std,718848,61189.250000,56390.140425
std,719872,61181.000000,56478.083065
std,720896,61177.000000,56562.119751
std,721920,61453.250000,56387.839537
std,722944,61379.250000,56535.900976
std,723968,61534.250000,56473.368896
std,724992,61590.000000,56502.055528
std,726016,61927.500000,56273.494005
std,727040,61763.000000,56502.954844
std,728064,61886.500000,56469.621000
std,729088,61952.250000,56489.028244
std,730112,62126.000000,56410.159997
std,731136,62153.500000,56464.282784
std,732160,62273.750000,56434.179731
std,733184,62361.500000,56433.588031
std,734208,62312.750000,56556.618028
std,735232,62796.250000,56199.432291
std,736256,62982.250000,56111.504432
std,737280,63093.000000,56090.913414
std,738304,62981.250000,56268.479905
std,739328,62947.750000,56376.509089
std,740352,63255.750000,56179.708564
std,741376,63182.500000,56322.633641
std,742400,63527.000000,56094.573961
std,743424,63182.000000,56478.667975
std,744448,63329.750000,56424.514545
std,745472,63248.250000,56574.934484
std,746496,63673.000000,56274.728692
std,747520,63968.750000,56091.388373
std,748544,63901.750000,56227.117411
std,749568,64045.750000,56177.441907
std,750592,63953.000000,56335.771582
std,751616,63872.250000,56483.947254
std,752640,63506.250000,56886.873339
std,753664,63984.750000,56538.272010
std,754688,64275.750000,56358.772943
std,755712,64084.250000,56603.886290
std,756736,64005.000000,56750.766346
std,757760,64276.000000,56587.964404
std,758784,64660.250000,56327.700558
std,759808,64589.750000,56465.281256
std,760832,64632.000000,56504.418864
std,761856,64608.000000,56601.485884
std,762880,65196.250000,56166.175202
std,763904,64820.750000,56567.367702
std,764928,64948.250000,56531.998938
std,765952,65498.750000,56131.904807
std,766976,65023.500000,56617.758195
std,768000,65207.000000,56533.807720
std,769024,65193.500000,56620.908526
std,770048,65865.000000,56118.278297
std,771072,65444.750000,56553.743425
std,772096,65538.500000,56547.842871
std,773120,65563.000000,56601.680826
std,774144,65833.750000,56443.559724
std,775168,65960.250000,56409.828647
std,776192,66022.500000,56431.089401
std,777216,66375.750000,56204.815765
std,778240,66278.750000,56361.231918
std,779264,66395.750000,56335.943189
std,780288,66418.250000,56390.862451
std,781312,67263.750000,55755.107320
std,782336,68945.000000,54466.789470
std,783360,66113.750000,56873.615549
std,784384,66520.750000,56599.530222
std,785408,66613.250000,56594.722521
std,786432,66671.750000,56618.786818
std,787456,66954.250000,56453.306549
std,788480,67212.750000,56309.316313
std,789504,67336.500000,56278.826491
std,790528,67535.250000,56185.982876
std,791552,67575.000000,56225.669256
std,792576,67441.750000,56409.639430
std,793600,67628.250000,56326.756940
std,794624,67811.750000,56246.818582
std,795648,67486.500000,56590.731480
std,796672,67636.750000,56537.689939
std,797696,67680.250000,56573.975421
std,798720,67653.750000,56668.787761
std,799744,67680.250000,56719.223112
std,800768,67844.500000,56654.355180
std,801792,68062.250000,56545.318440
std,802816,67857.000000,56788.788187
std,803840,67934.000000,56796.773339
std,804864,67963.250000,56844.650602
std,805888,67927.250000,56947.136827
std,806912,68407.250000,56619.402183
std,807936,68410.750000,56688.353804
std,808960,68585.750000,56615.375643
std,809984,68417.750000,56826.235882
std,811008,68671.000000,56688.243946
std,812032,68700.750000,56735.240882
std,813056,68857.000000,56677.880245
std,814080,68808.250000,56789.469286
std,815104,69212.750000,56528.590469
std,816128,68558.750000,57139.524860
std,817152,68638.250000,57144.953433
std,818176,68797.000000,57084.535663
std,819200,69197.000000,56825.584924
std,820224,69516.250000,56635.321957
std,821248,69619.250000,56622.132528
std,822272,69584.500000,56721.045635
std,823296,69856.500000,56570.552490
std,824320,69707.500000,56761.984005
std,825344,69972.750000,56617.057354
std,826368,69820.750000,56810.710283
std,827392,69941.250000,56783.108680
std,828416,70303.250000,56560.639800
std,829440,70509.250000,56465.102097
std,830464,70502.500000,56540.224815
std,831488,70428.250000,56669.623340
std,832512,70649.750000,56561.524988
std,833536,70646.250000,56633.902012
std,834560,70353.750000,56939.224988
std,835584,70337.250000,57022.462493
std,836608,70404.750000,57037.606127
std,837632,70454.750000,57066.891870
std,838656,70364.750000,57209.736409
std,839680,70573.500000,57110.161746
std,840704,70477.000000,57258.101225
std,841728,71222.750000,56727.582128
std,842752,71102.000000,56893.049422
std,843776,71310.500000,56795.630377
std,844800,71382.000000,56807.598554
std,845824,71493.250000,56787.951310
std,846848,71644.000000,56737.066607
std,847872,71532.750000,56894.018474
std,848896,71691.750000,56836.397493
std,849920,71733.500000,56871.838123
std,850944,71892.250000,56814.624664
std,851968,72341.000000,56530.133672
std,852992,72064.750000,56815.039253
std,854016,71681.500000,57187.374706
std,855040,71698.250000,57242.568682
std,856064,72260.250000,56865.388647
std,857088,72282.500000,56915.884204
std,858112,72829.000000,56556.283898
std,859136,72877.750000,56585.896244
std,860160,72782.250000,56727.677421
std,861184,73565.250000,56190.704171
std,862208,72953.250000,56729.458934
std,863232,72587.000000,57083.411630
std,864256,72667.250000,57088.011449
std,865280,72796.250000,57054.367498
std,866304,73139.000000,56854.198171
std,867328,72746.500000,57228.518210
std,868352,72836.500000,57225.286772
std,869376,73018.000000,57150.357446
std,870400,73083.750000,57166.196316
std,871424,73165.000000,57169.892708
std,872448,73264.750000,57159.144063
std,873472,73072.500000,57376.791543
std,874496,73087.500000,57432.266803
std,875520,73110.000000,57481.821912
std,876544,73179.750000,57494.200240
std,877568,74002.500000,56921.406709
std,878592,73905.750000,57062.428837
std,879616,73709.000000,57281.428319
std,880640,73792.000000,57283.607979
std,881664,73849.500000,57305.563342
std,882688,74040.000000,57224.505673
std,883712,74022.500000,57304.435813
std,884736,74082.750000,57324.178706
std,885760,74303.250000,57220.215805
std,886784,74456.000000,57168.840658
std,887808,74584.250000,57136.438323
std,888832,75370.000000,56605.991774
std,889856,76151.500000,56089.621347
std,890880,75015.750000,57004.349087
std,891904,75133.250000,56980.620431
std,892928,75072.500000,57092.202871
std,893952,75122.500000,57119.632600
std,894976,75258.500000,57081.722330
std,896000,76652.500000,56107.759042
std,897024,75678.500000,56894.827461
std,898048,75653.500000,56978.598479
std,899072,75783.000000,56946.090812
std,900096,75868.000000,56947.076501
std,901120,75851.500000,57024.264517
std,902144,75829.750000,57105.439488
std,903168,75993.000000,57047.443844
std,904192,76219.000000,56942.778047
std,905216,75904.750000,57243.279241
std,906240,75921.250000,57295.579301
std,907264,75907.250000,57370.899354
std,908288,75938.500000,57412.016303
std,909312,76025.000000,57411.346268
std,910336,76262.750000,57296.816598
std,911360,76659.750000,57064.209054
std,912384,76618.250000,57159.269495
std,913408,76702.500000,57160.567126
std,914432,76939.750000,57048.191604
std,915456,76478.750000,57456.336564
std,916480,76638.000000,57401.080404
std,917504,76617.500000,57480.591249
std,918528,76938.750000,57304.471414
std,919552,76853.750000,57431.805214
std,920576,77139.500000,57282.777306
std,921600,77145.250000,57342.221329
std,922624,77420.500000,57201.841889
std,923648,77430.500000,57257.933243
std,924672,77776.750000,57066.226089
std,925696,77658.500000,57216.412885
std,926720,77658.750000,57279.520981
std,927744,78011.000000,57083.888170
std,928768,77922.250000,57211.982457
std,929792,77613.750000,57502.718268
std,930816,78445.250000,56955.861572
std,931840,78470.000000,57000.535236
std,932864,78199.250000,57260.743549
std,933888,78097.000000,57398.650396
std,934912,78102.000000,57457.908888
std,935936,78274.000000,57394.445154
std,936960,78456.000000,57323.952279
std,937984,78244.500000,57541.721143
std,939008,78352.750000,57524.954772
std,940032,78520.500000,57464.657000
std,941056,78634.750000,57443.672168
std,942080,78677.500000,57474.932478
std,943104,78880.000000,57389.695740
std,944128,78932.500000,57413.795331
std,945152,78778.250000,57588.605992
std,946176,78963.750000,57515.566320
std,947200,78880.500000,57638.579877
std,948224,79439.500000,57294.862128
std,949248,79531.500000,57290.386828
std,950272,79704.500000,57227.704835
std,951296,79255.250000,57614.111368
std,952320,79946.250000,57177.616211
std,953344,80028.750000,57180.090905
std,954368,80067.750000,57213.627209
std,955392,80026.500000,57304.537872
std,956416,80107.000000,57308.310135
std,957440,80187.500000,57312.074825
std,958464,80355.500000,57253.420114
std,959488,80279.250000,57369.026243
std,960512,80393.000000,57348.993072
std,961536,80484.000000,57345.221410
std,962560,80892.750000,57116.218697
std,963584,81129.000000,57010.479607
std,964608,81014.250000,57151.901054
std,965632,81529.250000,56851.174272
std,966656,81078.750000,57227.680496
std,967680,81953.750000,56676.649940
std,968704,81356.500000,57153.137119
std,969728,80631.000000,57728.347658
std,970752,80794.000000,57672.718271
std,971776,80846.000000,57696.420355
std,972800,81067.750000,57599.230273
std,973824,81179.250000,57580.665010
std,974848,81058.000000,57727.434676
std,975872,81141.750000,57728.427104
std,976896,81453.250000,57568.001277
std,977920,81325.750000,57718.693034
std,978944,82028.500000,57284.129297
std,979968,82229.000000,57204.227219
std,980992,82308.250000,57208.865454
std,982016,82408.250000,57199.088683
std,983040,82513.500000,57185.696886
std,984064,82762.500000,57073.036701
std,985088,82362.750000,57409.719807
std,986112,82816.000000,57154.868624
std,987136,83176.250000,56966.415293
std,988160,83093.250000,57082.470598
std,989184,83221.250000,57053.735674
std,990208,83257.250000,57088.102237
std,991232,83275.500000,57134.614623
std,992256,82861.500000,57479.393928
std,993280,83105.250000,57369.949552
std,994304,83192.750000,57368.691382
std,995328,83505.000000,57213.033950
std,996352,84011.750000,56926.437076
std,997376,83935.250000,57036.880214
std,998400,84089.250000,56990.875766
std,999424,83386.250000,57530.290665
std,1000448,83673.000000,57391.875515
std,1001472,83559.500000,57528.654432
std,1002496,83789.000000,57429.743761
std,1003520,84117.500000,57263.898713
std,1004544,84196.500000,57268.546792
std,1005568,84138.000000,57366.783142
std,1006592,84184.000000,57393.823054
std,1007616,84541.750000,57209.092549
std,1008640,84610.250000,57220.868630
std,1009664,84644.750000,57255.614790
std,1010688,84825.000000,57191.893899
std,1011712,84823.750000,57250.682739
std,1012736,84795.500000,57327.721400
std,1013760,84958.750000,57275.418953
std,1014784,85075.000000,57254.930356
std,1015808,85246.250000,57197.570568
std,1016832,84974.500000,57438.332676
std,1017856,85026.750000,57460.843793
std,1018880,85146.750000,57437.588634
std,1019904,85402.750000,57322.969108
std,1020928,85357.250000,57411.109191
std,1021952,85638.000000,57280.291459
std,1022976,85555.250000,57393.144196
std,1024000,85641.750000,57392.568461
buff,1024,1721.000000,2856.013945
buff,2048,1806.250000,5442.435986
buff,3072,1895.000000,7781.319261
buff,4096,2013.000000,9766.915052
buff,5120,2253.250000,10906.912238
buff,6144,2354.750000,12524.132073
buff,7168,2456.750000,14004.843798
buff,8192,2580.750000,15236.501017
buff,9216,2782.750000,15896.792741
buff,10240,2917.750000,16845.857253
buff,11264,3031.250000,17836.602062
buff,12288,3095.000000,19057.318255
buff,13312,3263.750000,19577.970126
buff,14336,3467.000000,19847.937698
buff,15360,3504.250000,21039.594778
buff,16384,3962.750000,19845.612264
buff,17408,4109.250000,20334.221573
buff,18432,4322.750000,20466.971257
buff,19456,4457.750000,20949.761651
buff,20480,4590.000000,21416.993464
buff,21504,4730.250000,21821.087680
buff,22528,4950.750000,21842.023936
buff,23552,5067.750000,22307.651325
buff,24576,5190.000000,22729.248555
buff,25600,5405.000000,22734.505088
buff,26624,5571.500000,22937.305932
buff,27648,5726.750000,23173.772209
buff,28672,5901.500000,23320.443955
buff,29696,6088.250000,23412.441999
buff,30720,6328.250000,23301.228618
buff,31744,6511.500000,23400.322506
buff,32768,6908.000000,22768.731905
buff,33792,7178.750000,22594.685704
buff,34816,7427.750000,22498.980176
buff,35840,7477.000000,23008.158352
buff,36864,7526.250000,23510.672646
buff,37888,7605.250000,23912.744486
buff,38912,7775.000000,24022.842444
buff,39936,7806.250000,24556.323459
buff,40960,7901.500000,24882.364108
buff,41984,8041.250000,25061.178299
buff,43008,8212.250000,25137.861122
buff,44032,8370.750000,25249.063704
buff,45056,8589.000000,25179.741530
buff,46080,8846.500000,25002.430340
buff,47104,8918.500000,25351.707126
buff,48128,9030.750000,25580.865377
buff,49152,9145.000000,25798.753417
buff,50176,9420.250000,25566.710013
buff,51200,9655.250000,25453.509749
buff,52224,9848.500000,25453.134995
buff,53248,10078.750000,25359.335235
buff,54272,10084.250000,25832.917669
buff,55296,10279.500000,25820.399825
buff,56320,10433.750000,25909.763987
buff,57344,10563.000000,26058.051690
buff,58368,10763.000000,26030.511939
buff,59392,10916.500000,26114.743737
buff,60416,11102.750000,26119.366824
buff,61440,11193.250000,26347.307529
buff,62464,11397.000000,26307.554620
buff,63488,11622.000000,26221.166753
buff,64512,11718.750000,26424.115200
buff,65536,510765.500000,615.884980
buff,66560,511031.500000,625.182596
buff,67584,510999.500000,634.840543
buff,68608,511158.000000,644.259505
buff,69632,511314.000000,653.675823
buff,70656,511544.000000,662.990476
buff,71680,511633.000000,672.482033
buff,72704,511706.750000,681.990613
buff,73728,511863.250000,691.384662
buff,74752,512002.000000,700.797263
buff,75776,512154.000000,710.186389
buff,76800,512483.250000,719.321070
buff,77824,512656.000000,728.666396
buff,78848,512668.250000,738.236472
buff,79872,512839.000000,747.574970
buff,80896,513091.750000,756.786286
buff,81920,513036.000000,766.449138
buff,82944,513309.250000,775.616648
buff,83968,513425.250000,785.014761
buff,84992,513600.750000,794.316597
buff,86016,513742.250000,803.665262
buff,87040,513874.500000,813.023413
buff,88064,514163.000000,822.126835
buff,89088,514377.500000,831.339629
buff,90112,514714.500000,840.344696
buff,91136,514727.750000,849.872190
buff,92160,514862.250000,859.196805
buff,93184,514923.250000,868.640521
buff,94208,515109.500000,877.868492
buff,95232,515165.000000,887.314938
buff,96256,515271.250000,896.671025
buff,97280,515392.000000,905.997765
buff,98304,515734.500000,914.926576
buff,99328,515783.750000,924.368788
buff,100352,516010.000000,933.488886
buff,101376,516218.000000,942.634313
buff,102400,516159.000000,952.264709
buff,103424,516232.500000,961.650419
buff,104448,516358.000000,970.935669
buff,105472,516379.250000,980.414298
buff,106496,516586.000000,989.536689
buff,107520,516641.500000,998.944142
buff,108544,516790.500000,1008.167139
buff,109568,516790.000000,1017.679135
buff,110592,517087.250000,1026.599670
buff,111616,517079.750000,1036.120250
buff,112640,517220.750000,1045.340892
buff,113664,517322.750000,1054.636008
buff,114688,517550.500000,1063.668956
buff,115712,517648.250000,1072.963349
buff,116736,517982.500000,1081.760098
buff,117760,517968.250000,1091.279243
buff,118784,518191.000000,1100.295451
buff,119808,518409.750000,1109.312470
buff,120832,518490.500000,1118.619531
buff,121856,518617.750000,1127.822563
buff,122880,518711.500000,1137.094512
buff,123904,518939.750000,1146.065993
buff,124928,519207.500000,1154.941714
buff,125952,519226.000000,1164.366962
buff,126976,519244.500000,1173.791538
buff,128000,519318.750000,1183.088421
buff,129024,519803.000000,1191.442143
buff,130048,519672.250000,1201.200180
buff,131072,519872.000000,1210.193278
buff,132096,520590.500000,1217.964600
buff,133120,520624.250000,1227.326618
buff,134144,520886.000000,1236.146105
buff,135168,520918.500000,1245.504623
buff,136192,521073.500000,1254.566966
buff,137216,521271.500000,1263.519682
buff,138240,521571.500000,1272.216753
buff,139264,521602.500000,1281.564410
buff,140288,521748.250000,1290.627041
buff,141312,521780.500000,1299.967323
buff,142336,522143.000000,1308.478329
buff,143360,522114.250000,1317.964411
buff,144384,522332.750000,1326.823179
buff,145408,522261.000000,1336.416849
buff,146432,522482.250000,1345.258332
buff,147456,522503.000000,1354.611935
buff,148480,522801.250000,1363.240811
buff,149504,522846.250000,1372.524332
buff,150528,522931.250000,1381.700558
buff,151552,523395.750000,1389.865317
buff,152576,523387.250000,1399.279023
buff,153600,523339.750000,1408.798013
buff,154624,523531.750000,1417.669893
buff,155648,523702.250000,1426.593833
buff,156672,523807.250000,1435.691469
buff,157696,523916.250000,1444.774427
buff,158720,524233.000000,1453.277455
buff,159744,524240.500000,1462.632513
buff,160768,524418.000000,1471.510131
buff,161792,524677.000000,1480.151789
buff,162816,524809.000000,1489.145194
buff,163840,524995.750000,1497.977841
buff,164864,525105.500000,1507.025160
buff,165888,525196.500000,1516.122823
buff,166912,525285.750000,1525.222415
buff,167936,525433.500000,1534.148089
buff,168960,525565.750000,1543.114254
buff,169984,525921.250000,1551.417061
buff,171008,526116.500000,1560.183724
buff,172032,526371.250000,1568.766531
buff,173056,526463.000000,1577.829401
buff,174080,526474.000000,1587.132508
buff,175104,526378.500000,1596.758226
buff,176128,526713.250000,1605.075247
buff,177152,526701.000000,1614.444628
buff,178176,526915.250000,1623.116431
buff,179200,527090.250000,1631.902696
buff,180224,527178.000000,1640.954668
buff,181248,527120.750000,1650.457509
buff,182272,527241.000000,1659.403574
buff,183296,527384.750000,1668.271219
buff,184320,527579.750000,1676.971112
buff,185344,527769.500000,1685.681344
buff,186368,527869.250000,1694.674202
buff,187392,528062.750000,1703.361201
buff,188416,528205.500000,1712.206329
buff,189440,528243.500000,1721.387958
buff,190464,528614.250000,1729.478916
buff,191488,528565.500000,1738.937558
buff,192512,528853.250000,1747.285471
buff,193536,529011.750000,1756.053245
buff,194560,529179.000000,1764.786585
buff,195584,529335.250000,1773.551261
buff,196608,529675.750000,1781.690780
buff,197632,529983.250000,1789.931286
buff,198656,530225.750000,1798.382670
buff,199680,530251.500000,1807.564901
buff,200704,530185.250000,1817.061489
buff,201728,530454.500000,1825.405195
buff,202752,530457.750000,1834.659971
buff,203776,530734.750000,1842.963552
buff,204800,530905.250000,1851.629834
buff,205824,530995.500000,1860.571700
buff,206848,531108.250000,1869.431326
buff,207872,533397.500000,1870.622941
buff,208896,531418.250000,1886.839227
buff,209920,531365.250000,1896.277560
buff,210944,531370.000000,1905.510661
buff,211968,531616.250000,1913.873776
buff,212992,531756.750000,1922.611420
buff,214016,531828.750000,1931.593206
buff,215040,531823.750000,1940.853525
buff,216064,532245.250000,1948.551349
buff,217088,532245.000000,1957.787109
buff,218112,532454.000000,1966.249854
buff,219136,532398.000000,1975.688864
buff,220160,532780.250000,1983.496948
buff,221184,532675.000000,1993.116253
buff,222208,533023.750000,2001.033537
buff,223232,532875.250000,2010.815102
buff,224256,532961.000000,2019.714013
buff,225280,533135.250000,2028.273313
buff,226304,533408.500000,2036.448988
buff,227328,533720.000000,2044.469759
buff,228352,534345.500000,2051.275065
buff,229376,534155.250000,2061.207486
buff,230400,534714.000000,2068.245829
buff,231424,534702.750000,2077.481741
buff,232448,534897.500000,2085.914404
buff,233472,534997.750000,2094.710866
buff,234496,535066.750000,2103.626884
buff,235520,535292.250000,2111.922973
buff,236544,535479.000000,2120.365505
buff,237568,535572.250000,2129.173795
buff,238592,535728.750000,2137.726601
buff,239616,535836.250000,2146.470680
buff,240640,535949.000000,2155.190139
buff,241664,536052.000000,2163.945289
buff,242688,536138.750000,2172.762928
buff,243712,536203.250000,2181.668239
buff,244736,536363.000000,2190.182395
buff,245760,536447.250000,2199.000927
buff,246784,536502.250000,2207.937059
buff,247808,536739.750000,2216.117588
buff,248832,536778.500000,2225.114456
buff,249856,536685.750000,2234.657432
buff,250880,536774.500000,2243.444873
buff,251904,536890.750000,2252.114047
buff,252928,537156.250000,2260.151306
buff,253952,537277.000000,2268.791703
buff,254976,537369.250000,2277.549004
buff,256000,537603.500000,2285.699405
buff,257024,537974.000000,2293.261756
buff,258048,537855.750000,2302.904450
buff,259072,537996.500000,2311.438086
buff,260096,538102.000000,2320.119234
buff,261120,538355.000000,2328.158929
buff,262144,538668.500000,2335.928683
buff,263168,538594.500000,2345.375603
buff,264192,538784.000000,2353.673457
buff,265216,538718.000000,2363.085696
buff,266240,539004.250000,2370.949765
buff,267264,539211.250000,2379.155109
buff,268288,539388.000000,2387.488042
buff,269312,539403.250000,2396.532835
buff,270336,539490.500000,2405.256070
buff,271360,539753.500000,2413.190466
buff,272384,539641.250000,2422.800703
buff,273408,539839.000000,2431.018137
buff,274432,540192.750000,2438.525138
buff,275456,540238.000000,2447.419100
buff,276480,540563.250000,2455.039258
buff,277504,540734.250000,2463.352747
buff,278528,540508.000000,2473.477543
buff,279552,540784.500000,2481.301886
buff,280576,540824.500000,2490.206712
buff,281600,540835.750000,2499.243070
buff,282624,541004.250000,2507.549987
buff,283648,541144.750000,2515.981907
buff,284672,541169.750000,2524.948226
buff,285696,541428.750000,2532.818584
buff,286720,541269.250000,2542.645827
buff,287744,541379.500000,2551.207055
buff,288768,541571.000000,2559.380764
buff,289792,541863.000000,2567.072489
buff,290816,542051.000000,2575.249930
buff,291840,542139.000000,2583.898225
buff,292864,542372.250000,2591.849417
buff,293888,542634.750000,2599.653634
buff,294912,542838.500000,2607.732502
buff,295936,542969.000000,2616.158197
buff,296960,543172.250000,2624.228318
buff,297984,543330.500000,2632.510415
buff,299008,543461.750000,2640.918887
buff,300032,543198.000000,2651.249821
buff,301056,543394.500000,2659.336449
buff,302080,543638.250000,2667.185394
buff,303104,543681.250000,2676.015036
buff,304128,543801.250000,2684.463120
buff,305152,543769.000000,2693.661463
buff,306176,543975.750000,2701.673374
buff,307200,543906.000000,2711.056690
buff,308224,543982.500000,2719.711020
buff,309248,544288.000000,2727.215004
buff,310272,544694.750000,2734.202230
buff,311296,544494.750000,2744.233622
buff,312320,544722.250000,2752.110823
buff,313344,544997.500000,2759.739632
buff,314368,545186.500000,2767.798542
buff,315392,545463.250000,2775.405309
buff,316416,545161.000000,2785.960111
buff,317440,545155.750000,2795.003079
buff,318464,545361.000000,2802.963908
buff,319488,545414.750000,2811.699537
buff,320512,545681.000000,2819.335106
buff,321536,548270.500000,2814.984209
buff,322560,545969.000000,2835.853318
buff,323584,546196.500000,2843.671096
buff,324608,546416.750000,2851.520200
buff,325632,546472.750000,2860.222399
buff,326656,546934.750000,2866.793160
buff,327680,546758.000000,2876.709623
buff,328704,546987.750000,2884.487267
buff,329728,547137.750000,2892.679951
buff,330752,547279.750000,2900.910549
buff,331776,547284.500000,2909.866441
buff,332800,547229.000000,2919.143540
buff,333824,547437.000000,2927.012971
buff,334848,547931.250000,2933.343189
buff,335872,548019.750000,2941.838501
buff,336896,548102.750000,2950.360676
buff,337920,548254.500000,2958.509232
buff,338944,548015.750000,2968.767230
buff,339968,548180.000000,2976.844102
buff,340992,548694.000000,2983.013483
buff,342016,548826.000000,2991.251872
buff,343040,548930.750000,2999.635200
buff,344064,549211.500000,3007.051382
buff,345088,549143.750000,3016.373035
buff,346112,548940.500000,3026.443850
buff,347136,549304.500000,3033.386400
buff,348160,549512.750000,3041.181483
buff,349184,549389.250000,3050.811788
buff,350208,549662.750000,3058.235982
buff,351232,549643.500000,3067.285613
buff,352256,549808.750000,3075.303549
buff,353280,549813.000000,3084.219544
buff,354304,549938.000000,3092.456241
buff,355328,550163.500000,3100.122782
buff,356352,550590.250000,3106.647094
buff,357376,550711.000000,3114.891113
buff,358400,550445.750000,3125.321614
buff,359424,550876.750000,3131.798901
buff,360448,551238.250000,3138.661731
buff,361472,551428.500000,3146.492428
buff,362496,551597.000000,3154.442102
buff,363520,552490.750000,3158.235681
buff,364544,551767.500000,3171.283557
buff,365568,551844.250000,3179.749359
buff,366592,552137.500000,3186.962668
buff,367616,552087.250000,3196.155680
buff,368640,552114.500000,3204.900433
buff,369664,552275.750000,3212.864588
buff,370688,552677.000000,3219.425451
buff,371712,552801.750000,3227.590361
buff,372736,552955.500000,3235.581887
buff,373760,553316.000000,3242.356989
buff,374784,553073.500000,3252.665695
buff,375808,553131.500000,3261.210761
buff,376832,552848.000000,3271.773797
buff,377856,553228.500000,3278.408108
buff,378880,553261.000000,3287.099579
buff,379904,553657.000000,3293.626198
buff,380928,553672.000000,3302.414426
buff,381952,553905.750000,3309.894508
buff,382976,554085.000000,3317.694578
buff,384000,554002.250000,3327.062300
buff,385024,554705.750000,3331.703701
buff,386048,554188.000000,3343.685536
buff,387072,554636.500000,3349.843726
buff,388096,554665.000000,3358.533169
buff,389120,554862.750000,3366.194613
buff,390144,555107.250000,3373.566459
buff,391168,555282.000000,3381.356500
buff,392192,555429.250000,3389.309440
buff,393216,555337.000000,3398.723298
buff,394240,555476.250000,3406.719909
buff,395264,555849.250000,3413.276531
buff,396288,555807.000000,3422.379351
buff,397312,555955.000000,3430.309288
buff,398336,556064.000000,3438.476147
buff,399360,556097.750000,3447.106197
buff,400384,556355.250000,3454.345403
buff,401408,556532.000000,3462.080168
buff,402432,556533.500000,3470.902650
buff,403456,556487.500000,3480.022103
buff,404480,556932.250000,3486.068548
buff,405504,557198.500000,3493.224049
buff,406528,556885.500000,3504.013662
buff,407552,556957.750000,3512.384198
buff,408576,557096.500000,3520.332294
buff,409600,557307.750000,3527.817440
buff,410624,557409.500000,3535.991403
buff,411648,557796.000000,3542.353118
buff,412672,558153.250000,3548.891993
buff,413696,558160.250000,3557.653559
buff,414720,558332.000000,3565.362544
buff,415744,558264.750000,3574.596462
buff,416768,558417.000000,3582.423888
buff,417792,558377.250000,3591.481566
buff,418816,558535.750000,3599.262536
buff,419840,558792.000000,3606.408109
buff,420864,558581.000000,3616.569844
buff,421888,558757.750000,3624.222483
buff,422912,558779.750000,3632.876102
buff,423936,559108.000000,3639.534401
buff,424960,559161.750000,3647.974848
buff,425984,559527.000000,3654.378073
buff,427008,559591.500000,3662.740410
buff,428032,560058.500000,3668.462491
buff,429056,559877.750000,3678.425871
buff,430080,559962.000000,3686.650166
buff,431104,560081.750000,3694.637792
buff,432128,560950.500000,3697.678137
buff,433152,561195.500000,3704.822295
buff,434176,560876.500000,3715.692849
buff,435200,560986.000000,3723.729291
buff,436224,560846.000000,3733.422722
buff,437248,561131.250000,3740.284292
buff,438272,560978.750000,3750.062903
buff,439296,561166.250000,3757.568813
buff,440320,561169.500000,3766.305902
buff,441344,561537.000000,3772.594148
buff,442368,561426.750000,3782.089827
buff,443392,561566.750000,3789.899598
buff,444416,561652.750000,3798.070605
buff,445440,561771.750000,3806.015521
buff,446464,562024.250000,3813.051127
buff,447488,562024.500000,3821.794957
buff,448512,562266.500000,3828.891816
buff,449536,562217.500000,3837.968046
buff,450560,562327.750000,3845.956384
buff,451584,562980.000000,3850.231269
buff,452608,563155.000000,3857.762783
buff,453632,563132.000000,3866.648672
buff,454656,562977.000000,3876.443976
buff,455680,563020.250000,3884.876254
buff,456704,563409.000000,3890.919740
buff,457728,563355.750000,3900.012381
buff,458752,564107.250000,3903.530047
buff,459776,564118.250000,3912.166997
buff,460800,564264.250000,3919.865559
buff,461824,563928.000000,3930.918841
buff,462848,563872.500000,3940.022611
buff,463872,564012.750000,3947.757564
buff,464896,564137.500000,3955.597350
buff,465920,564632.750000,3960.832948
buff,466944,564697.250000,3969.084673
buff,467968,564925.250000,3976.183398
buff,468992,565086.250000,3983.748676
buff,470016,565376.250000,3990.398960
buff,471040,564912.750000,4002.373818
buff,472064,565243.000000,4008.731112
buff,473088,565752.500000,4013.808865
buff,474112,565441.250000,4024.710967
buff,475136,565517.500000,4032.859814
buff,476160,565954.000000,4038.434219
buff,477184,566125.750000,4045.891218
buff,478208,565969.000000,4055.696337
buff,479232,566181.250000,4062.857256
buff,480256,566379.250000,4070.115210
buff,481280,566722.250000,4076.324866
buff,482304,566582.000000,4086.009086
buff,483328,566477.750000,4095.437817
buff,484352,566662.250000,4102.778330
buff,485376,566675.000000,4111.359774
buff,486400,566905.750000,4118.356535
buff,487424,567299.500000,4124.162281
buff,488448,567230.750000,4133.327398
buff,489472,567535.250000,4139.770349
buff,490496,570857.500000,4124.288110
buff,491520,568531.500000,4149.807003
buff,492544,568904.250000,4155.727787
buff,493568,568422.000000,4167.900609
buff,494592,568167.250000,4178.420351
buff,495616,568669.750000,4183.371456
buff,496640,569114.500000,4188.738821
buff,497664,568737.750000,4200.155871
buff,498688,568709.000000,4209.010935
buff,499712,568907.750000,4216.180216
buff,500736,569365.250000,4221.425175
buff,501760,570387.500000,4222.476825
buff,502784,569133.750000,4240.414841
buff,503808,569653.000000,4245.178029
buff,504832,569689.250000,4253.535765
buff,505856,569941.500000,4260.277239
buff,506880,569679.000000,4270.868331
buff,507904,570081.750000,4276.472980
buff,508928,570128.750000,4284.741648
buff,509952,570311.250000,4291.988980
buff,510976,570411.000000,4299.855367
buff,512000,570209.000000,4309.998615
buff,513024,570996.750000,4312.660624
buff,514048,571373.250000,4318.421277
buff,515072,571035.750000,4329.581116
buff,516096,571308.250000,4336.119424
buff,517120,571146.000000,4345.957076
buff,518144,571202.500000,4354.132204
buff,519168,571070.000000,4363.749453
buff,520192,571353.000000,4370.190758
buff,521216,571312.500000,4379.103905
buff,522240,571454.000000,4386.620795
buff,523264,571628.500000,4393.880291
buff,524288,571969.500000,4399.854188
buff,525312,572737.500000,4402.536240
buff,526336,572813.500000,4410.532922
buff,527360,572841.500000,4418.897723
buff,528384,572810.750000,4427.715786
buff,529408,572933.500000,4435.346161
buff,530432,573112.750000,4442.535260
buff,531456,573332.250000,4449.407477
buff,532480,573175.750000,4459.197724
buff,533504,573788.500000,4463.001960
buff,534528,573590.250000,4473.113690
buff,535552,573972.250000,4478.700146
buff,536576,574214.500000,4485.370537
buff,537600,574322.000000,4493.089243
buff,538624,574287.500000,4501.917942
buff,539648,574444.250000,4509.245936
buff,540672,574467.750000,4517.617569
buff,541696,576145.000000,4512.997249
buff,542720,574937.250000,4531.026647
buff,543744,574873.500000,4540.079165
buff,544768,574891.750000,4548.484823
buff,545792,574982.000000,4556.319328
buff,546816,575608.000000,4559.903268
buff,547840,575637.000000,4568.212259
buff,548864,575344.250000,4579.079742
buff,549888,575060.000000,4589.890446
buff,550912,575357.750000,4596.058018
buff,551936,575430.250000,4604.020731
buff,552960,575612.500000,4611.102087
buff,553984,575698.250000,4618.953071
buff,555008,575681.000000,4627.629538
buff,556032,576567.500000,4629.039271
buff,557056,576376.000000,4639.105029
buff,558080,576766.750000,4644.484100
buff,559104,576488.500000,4655.251926
buff,560128,576690.250000,4662.146447
buff,561152,576812.250000,4669.681686
buff,562176,576984.000000,4676.810449
buff,563200,576959.250000,4685.530217
buff,564224,576950.500000,4694.120553
buff,565248,577449.000000,4698.580134
buff,566272,577513.000000,4706.570415
buff,567296,577702.750000,4713.532695
buff,568320,577769.250000,4721.497380
buff,569344,578023.250000,4727.926083
buff,570368,578583.500000,4731.843200
buff,571392,578218.000000,4743.334867
buff,572416,578131.750000,4752.544381
buff,573440,578503.250000,4757.988827
buff,574464,578622.250000,4765.504956
buff,575488,578920.250000,4771.542194
buff,576512,578632.250000,4782.411627
buff,577536,578581.250000,4791.328443
buff,578560,578963.250000,4796.656783
buff,579584,579344.500000,4801.984312
buff,580608,578784.000000,4815.126887
buff,581632,578884.750000,4822.779664
buff,582656,579283.500000,4827.944866
buff,583680,579552.500000,4834.184996
buff,584704,579749.750000,4841.018388
buff,585728,579696.000000,4849.946179
buff,586752,579881.500000,4856.870930
buff,587776,580294.500000,4861.884440
buff,588800,580511.750000,4868.531946
buff,589824,580584.500000,4876.387847
buff,590848,580397.000000,4886.431873
buff,591872,580338.000000,4895.398199
buff,592896,581078.500000,4897.618480
buff,593920,581411.750000,4903.265199
buff,594944,581522.500000,4910.783676
buff,595968,581257.500000,4921.478691
buff,596992,581038.250000,4931.795110
buff,598016,581422.000000,4936.993784
buff,599040,582197.250000,4938.862216
buff,600064,581836.000000,4950.376395
buff,601088,581908.000000,4958.210576
buff,602112,582286.750000,4963.426697
buff,603136,582105.500000,4973.415987
buff,604160,584316.750000,4963.006794
buff,605184,582442.250000,4987.418409
buff,606208,582267.750000,4997.354568
buff,607232,582512.750000,5003.690649
buff,608256,582989.000000,5008.034114
buff,609280,582993.250000,5016.428578
buff,610304,583148.000000,5023.526103
buff,611328,583317.250000,5030.494812
buff,612352,583100.000000,5040.798491
buff,613376,583000.500000,5050.089665
buff,614400,583192.250000,5056.857323
buff,615424,583337.500000,5064.024171
buff,616448,583494.750000,5071.083159
buff,617472,583357.750000,5080.699794
buff,618496,583389.500000,5088.848531
buff,619520,584443.000000,5088.085579
buff,620544,584077.000000,5099.689253
buff,621568,584366.750000,5105.571801
buff,622592,584618.250000,5111.782946
buff,623616,584770.750000,5118.855209
buff,624640,584544.500000,5129.245079
buff,625664,584954.500000,5134.052649
buff,626688,584923.250000,5142.730093
buff,627712,584965.500000,5150.761199
buff,628736,585714.250000,5152.568509
buff,629760,585271.500000,5164.864512
buff,630784,585447.250000,5171.709663
buff,631808,585806.750000,5176.926350
buff,632832,586389.250000,5180.165905
buff,633856,586571.500000,5186.935949
buff,634880,585888.250000,5201.374153
buff,635904,586146.000000,5207.472541
buff,636928,585889.500000,5218.141646
buff,637952,586104.250000,5224.615928
buff,638976,586442.000000,5229.988302
buff,640000,586555.250000,5237.358288
buff,641024,586673.750000,5244.678495
buff,642048,586745.500000,5252.414207
buff,643072,587534.750000,5253.724312
buff,644096,587008.750000,5266.805307
buff,645120,587329.000000,5272.302236
buff,646144,587582.250000,5278.394982
buff,647168,587516.250000,5287.354009
buff,648192,587584.250000,5295.107212
buff,649216,587649.750000,5302.881180
buff,650240,587395.000000,5313.548804
buff,651264,587542.250000,5320.582818
buff,652288,587704.000000,5327.481862
buff,653312,587867.750000,5334.358961
buff,654336,588072.500000,5340.859843
buff,655360,589122.000000,5339.688553
buff,656384,588708.500000,5351.788194
buff,657408,588919.250000,5358.219145
buff,658432,588825.000000,5367.424277
buff,659456,588860.000000,5375.452230
buff,660480,589149.000000,5381.158247
buff,661504,590387.750000,5378.192891
buff,662528,590141.000000,5388.770480
buff,663552,590296.750000,5395.675311
buff,664576,590431.750000,5402.766365
buff,665600,590560.250000,5409.913722
buff,666624,590805.000000,5415.992079
buff,667648,590703.500000,5425.243629
buff,668672,591075.250000,5430.147177
buff,669696,590620.250000,5442.652534
buff,670720,590768.250000,5449.609047
buff,671744,590883.750000,5456.862200
buff,672768,590975.000000,5464.336732
buff,673792,591260.000000,5470.015898
buff,674816,591100.000000,5479.811876
buff,675840,591412.500000,5485.227316
buff,676864,591409.250000,5493.568455
buff,677888,591951.500000,5496.839521
buff,678912,592124.000000,5503.539124
buff,679936,592466.250000,5508.656063
buff,680960,592625.250000,5515.472046
buff,681984,592532.000000,5524.635294
buff,683008,592561.500000,5532.655091
buff,684032,592878.250000,5537.989629
buff,685056,592943.250000,5545.672035
buff,686080,593072.750000,5552.748799
buff,687104,592914.500000,5562.520734
buff,688128,593062.500000,5569.420424
buff,689152,593269.750000,5575.759762
buff,690176,593347.000000,5583.317688
buff,691200,593264.250000,5592.381473
buff,692224,593970.500000,5594.007110
buff,693248,593803.750000,5603.855483
buff,694272,593951.750000,5610.734542
buff,695296,594026.750000,5618.300523
buff,696320,594152.750000,5625.381688
buff,697344,593922.000000,5635.843091
buff,698368,594761.250000,5636.154676
buff,699392,594854.750000,5643.531635
buff,700416,594879.000000,5651.564100
buff,701440,594879.250000,5659.824242
buff,702464,595216.000000,5664.879976
buff,703488,594899.000000,5676.160827
buff,704512,595224.500000,5681.314529
buff,705536,595380.250000,5688.083876
buff,706560,595373.250000,5696.406414
buff,707584,595179.500000,5706.519126
buff,708608,595349.000000,5713.150438
buff,709632,595120.250000,5723.605607
buff,710656,595553.000000,5727.699802
buff,711680,595946.500000,5732.165555
buff,712704,595668.000000,5743.097162
buff,713728,595714.000000,5750.904629
buff,714752,596185.000000,5754.605701
buff,715776,596256.500000,5762.159071
buff,716800,596549.000000,5767.573158
buff,717824,597110.750000,5770.378778
buff,718848,597232.000000,5777.437244
buff,719872,596648.500000,5791.325378
buff,720896,596881.000000,5797.304320
buff,721920,596992.000000,5804.459691
buff,722944,597744.500000,5805.375374
buff,723968,597501.000000,5815.967505
buff,724992,598071.250000,5818.640505
buff,726016,597760.000000,5829.892934
buff,727040,597660.500000,5839.087576
buff,728064,597692.500000,5846.998582
buff,729088,598133.750000,5850.902745
buff,730112,598238.500000,5858.094389
buff,731136,598442.250000,5864.313223
buff,732160,598682.750000,5870.167463
buff,733184,598640.250000,5878.794819
buff,734208,598771.750000,5885.712544
buff,735232,598752.250000,5894.113300
buff,736256,599190.250000,5898.007853
buff,737280,598922.750000,5908.848846
buff,738304,598756.750000,5918.696031
buff,739328,599716.500000,5917.419981
buff,740352,599809.250000,5924.699561
buff,741376,600118.000000,5929.841798
buff,742400,600074.250000,5938.465115
buff,743424,600259.000000,5944.825817
buff,744448,600870.750000,5946.953484
buff,745472,600625.750000,5957.562759
buff,746496,600924.750000,5962.777869
buff,747520,603406.000000,5946.404245
buff,748544,600697.250000,5981.401113
buff,749568,600655.000000,5990.004911
buff,750592,600833.250000,5996.408488
buff,751616,601930.750000,5993.640963
buff,752640,601089.500000,6010.206467
buff,753664,601834.500000,6010.933571
buff,754688,601866.250000,6018.783077
buff,755712,602529.500000,6020.315354
buff,756736,602359.750000,6030.171837
buff,757760,601554.750000,6046.412234
buff,758784,602100.750000,6049.092614
buff,759808,602266.250000,6055.591526
buff,760832,602218.750000,6064.230979
buff,761856,602685.500000,6067.690031
buff,762880,602812.250000,6074.567994
buff,763904,603006.750000,6080.759792
buff,764928,603075.250000,6088.219339
buff,765952,603182.250000,6095.288116
buff,766976,603345.250000,6101.787990
buff,768000,603653.750000,6106.812059
buff,769024,603826.750000,6113.202504
buff,770048,603786.750000,6121.748117
buff,771072,603820.250000,6129.548653
buff,772096,603894.250000,6136.936724
buff,773120,606222.500000,6121.475201
buff,774144,603694.250000,6155.253591
buff,775168,603422.250000,6166.173687
buff,776192,604425.250000,6164.073390
buff,777216,604407.000000,6172.391782
buff,778240,605001.750000,6174.448256
buff,779264,604549.000000,6187.202692
buff,780288,604537.500000,6195.450903
buff,781312,604793.750000,6200.952970
buff,782336,605034.500000,6206.609375
buff,783360,605169.750000,6213.344272
buff,784384,604899.500000,6224.245846
buff,785408,605036.750000,6230.957706
buff,786432,605621.500000,6233.057446
buff,787456,606500.250000,6232.130655
buff,788480,605913.250000,6246.280305
buff,789504,606027.000000,6253.218421
buff,790528,606151.500000,6260.042910
buff,791552,606677.500000,6262.717177
buff,792576,606693.250000,6270.656217
buff,793600,606376.250000,6282.040235
buff,794624,606628.500000,6287.530507
buff,795648,610061.000000,6260.210700
buff,796672,606834.000000,6301.600767
buff,797696,606691.250000,6311.185137
buff,798720,606700.500000,6319.190441
buff,799744,606820.500000,6326.040732
buff,800768,606969.500000,6332.585739
buff,801792,607095.750000,6339.365084
buff,802816,607129.000000,6347.113711
buff,803840,607284.500000,6353.582217
buff,804864,607390.500000,6360.565732
buff,805888,608239.000000,6359.773707
buff,806912,608324.500000,6366.959739
buff,807936,608306.500000,6375.228277
buff,808960,608252.750000,6383.872494
buff,809984,608491.000000,6389.450625
buff,811008,608621.250000,6396.159188
buff,812032,611631.250000,6372.718202
buff,813056,608295.000000,6415.750253
buff,814080,608428.250000,6422.423679
buff,815104,608639.500000,6428.270265
buff,816128,608760.750000,6435.064021
buff,817152,608856.500000,6442.124869
buff,818176,609095.500000,6447.666745
buff,819200,610204.500000,6444.003609
buff,820224,610374.000000,6450.266886
buff,821248,610496.250000,6457.026395
buff,822272,611029.500000,6459.435428
buff,823296,611119.000000,6466.532377
buff,824320,611483.750000,6470.713245
buff,825344,611413.500000,6479.495791
buff,826368,611468.000000,6486.956636
buff,827392,611483.000000,6494.835670
buff,828416,612169.750000,6495.578718
buff,829440,611079.500000,6515.211196
buff,830464,611206.250000,6521.901895
buff,831488,611849.250000,6523.081298
buff,832512,611450.000000,6535.379181
buff,833536,611729.250000,6540.430754
buff,834560,611636.500000,6549.458706
buff,835584,611921.000000,6554.446080
buff,836608,612125.500000,6560.286085
buff,837632,612114.500000,6568.433847
buff,838656,612132.250000,6576.273019
buff,839680,613030.250000,6574.657613
buff,840704,613008.500000,6582.909046
buff,841728,613120.750000,6589.720540
buff,842752,613211.500000,6596.760824
buff,843776,612759.750000,6609.645624
buff,844800,612588.250000,6619.519718
buff,845824,613457.500000,6618.152358
buff,846848,613832.500000,6622.116620
buff,847872,613835.750000,6630.088912
buff,848896,613768.500000,6638.823596
buff,849920,613917.750000,6645.215910
buff,850944,613814.000000,6654.346757
buff,851968,614087.750000,6659.384428
buff,852992,614310.750000,6664.968178
buff,854016,614486.750000,6671.058082
buff,855040,615208.000000,6671.226642
buff,856064,615470.750000,6676.364718
buff,857088,615392.000000,6685.206178
buff,858112,615687.750000,6689.978159
buff,859136,615133.500000,6703.996450
buff,860160,615002.250000,6713.419341
buff,861184,615123.500000,6720.086617
buff,862208,615384.750000,6725.220929
buff,863232,615703.250000,6729.725075
buff,864256,615770.000000,6736.977768
buff,865280,616080.500000,6741.560559
buff,866304,616200.750000,6748.221582
buff,867328,616370.250000,6754.340269
buff,868352,616341.250000,6762.632876
buff,869376,617277.750000,6760.335684
buff,870400,616694.500000,6774.699628
buff,871424,616640.000000,6783.269331
buff,872448,616894.750000,6788.435791
buff,873472,616554.500000,6800.154082
buff,874496,616440.500000,6809.385172
buff,875520,616759.250000,6813.835382
buff,876544,616685.500000,6822.620606
buff,877568,618153.250000,6814.372326
buff,878592,617762.000000,6826.644565
buff,879616,617080.000000,6842.154664
buff,880640,618143.750000,6838.331699
buff,881664,618292.750000,6844.633388
buff,882688,618789.750000,6847.079157
buff,883712,619917.750000,6842.549032
buff,884736,618013.500000,6871.585815
buff,885760,618945.250000,6869.182694
buff,886784,619118.750000,6875.196721
buff,887808,619113.000000,6883.199674
buff,888832,618803.250000,6894.588223
buff,889856,619319.250000,6896.780295
buff,890880,619095.500000,6907.212215
buff,891904,619155.500000,6914.481419
buff,892928,619812.250000,6915.085012
buff,893952,619910.500000,6921.917922
buff,894976,619987.750000,6928.983355
buff,896000,619623.000000,6940.994766
buff,897024,619392.750000,6951.510492
buff,898048,620002.500000,6952.601643
buff,899072,621665.000000,6941.915019
buff,900096,620296.750000,6965.151438
buff,901120,620503.500000,6970.751978
buff,902144,619883.250000,6985.656089
buff,903168,620399.500000,6987.765786
buff,904192,620116.500000,6998.881017
buff,905216,620686.500000,7000.372652
buff,906240,620819.750000,7006.787397
buff,907264,621065.250000,7011.931838
buff,908288,622136.750000,7007.755771
buff,909312,621131.500000,7027.010544
buff,910336,620859.000000,7038.011529
buff,911360,621619.000000,7037.313853
buff,912384,622287.500000,7037.652532
buff,913408,621247.000000,7057.351424
buff,914432,621965.250000,7057.104235
buff,915456,622354.500000,7060.588137
buff,916480,622207.750000,7070.153016
buff,917504,622241.750000,7077.665875
buff,918528,622613.500000,7081.334407
buff,919552,622293.750000,7092.871494
buff,920576,622605.750000,7097.211678
buff,921600,623358.000000,7096.532009
buff,922624,622830.250000,7110.436913
buff,923648,623415.750000,7111.643233
buff,924672,623844.000000,7114.640199
buff,925696,623786.500000,7123.175638
buff,926720,624022.250000,7128.361208
buff,927744,624095.250000,7135.403130
buff,928768,624214.500000,7141.914198
buff,929792,624658.000000,7144.712146
buff,930816,624739.000000,7151.653410
buff,931840,624915.000000,7157.504621
buff,932864,625665.000000,7156.780705
buff,933888,624412.500000,7179.008108
buff,934912,624570.250000,7185.064610
buff,935936,624606.000000,7192.522646
buff,936960,624776.000000,7198.432718
buff,937984,627644.250000,7173.368035
buff,939008,625059.750000,7210.892079
buff,940032,625682.750000,7211.567843
buff,941056,626317.250000,7212.109837
buff,942080,625837.250000,7225.495127
buff,943104,626045.000000,7230.948574
buff,944128,626254.000000,7236.383959
buff,945152,626119.250000,7245.791596
buff,946176,626106.750000,7253.786675
buff,947200,626156.750000,7261.057235
buff,948224,626287.000000,7267.395300
buff,949248,627178.000000,7264.907889
buff,950272,627527.500000,7268.694360
buff,951296,627706.000000,7274.457788
buff,952320,627775.250000,7281.484894
buff,953344,627836.500000,7288.603323
buff,954368,627279.500000,7302.911063
buff,955392,627502.750000,7308.145821
buff,956416,627425.000000,7316.885365
buff,957440,627830.000000,7319.994266
buff,958464,627582.750000,7330.710094
buff,959488,627738.750000,7336.718340
buff,960512,628057.250000,7340.823786
buff,961536,628099.000000,7348.161357
buff,962560,629132.750000,7343.899996
buff,963584,629274.750000,7350.053693
buff,964608,628553.250000,7366.310492
buff,965632,628618.000000,7373.370791
buff,966656,629192.250000,7374.453198
buff,967680,628928.750000,7385.358039
buff,968704,629372.000000,7387.966417
buff,969728,629463.750000,7394.698106
buff,970752,629386.250000,7403.418171
buff,971776,629585.000000,7408.888077
buff,972800,629790.000000,7414.280951
buff,973824,629729.250000,7422.801466
buff,974848,629336.250000,7435.246897
buff,975872,630002.250000,7435.188684
buff,976896,631531.750000,7424.964461
buff,977920,630079.500000,7449.878944
buff,978944,630455.750000,7453.229192
buff,979968,630411.250000,7461.552122
buff,980992,631757.500000,7453.432053
buff,982016,631170.000000,7468.157232
buff,983040,631766.250000,7468.889008
buff,984064,632004.500000,7473.850582
buff,985088,631210.000000,7491.044819
buff,986112,631422.000000,7496.314034
buff,987136,631536.750000,7502.734876
buff,988160,632223.250000,7502.362496
buff,989184,631621.250000,7517.294898
buff,990208,631850.000000,7522.352457
buff,991232,632468.500000,7522.767695
buff,992256,632769.750000,7526.953999
buff,993280,633830.500000,7522.111984
buff,994304,632767.500000,7542.516327
buff,995328,632788.000000,7550.039508
buff,996352,632446.000000,7561.893980
buff,997376,632752.500000,7565.999028
buff,998400,634175.750000,7556.769555
buff,999424,633548.000000,7572.015380
buff,1000448,632632.000000,7590.748492
buff,1001472,632648.000000,7598.325767
buff,1002496,632819.250000,7604.036698
buff,1003520,633465.250000,7604.041421
buff,1004544,633634.500000,7609.767461
buff,1005568,633583.500000,7618.137783
buff,1006592,633813.500000,7623.128255
buff,1007616,633900.500000,7629.835913
buff,1008640,634785.250000,7626.944703
buff,1009664,634892.500000,7633.398095
buff,1010688,635010.000000,7639.725989
buff,1011712,634744.000000,7650.671137
buff,1012736,634530.750000,7660.988534
buff,1013760,634768.750000,7665.859417
buff,1014784,634914.500000,7671.841169
buff,1015808,635001.000000,7678.536569
buff,1016832,635931.250000,7675.033425
buff,1017856,635997.250000,7681.965292
buff,1018880,637285.000000,7674.155205
buff,1019904,635781.750000,7700.031025
buff,1020928,635995.000000,7705.177556
buff,1021952,636094.250000,7711.702472
buff,1022976,636801.250000,7710.859236
buff,1024000,636101.250000,7727.071752
```

## Przepustowość -- two nodes

```csv
type,msgsize,time,throughput
std,1024,31614.500000,41.459457
std,3072,23552.750000,166.951205
std,5120,12843.750000,510.255961
std,7168,36472.750000,251.558766
std,9216,39170.500000,301.157248
std,11264,15203.250000,948.344597
std,13312,38818.250000,438.952297
std,15360,41411.000000,474.772403
std,17408,30551.000000,729.345684
std,19456,30019.500000,829.583437
std,21504,22306.750000,1233.936813
std,23552,31166.500000,967.274477
std,25600,41833.000000,783.305046
std,27648,42684.500000,829.093465
std,29696,66071.000000,575.303537
std,31744,62294.500000,652.261757
std,33792,54975.250000,786.786054
std,35840,72248.750000,634.961851
std,37888,67411.500000,719.411970
std,39936,66058.500000,773.830468
std,41984,66807.500000,804.393519
std,44032,66591.750000,846.365503
std,46080,57178.250000,1031.553082
std,48128,68626.250000,897.671664
std,50176,58292.000000,1101.785494
std,52224,74177.750000,901.169421
std,54272,58145.750000,1194.724636
std,56320,69646.000000,1035.086006
std,58368,70848.750000,1054.514582
std,60416,57179.000000,1352.462967
std,62464,70271.750000,1137.781826
std,64512,70659.500000,1168.637763
std,66560,70609.500000,1206.591181
std,68608,71595.000000,1226.597388
std,70656,72145.500000,1253.573404
std,72704,91865.750000,1013.012140
std,74752,73274.750000,1305.805342
std,76800,72161.250000,1362.282388
std,78848,48903.250000,2063.777765
std,80896,70074.750000,1477.663210
std,82944,50641.500000,2096.468706
std,84992,72140.750000,1508.020917
std,87040,61338.750000,1816.326547
std,89088,62003.500000,1839.132307
std,91136,61605.250000,1893.573681
std,93184,61993.750000,1923.992661
std,95232,62959.750000,1936.109340
std,97280,75970.000000,1639.046992
std,99328,75105.000000,1692.827908
std,101376,63721.500000,2036.381441
std,103424,76112.750000,1739.297555
std,105472,64482.000000,2093.672033
std,107520,63339.500000,2172.824225
std,109568,75515.000000,1857.207707
std,111616,76798.750000,1860.296945
std,113664,77450.500000,1878.489099
std,115712,64862.250000,2283.475519
std,117760,75282.000000,2002.242236
std,119808,99143.000000,1546.798463
std,121856,123203.000000,1266.005536
std,123904,100207.000000,1582.695021
std,125952,76354.500000,2111.448048
std,128000,103137.000000,1588.566664
std,130048,102719.250000,1620.547658
std,132096,139893.500000,1208.654298
std,134144,151358.500000,1134.421390
std,136192,261523.000000,666.579077
std,138240,167984.750000,1053.352760
std,140288,138574.250000,1295.829781
std,142336,145007.750000,1256.416157
std,144384,150356.250000,1229.157551
std,146432,136937.500000,1368.748225
std,148480,142758.500000,1331.300063
std,150528,150638.750000,1279.058941
std,152576,169682.500000,1150.957111
std,154624,159032.000000,1244.521354
std,156672,225469.250000,889.434635
std,158720,186444.000000,1089.665530
std,160768,200144.750000,1028.171061
std,162816,178387.250000,1168.270042
std,164864,198969.000000,1060.596977
std,166912,189373.000000,1128.182793
std,168960,192365.500000,1124.259808
std,171008,621415.500000,352.244577
std,173056,190512.500000,1162.714678
std,175104,178677.250000,1254.402113
std,177152,201140.000000,1127.346923
std,179200,189573.000000,1209.961334
std,181248,200999.250000,1154.220426
std,183296,181066.000000,1295.764417
std,185344,200746.250000,1181.792038
std,187392,648173.250000,370.058098
std,189440,190180.250000,1275.017779
std,191488,190970.250000,1283.470279
std,193536,200050.750000,1238.316177
std,195584,201201.250000,1244.264238
std,197632,200159.000000,1263.840047
std,199680,201300.000000,1269.698957
std,201728,193617.750000,1333.616572
std,203776,201199.500000,1296.391293
std,205824,204203.500000,1290.157710
std,207872,204130.500000,1303.461070
std,209920,191988.500000,1399.550494
std,211968,191941.000000,1413.554373
std,214016,191468.000000,1430.737669
std,216064,207598.750000,1332.194534
std,218112,537657.250000,519.258989
std,220160,184323.250000,1528.861931
std,222208,227611.250000,1249.614156
std,224256,196118.500000,1463.644072
std,226304,230784.500000,1255.149804
std,228352,208221.000000,1403.751591
std,230400,183387.500000,1608.135778
std,232448,197261.250000,1508.321781
std,234496,676003.250000,444.013960
std,236544,215218.500000,1406.832219
std,238592,196212.000000,1556.468310
std,240640,206553.500000,1491.232054
std,242688,197392.500000,1573.720582
std,244736,666761.500000,469.826287
std,246784,219533.750000,1438.883634
std,248832,234714.000000,1356.991743
std,250880,694997.250000,462.054202
std,252928,207179.000000,1562.647952
std,254976,222422.750000,1467.337671
std,257024,222611.000000,1477.872702
std,259072,726881.500000,456.212134
std,261120,236768.250000,1411.648732
std,263168,217328.500000,1549.980974
std,265216,694490.250000,488.813889
std,267264,237067.500000,1443.040147
std,269312,235895.750000,1461.320774
std,271360,240123.500000,1446.508984
std,273408,236898.750000,1477.265034
std,275456,231022.500000,1526.187622
std,277504,227848.000000,1558.956497
std,279552,498871.500000,717.272003
std,281600,418324.250000,861.647394
std,283648,253613.000000,1431.588444
std,285696,696516.750000,525.028120
std,287744,264615.500000,1391.877347
std,289792,730334.250000,507.895885
std,291840,229791.750000,1625.624941
std,293888,924236.000000,407.013620
std,295936,337586.500000,1122.077097
std,297984,319597.750000,1193.436187
std,300032,310866.000000,1235.390683
std,302080,272022.000000,1421.437972
std,304128,272653.750000,1427.758980
std,306176,289113.250000,1355.542439
std,308224,230775.250000,1709.571195
std,310272,229575.000000,1729.927736
std,312320,682658.000000,585.607434
std,314368,263757.000000,1525.612742
std,316416,253798.000000,1595.806429
std,318464,264762.750000,1539.619603
std,320512,265079.250000,1547.670593
std,322560,272299.000000,1516.262638
std,324608,624860.750000,664.945334
std,326656,256688.750000,1628.897566
std,328704,261361.500000,1609.805270
std,330752,258082.500000,1640.415604
std,332800,268816.750000,1584.663158
std,334848,259541.500000,1651.394632
std,336896,268852.000000,1603.956377
std,338944,260790.750000,1663.587838
std,340992,260703.500000,1674.199848
std,343040,712492.750000,616.274622
std,345088,269138.250000,1641.210939
std,347136,759759.000000,584.835560
std,349184,270852.000000,1650.183569
std,351232,756847.750000,594.012415
std,353280,251737.250000,1796.311035
std,355328,728232.750000,624.552851
std,357376,305765.000000,1496.055075
std,359424,621425.500000,740.334473
std,361472,284159.500000,1628.255117
std,363520,428419.000000,1086.099356
std,365568,546248.000000,856.620143
std,367616,353061.750000,1332.765387
std,369664,571638.750000,827.742906
std,371712,272552.500000,1745.687014
std,373760,261900.250000,1826.698524
std,375808,263497.500000,1825.574208
std,377856,298021.500000,1622.888550
std,379904,691483.500000,703.237489
std,381952,292093.000000,1673.777050
std,384000,731390.750000,672.034750
std,386048,474471.500000,1041.456526
std,388096,613849.250000,809.258755
std,390144,1015612.000000,491.707778
std,392192,294414.500000,1705.098628
std,394240,285135.250000,1769.781884
std,396288,285103.500000,1779.173669
std,398336,284761.250000,1790.517776
std,400384,752437.000000,681.108877
std,402432,691125.000000,745.325317
std,404480,312955.250000,1654.340037
std,406528,228691.500000,2275.361524
std,408576,748703.500000,698.510532
std,410624,330847.500000,1588.643469
std,412672,331631.250000,1592.793683
std,414720,332596.750000,1596.051675
std,416768,308983.500000,1726.509797
std,418816,308465.750000,1737.906007
std,420864,298724.250000,1803.355168
std,422912,330056.000000,1640.107618
std,424960,782458.500000,695.179105
std,427008,331367.500000,1649.438282
std,429056,529824.250000,1036.554442
std,431104,467081.750000,1181.405867
std,433152,432736.500000,1281.229016
std,435200,384106.750000,1450.263501
std,437248,358995.250000,1559.010711
std,439296,353980.250000,1588.503539
std,441344,324158.250000,1742.730040
std,443392,335416.250000,1692.052070
std,445440,449601.000000,1268.153763
std,447488,333309.500000,1718.476791
std,449536,364884.250000,1576.955103
std,451584,1116979.250000,517.491726
std,453632,364450.250000,1593.218718
std,455680,336650.000000,1732.572108
std,457728,1196206.000000,489.791758
std,459776,326446.250000,1802.787687
std,461824,337333.000000,1752.377384
std,463872,314271.750000,1889.308091
std,465920,363283.250000,1641.632528
std,467968,1208954.750000,495.468536
std,470016,339553.500000,1771.798789
std,472064,543008.250000,1112.767476
std,474112,531831.000000,1141.083088
std,476160,834076.000000,730.730533
std,478208,363135.250000,1685.615043
std,480256,1128644.500000,544.660148
std,482304,352479.000000,1751.449363
std,484352,317134.000000,1954.916723
std,486400,316699.250000,1965.877722
std,488448,1303829.000000,479.521041
std,490496,339293.500000,1850.418237
std,492544,363233.750000,1735.676599
std,494592,340195.250000,1860.924748
std,496640,329700.500000,1928.111119
std,498688,342177.250000,1865.467795
std,500736,366225.000000,1750.131968
std,502784,1264155.750000,509.085625
std,504832,370267.500000,1745.184117
std,506880,478267.000000,1356.577811
std,508928,652353.250000,998.581428
std,510976,763589.250000,856.545951
buff,1024,9131.000000,143.546161
buff,3072,35274.750000,111.472370
buff,5120,13734.000000,477.180719
buff,7168,26408.500000,347.427533
buff,9216,29601.500000,398.509535
buff,11264,40011.250000,360.346653
buff,13312,29082.500000,585.897361
buff,15360,66686.500000,294.824290
buff,17408,30457.250000,731.590672
buff,19456,39407.000000,631.960819
buff,21504,42273.500000,651.119969
buff,23552,40940.750000,736.346061
buff,25600,40644.750000,806.204983
buff,27648,40653.500000,870.513978
buff,29696,30705.500000,1237.917637
buff,31744,32303.750000,1257.820532
buff,33792,43749.000000,988.679970
buff,35840,66415.750000,690.727726
buff,37888,66721.500000,726.851764
buff,39936,67315.000000,759.386170
buff,41984,68051.250000,789.691887
buff,44032,57207.750000,985.197985
buff,46080,55096.500000,1070.528981
buff,48128,53940.000000,1142.080831
buff,50176,44814.000000,1433.152140
buff,52224,59852.500000,1116.857608
buff,54272,57767.500000,1202.547453
buff,56320,71344.500000,1010.443692
buff,58368,47370.000000,1577.180494
buff,60416,70902.250000,1090.691480
buff,62464,66770.750000,1197.439298
buff,64512,61068.250000,1352.181535
buff,66560,62519.500000,1362.723630
buff,68608,62128.000000,1413.505022
buff,70656,107574.750000,840.714759
buff,72704,111003.750000,838.360145
buff,74752,85263.750000,1122.195071
buff,76800,87652.750000,1121.516438
buff,78848,69573.750000,1450.625272
buff,80896,66162.750000,1565.032892
buff,82944,65870.750000,1611.767287
buff,84992,76975.000000,1413.312894
buff,87040,75214.000000,1481.256149
buff,89088,77916.750000,1463.518948
buff,91136,84543.500000,1379.811340
buff,93184,100170.250000,1190.727986
buff,95232,99998.000000,1218.993980
buff,97280,101115.250000,1231.450251
buff,99328,58708.000000,2165.630578
buff,101376,56272.750000,2305.934578
buff,103424,89094.750000,1485.864431
buff,105472,89986.500000,1500.271263
buff,107520,91528.750000,1503.632465
buff,109568,92897.500000,1509.696601
buff,111616,91955.250000,1553.673988
buff,113664,91300.250000,1593.532548
buff,115712,92590.750000,1599.634521
buff,117760,91680.750000,1644.105224
buff,119808,82725.000000,1853.783500
buff,121856,102148.500000,1526.950273
buff,123904,102747.500000,1543.561838
buff,125952,83576.250000,1928.999686
buff,128000,104635.000000,1565.824055
buff,130048,84732.000000,1964.564037
buff,132096,663292.500000,254.914506
buff,134144,687747.000000,249.662041
buff,136192,672859.000000,259.082155
buff,138240,684807.000000,258.389882
buff,140288,705685.250000,254.459959
buff,142336,695835.250000,261.829334
buff,144384,697803.250000,264.847606
buff,146432,704795.750000,265.939402
buff,148480,719868.250000,264.012755
buff,150528,707710.250000,272.252437
buff,152576,720278.750000,271.141249
buff,154624,703710.000000,281.250401
buff,156672,703723.500000,284.970105
buff,158720,702818.500000,289.066950
buff,160768,705823.750000,291.550178
buff,162816,729818.500000,285.556587
buff,164864,708775.500000,297.733090
buff,166912,707814.500000,301.840892
buff,168960,734830.750000,294.311037
buff,171008,727748.250000,300.777419
buff,173056,731758.250000,302.711558
buff,175104,724780.750000,309.242650
buff,177152,728844.000000,311.115355
buff,179200,707806.500000,324.065970
buff,181248,731717.500000,317.058756
buff,183296,728855.250000,321.900515
buff,185344,729786.250000,325.081926
buff,187392,733817.000000,326.868633
buff,189440,723841.250000,334.995001
buff,191488,752763.000000,325.606652
buff,193536,737793.500000,335.766146
buff,195584,738756.000000,338.877139
buff,197632,728805.250000,347.100902
buff,199680,738869.000000,345.921131
buff,201728,723466.750000,356.909063
buff,203776,830973.250000,313.888901
buff,205824,811767.500000,324.544552
buff,207872,779787.750000,341.216132
buff,209920,795715.500000,337.680490
buff,211968,727814.750000,372.785850
buff,214016,864857.000000,316.746560
buff,216064,767801.750000,360.199648
buff,218112,726807.750000,384.122706
buff,220160,737796.500000,381.954645
buff,222208,740811.000000,383.939007
buff,224256,734812.750000,390.640582
buff,226304,749717.250000,386.371155
buff,228352,762831.000000,383.165550
buff,230400,743808.250000,396.489283
buff,232448,768763.500000,387.028573
buff,234496,756798.000000,396.611619
buff,236544,739788.250000,409.274302
buff,238592,744769.750000,410.056611
buff,240640,739788.250000,416.361303
buff,242688,740814.250000,419.323251
buff,244736,747802.250000,418.910320
buff,246784,748757.000000,421.877218
buff,248832,750796.500000,424.222755
buff,250880,754820.250000,425.434267
buff,252928,774763.500000,417.866665
buff,254976,742542.500000,439.529428
buff,257024,620064.000000,530.575424
buff,259072,780778.750000,424.719756
buff,261120,776795.250000,430.272456
buff,263168,791743.750000,425.459677
buff,265216,784853.500000,432.534836
buff,267264,786758.750000,434.819339
buff,269312,783753.750000,439.831210
buff,271360,792827.250000,438.104013
buff,273408,796753.500000,439.235272
buff,275456,795829.750000,443.039080
buff,277504,774806.000000,458.443946
buff,279552,780731.500000,458.322176
buff,281600,788835.250000,456.936984
buff,283648,774725.500000,468.642687
buff,285696,789766.750000,463.036561
buff,287744,763916.500000,482.136883
buff,289792,797689.250000,465.010353
buff,291840,795756.750000,469.433907
buff,293888,793827.500000,473.877058
buff,295936,796769.000000,475.417693
buff,297984,798803.750000,477.488394
buff,300032,1225480.250000,313.379967
buff,302080,867932.250000,445.498367
buff,304128,819623.000000,474.954754
buff,306176,820708.750000,477.520533
buff,308224,805843.500000,489.582307
buff,310272,786488.500000,504.963722
buff,312320,798000.500000,500.964097
buff,314368,799773.250000,503.131406
buff,316416,822772.250000,492.253452
buff,318464,802796.250000,507.767594
buff,320512,819811.750000,500.426299
buff,322560,821708.500000,502.461396
buff,324608,806927.250000,514.914126
buff,326656,822741.500000,508.202977
buff,328704,809725.000000,519.609892
buff,330752,804825.500000,526.030251
buff,332800,827784.000000,514.607675
buff,334848,824004.250000,520.149550
buff,336896,822551.250000,524.255334
buff,338944,829819.750000,522.822360
buff,340992,819759.750000,532.436192
buff,343040,805762.750000,544.938569
buff,345088,807750.000000,546.843256
buff,347136,815831.750000,544.639357
buff,349184,885822.750000,504.565411
buff,351232,830797.250000,541.139201
buff,353280,827756.250000,546.294154
buff,355328,818797.500000,555.472922
buff,357376,819712.250000,558.051048
buff,359424,833833.000000,551.744438
buff,361472,839821.750000,550.931385
buff,363520,825717.500000,563.516699
buff,365568,809803.500000,577.827880
buff,367616,834795.500000,563.669162
buff,369664,810804.500000,583.580777
buff,371712,853690.250000,557.334888
buff,373760,839797.250000,569.676550
buff,375808,827807.500000,581.094324
buff,377856,825786.750000,585.690773
buff,379904,834800.500000,582.506982
buff,381952,827795.750000,590.602887
buff,384000,823771.500000,596.670315
buff,386048,863835.000000,572.032205
buff,388096,855817.750000,580.454051
buff,390144,856694.750000,582.919786
buff,392192,887422.000000,565.690010
buff,394240,747016.500000,675.523499
buff,396288,894662.750000,566.971901
buff,398336,867706.250000,587.606785
buff,400384,844797.250000,606.644399
buff,402432,866818.250000,594.257170
buff,404480,860728.500000,601.507212
buff,406528,863849.750000,602.368456
buff,408576,867783.250000,602.658878
buff,410624,884750.500000,594.064338
buff,412672,886400.750000,595.915741
buff,414720,883776.750000,600.651239
buff,416768,873747.000000,610.546348
buff,418816,869829.250000,616.310017
buff,420864,882717.250000,610.281401
buff,422912,872806.750000,620.214452
buff,424960,862750.750000,630.481979
buff,427008,869917.500000,628.301235
buff,429056,885678.250000,620.080351
buff,431104,875597.750000,630.213040
buff,433152,872775.750000,635.254314
buff,435200,870820.000000,639.691325
buff,437248,897771.000000,623.407796
buff,439296,880825.500000,638.377159
buff,441344,931800.750000,606.267295
buff,443392,895764.000000,633.584024
buff,445440,896780.500000,635.789025
buff,447488,914772.000000,626.150166
buff,449536,893813.000000,643.765620
buff,451584,890763.250000,648.912626
buff,453632,900740.000000,644.635477
buff,455680,889784.500000,655.518724
buff,457728,892761.250000,656.269344
buff,459776,896762.750000,656.264190
buff,461824,895858.250000,659.852962
buff,463872,904715.500000,656.290469
buff,465920,893784.500000,667.249880
buff,467968,893794.250000,670.175535
buff,470016,892806.500000,673.853159
buff,472064,886895.000000,681.300402
buff,474112,919710.250000,659.841901
buff,476160,918870.250000,663.298001
buff,478208,899765.250000,680.295488
buff,480256,983842.500000,624.823262
buff,482304,1167863.500000,528.614106
buff,484352,973695.250000,636.719302
buff,486400,917770.750000,678.374202
buff,488448,907758.000000,688.744621
buff,490496,898777.250000,698.543360
buff,492544,904783.000000,696.803897
buff,494592,931833.500000,679.389354
buff,496640,921681.750000,689.716597
buff,498688,904778.000000,705.499736
buff,500736,888781.750000,721.146761
buff,502784,888789.500000,724.089922
buff,504832,929909.500000,694.890159
buff,506880,902673.250000,718.761080
buff,508928,928827.500000,701.344265
buff,510976,919777.500000,711.095107
```

## Opóźnienie -- single node

```csv
type,time
std,0.286200
buff,2.252350
```

## Opóźnienie -- two nodes

```csv
type,time
std,429.878000
buff,483.279650
```

# Dane - Badanie ...

## Ares

```csv
type,series,proc_count,total_point_count,point_count,avg_pi,time
strong,1,10,100000000000,10000000000,3.141596,120032432.075000
strong,1,11,100000000000,9090909090,3.141586,108381701.206000
strong,1,12,100000000000,8333333333,3.141598,108186392.409000
strong,1,1,100000000000,100000000000,3.141586,1199232524.142000
strong,1,2,100000000000,50000000000,3.141590,598072517.305000
strong,1,3,100000000000,33333333333,3.141580,398158405.868000
strong,1,4,100000000000,25000000000,3.141590,299159408.583000
strong,1,5,100000000000,20000000000,3.141595,238963165.912000
strong,1,6,100000000000,16666666666,3.141594,200275781.932000
strong,1,7,100000000000,14285714285,3.141591,170659347.132000
strong,1,8,100000000000,12500000000,3.141590,148989523.788000
strong,1,9,100000000000,11111111111,3.141597,133795462.777000
strong,1,10,10000000,1000000,3.142154,13125.524000
strong,1,11,10000000,909090,3.141962,10879.484000
strong,1,12,10000000,833333,3.141995,10068.013000
strong,1,1,10000000,10000000,3.140603,118934.618000
strong,1,2,10000000,5000000,3.141387,67614.750000
strong,1,3,10000000,3333333,3.141608,39696.003000
strong,1,4,10000000,2500000,3.141625,37657.081000
strong,1,5,10000000,2000000,3.142394,31766.199000
strong,1,6,10000000,1666666,3.141349,20498.349000
strong,1,7,10000000,1428571,3.141158,23675.281000
strong,1,8,10000000,1250000,3.141476,16588.836000
strong,1,9,10000000,1111111,3.142203,14192.696000
strong,1,10,1000,100,3.160000,5266.104000
strong,1,11,1000,90,3.179798,60.518000
strong,1,12,1000,83,3.232932,165.901000
strong,1,1,1000,1000,3.128000,58.725000
strong,1,2,1000,500,3.176000,65.715000
strong,1,3,1000,333,3.027027,54.651000
strong,1,4,1000,250,3.132000,66.892000
strong,1,5,1000,200,3.072000,86.458000
strong,1,6,1000,166,3.132530,234.906000
strong,1,7,1000,142,3.162978,47.993000
strong,1,8,1000,125,3.068000,2006.943000
strong,1,9,1000,111,3.179179,314.999000
strong,2,10,100000000000,10000000000,3.141588,120397876.479000
strong,2,11,100000000000,9090909090,3.141583,124084700.897000
strong,2,12,100000000000,8333333333,3.141596,99126212.685000
strong,2,1,100000000000,100000000000,3.141596,1204143310.642000
strong,2,2,100000000000,50000000000,3.141597,597690686.703000
strong,2,3,100000000000,33333333333,3.141592,398479525.962000
strong,2,4,100000000000,25000000000,3.141595,299934196.050000
strong,2,5,100000000000,20000000000,3.141603,269941496.143000
strong,2,6,100000000000,16666666666,3.141588,233407464.440000
strong,2,7,100000000000,14285714285,3.141590,170520002.564000
strong,2,8,100000000000,12500000000,3.141586,148727723.138000
strong,2,9,100000000000,11111111111,3.141592,133097493.440000
strong,2,10,10000000,1000000,3.141880,11971.957000
strong,2,11,10000000,909090,3.141935,12393.972000
strong,2,12,10000000,833333,3.141197,9952.728000
strong,2,1,10000000,10000000,3.140960,119049.873000
strong,2,2,10000000,5000000,3.141568,59639.096000
strong,2,3,10000000,3333333,3.141037,43331.718000
strong,2,4,10000000,2500000,3.141614,29791.738000
strong,2,5,10000000,2000000,3.141998,24162.275000
strong,2,6,10000000,1666666,3.141175,19912.944000
strong,2,7,10000000,1428571,3.141290,23408.437000
strong,2,8,10000000,1250000,3.142105,14959.338000
strong,2,9,10000000,1111111,3.141466,13478.292000
strong,2,10,1000,100,3.128000,8419.428000
strong,2,11,1000,90,3.094949,48.822000
strong,2,12,1000,83,3.200803,62.578000
strong,2,1,1000,1000,3.132000,46.259000
strong,2,2,1000,500,3.168000,78.149000
strong,2,3,1000,333,3.155155,50.689000
strong,2,4,1000,250,3.220000,355.603000
strong,2,5,1000,200,3.176000,115.738000
strong,2,6,1000,166,3.204819,291.535000
strong,2,7,1000,142,3.187123,55.491000
strong,2,8,1000,125,2.976000,55.381000
strong,2,9,1000,111,3.127127,50.069000
strong,3,10,100000000000,10000000000,3.141593,118957985.628000
strong,3,11,100000000000,9090909090,3.141596,108982260.126000
strong,3,12,100000000000,8333333333,3.141597,100220059.297000
strong,3,1,100000000000,100000000000,3.141590,1196607055.130000
strong,3,2,100000000000,50000000000,3.141592,598461943.964000
strong,3,3,100000000000,33333333333,3.141590,399811133.294000
strong,3,4,100000000000,25000000000,3.141605,298297913.227000
strong,3,5,100000000000,20000000000,3.141591,240241591.521000
strong,3,6,100000000000,16666666666,3.141591,198254993.962000
strong,3,7,100000000000,14285714285,3.141590,171418839.015000
strong,3,8,100000000000,12500000000,3.141598,149998507.895000
strong,3,9,100000000000,11111111111,3.141598,132568392.433000
strong,3,10,10000000,1000000,3.141472,12923.572000
strong,3,11,10000000,909090,3.141993,10995.640000
strong,3,12,10000000,833333,3.140938,10043.246000
strong,3,1,10000000,10000000,3.141481,119866.101000
strong,3,2,10000000,5000000,3.141419,59678.041000
strong,3,3,10000000,3333333,3.141659,39704.815000
strong,3,4,10000000,2500000,3.141955,37455.372000
strong,3,5,10000000,2000000,3.140772,23859.852000
strong,3,6,10000000,1666666,3.141935,21016.689000
strong,3,7,10000000,1428571,3.142093,18154.869000
strong,3,8,10000000,1250000,3.140925,14876.137000
strong,3,9,10000000,1111111,3.140898,13377.504000
strong,3,10,1000,100,3.112000,42.709000
strong,3,11,1000,90,3.123232,5645.243000
strong,3,12,1000,83,3.152610,334.514000
strong,3,1,1000,1000,3.072000,56.647000
strong,3,2,1000,500,3.100000,12363.415000
strong,3,3,1000,333,3.187187,86.939000
strong,3,4,1000,250,3.156000,317.228000
strong,3,5,1000,200,3.128000,62.496000
strong,3,6,1000,166,3.160643,92.437000
strong,3,7,1000,142,3.126761,329.356000
strong,3,8,1000,125,3.168000,570.551000
strong,3,9,1000,111,3.107107,319.761000
strong,4,10,100000000000,10000000000,3.141585,118961722.611000
strong,4,11,100000000000,9090909090,3.141588,108556152.251000
strong,4,12,100000000000,8333333333,3.141590,112432715.684000
strong,4,1,100000000000,100000000000,3.141590,1185622956.498000
strong,4,2,100000000000,50000000000,3.141589,593778619.539000
strong,4,3,100000000000,33333333333,3.141593,396499188.846000
strong,4,4,100000000000,25000000000,3.141586,296666015.137000
strong,4,5,100000000000,20000000000,3.141591,237344804.806000
strong,4,6,100000000000,16666666666,3.141602,198186216.586000
strong,4,7,100000000000,14285714285,3.141595,169562475.173000
strong,4,8,100000000000,12500000000,3.141598,148397953.517000
strong,4,9,100000000000,11111111111,3.141601,131853954.904000
strong,4,10,10000000,1000000,3.141096,12624.975000
strong,4,11,10000000,909090,3.142437,10831.567000
strong,4,12,10000000,833333,3.141568,10844.563000
strong,4,1,10000000,10000000,3.142206,119634.555000
strong,4,2,10000000,5000000,3.141372,59862.435000
strong,4,3,10000000,3333333,3.141597,39590.114000
strong,4,4,10000000,2500000,3.141433,30938.734000
strong,4,5,10000000,2000000,3.141840,30698.556000
strong,4,6,10000000,1666666,3.142453,19888.552000
strong,4,7,10000000,1428571,3.141683,18520.194000
strong,4,8,10000000,1250000,3.140929,14895.268000
strong,4,9,10000000,1111111,3.141404,13236.267000
strong,4,10,1000,100,3.176000,6825.730000
strong,4,11,1000,90,3.127273,37.849000
strong,4,12,1000,83,3.108434,35.355000
strong,4,1,1000,1000,3.088000,44.232000
strong,4,2,1000,500,3.056000,43.280000
strong,4,3,1000,333,3.139139,37.474000
strong,4,4,1000,250,3.136000,350.786000
strong,4,5,1000,200,3.208000,45.344000
strong,4,6,1000,166,3.068273,1430.005000
strong,4,7,1000,142,3.191147,336.132000
strong,4,8,1000,125,3.152000,4658.181000
strong,4,9,1000,111,3.187187,31.480000
strong,5,10,100000000000,10000000000,3.141600,135065570.519000
strong,5,11,100000000000,9090909090,3.141596,123975117.951000
strong,5,12,100000000000,8333333333,3.141596,99103666.458000
strong,5,1,100000000000,100000000000,3.141595,1187090026.470000
strong,5,2,100000000000,50000000000,3.141586,593831010.809000
strong,5,3,100000000000,33333333333,3.141584,395752928.105000
strong,5,4,100000000000,25000000000,3.141592,297468558.392000
strong,5,5,100000000000,20000000000,3.141598,237387781.964000
strong,5,6,100000000000,16666666666,3.141599,197815439.004000
strong,5,7,100000000000,14285714285,3.141597,192771808.058000
strong,5,8,100000000000,12500000000,3.141590,165427328.493000
strong,5,9,100000000000,11111111111,3.141597,132012223.574000
strong,5,10,10000000,1000000,3.141573,11923.282000
strong,5,11,10000000,909090,3.142293,10829.481000
strong,5,12,10000000,833333,3.141118,10649.421000
strong,5,1,10000000,10000000,3.141140,119226.137000
strong,5,2,10000000,5000000,3.141816,66894.691000
strong,5,3,10000000,3333333,3.141563,39851.540000
strong,5,4,10000000,2500000,3.140984,30086.172000
strong,5,5,10000000,2000000,3.142879,23788.990000
strong,5,6,10000000,1666666,3.142011,27183.564000
strong,5,7,10000000,1428571,3.141673,17291.518000
strong,5,8,10000000,1250000,3.141630,15432.820000
strong,5,9,10000000,1111111,3.141594,13247.182000
strong,5,10,1000,100,3.172000,1033.352000
strong,5,11,1000,90,3.135354,33.216000
strong,5,12,1000,83,3.116466,46.349000
strong,5,1,1000,1000,3.224000,43.036000
strong,5,2,1000,500,3.216000,48.638000
strong,5,3,1000,333,3.203203,52.358000
strong,5,4,1000,250,3.168000,315.487000
strong,5,5,1000,200,3.172000,6757.697000
strong,5,6,1000,166,3.140562,5365.961000
strong,5,7,1000,142,3.142857,7777.011000
strong,5,8,1000,125,3.144000,47.806000
strong,5,9,1000,111,3.231231,42.709000
strong,6,10,100000000000,10000000000,3.141605,133124788.078000
strong,6,11,100000000000,9090909090,3.141589,107947157.187000
strong,6,12,100000000000,8333333333,3.141591,99557192.534000
strong,6,1,100000000000,100000000000,3.141590,1186296306.448000
strong,6,2,100000000000,50000000000,3.141588,593721014.033000
strong,6,3,100000000000,33333333333,3.141586,395553316.273000
strong,6,4,100000000000,25000000000,3.141596,297642404.490000
strong,6,5,100000000000,20000000000,3.141587,237998325.934000
strong,6,6,100000000000,16666666666,3.141596,197809293.654000
strong,6,7,100000000000,14285714285,3.141594,169573845.510000
strong,6,8,100000000000,12500000000,3.141602,170350097.121000
strong,6,9,100000000000,11111111111,3.141590,131927805.974000
strong,6,10,10000000,1000000,3.141632,12454.706000
strong,6,11,10000000,909090,3.143062,10920.908000
strong,6,12,10000000,833333,3.141959,18300.696000
strong,6,1,10000000,10000000,3.140898,119264.590000
strong,6,2,10000000,5000000,3.140972,59398.780000
strong,6,3,10000000,3333333,3.141631,40021.826000
strong,6,4,10000000,2500000,3.141474,34102.820000
strong,6,5,10000000,2000000,3.140900,24022.986000
strong,6,6,10000000,1666666,3.142405,20247.822000
strong,6,7,10000000,1428571,3.141658,17025.347000
strong,6,8,10000000,1250000,3.140757,14868.650000
strong,6,9,10000000,1111111,3.142488,13226.551000
strong,6,10,1000,100,3.180000,38.752000
strong,6,11,1000,90,3.179798,50.072000
strong,6,12,1000,83,3.112450,48.777000
strong,6,1,1000,1000,3.180000,42.979000
strong,6,2,1000,500,3.148000,38.284000
strong,6,3,1000,333,3.083083,34.814000
strong,6,4,1000,250,3.144000,77.153000
strong,6,5,1000,200,3.136000,47.073000
strong,6,6,1000,166,3.212851,9412.657000
strong,6,7,1000,142,3.179074,8741.168000
strong,6,8,1000,125,3.136000,32.828000
strong,6,9,1000,111,3.199199,51.591000
strong,7,10,100000000000,10000000000,3.141596,118804972.926000
strong,7,11,100000000000,9090909090,3.141587,108067049.980000
strong,7,12,100000000000,8333333333,3.141586,108484926.931000
strong,7,1,100000000000,100000000000,3.141601,1186060975.654000
strong,7,2,100000000000,50000000000,3.141590,593889916.119000
strong,7,3,100000000000,33333333333,3.141599,395731681.494000
strong,7,4,100000000000,25000000000,3.141597,296849684.780000
strong,7,5,100000000000,20000000000,3.141595,237442040.905000
strong,7,6,100000000000,16666666666,3.141600,198097320.591000
strong,7,7,100000000000,14285714285,3.141583,169672790.212000
strong,7,8,100000000000,12500000000,3.141592,148552073.695000
strong,7,9,100000000000,11111111111,3.141592,132034750.613000
strong,7,10,10000000,1000000,3.141811,11931.008000
strong,7,11,10000000,909090,3.141306,11329.860000
strong,7,12,10000000,833333,3.142034,9997.447000
strong,7,1,10000000,10000000,3.141551,119363.316000
strong,7,2,10000000,5000000,3.141716,60334.872000
strong,7,3,10000000,3333333,3.142307,47344.804000
strong,7,4,10000000,2500000,3.141248,29716.686000
strong,7,5,10000000,2000000,3.141379,23875.666000
strong,7,6,10000000,1666666,3.142167,21292.454000
strong,7,7,10000000,1428571,3.141836,28926.770000
strong,7,8,10000000,1250000,3.140950,14893.255000
strong,7,9,10000000,1111111,3.141828,14446.441000
strong,7,10,1000,100,3.124000,62.913000
strong,7,11,1000,90,3.139394,58.311000
strong,7,12,1000,83,3.168675,241.249000
strong,7,1,1000,1000,3.016000,41.083000
strong,7,2,1000,500,3.108000,369.269000
strong,7,3,1000,333,3.195195,56.616000
strong,7,4,1000,250,3.100000,96.304000
strong,7,5,1000,200,3.136000,8671.871000
strong,7,6,1000,166,3.088353,467.971000
strong,7,7,1000,142,3.110664,247.679000
strong,7,8,1000,125,3.228000,62.444000
strong,7,9,1000,111,3.127127,352.084000
strong,8,10,100000000000,10000000000,3.141583,118846968.991000
strong,8,11,100000000000,9090909090,3.141601,108269821.383000
strong,8,12,100000000000,8333333333,3.141586,99190463.529000
strong,8,1,100000000000,100000000000,3.141597,1186035286.716000
strong,8,2,100000000000,50000000000,3.141591,593776674.016000
strong,8,3,100000000000,33333333333,3.141594,395668049.533000
strong,8,4,100000000000,25000000000,3.141595,296887521.724000
strong,8,5,100000000000,20000000000,3.141598,237605063.986000
strong,8,6,100000000000,16666666666,3.141587,197919604.860000
strong,8,7,100000000000,14285714285,3.141595,169692447.809000
strong,8,8,100000000000,12500000000,3.141586,148562953.481000
strong,8,9,100000000000,11111111111,3.141590,132455457.792000
strong,8,10,10000000,1000000,3.141734,11964.345000
strong,8,11,10000000,909090,3.141426,18851.487000
strong,8,12,10000000,833333,3.140520,10005.048000
strong,8,1,10000000,10000000,3.141184,119167.360000
strong,8,2,10000000,5000000,3.140564,60182.473000
strong,8,3,10000000,3333333,3.140978,39653.461000
strong,8,4,10000000,2500000,3.141856,34527.604000
strong,8,5,10000000,2000000,3.141355,32074.653000
strong,8,6,10000000,1666666,3.142515,20160.791000
strong,8,7,10000000,1428571,3.141809,17040.781000
strong,8,8,10000000,1250000,3.141338,15263.484000
strong,8,9,10000000,1111111,3.141735,14044.934000
strong,8,10,1000,100,3.160000,48.263000
strong,8,11,1000,90,3.179798,44.130000
strong,8,12,1000,83,3.160643,4654.326000
strong,8,1,1000,1000,3.184000,40.773000
strong,8,2,1000,500,3.204000,62.047000
strong,8,3,1000,333,3.079079,219.584000
strong,8,4,1000,250,3.140000,9035.421000
strong,8,5,1000,200,3.240000,357.441000
strong,8,6,1000,166,3.088353,1182.108000
strong,8,7,1000,142,3.158954,1713.054000
strong,8,8,1000,125,3.184000,127.279000
strong,8,9,1000,111,3.139139,47.957000
strong,9,10,100000000000,10000000000,3.141600,119286699.942000
strong,9,11,100000000000,9090909090,3.141596,123455788.549000
strong,9,12,100000000000,8333333333,3.141595,100783211.409000
strong,9,1,100000000000,100000000000,3.141592,1186535794.725000
strong,9,2,100000000000,50000000000,3.141598,594634383.402000
strong,9,3,100000000000,33333333333,3.141600,396121007.262000
strong,9,4,100000000000,25000000000,3.141594,297035076.836000
strong,9,5,100000000000,20000000000,3.141591,237706274.233000
strong,9,6,100000000000,16666666666,3.141597,199442079.811000
strong,9,7,100000000000,14285714285,3.141594,169901047.164000
strong,9,8,100000000000,12500000000,3.141603,149449860.852000
strong,9,9,100000000000,11111111111,3.141585,144610742.831000
strong,9,10,10000000,1000000,3.141240,11921.266000
strong,9,11,10000000,909090,3.141504,12521.513000
strong,9,12,10000000,833333,3.141213,10906.328000
strong,9,1,10000000,10000000,3.141844,119506.919000
strong,9,2,10000000,5000000,3.142286,59461.262000
strong,9,3,10000000,3333333,3.140965,39983.585000
strong,9,4,10000000,2500000,3.141435,37776.946000
strong,9,5,10000000,2000000,3.141447,31884.894000
strong,9,6,10000000,1666666,3.141576,19920.245000
strong,9,7,10000000,1428571,3.141397,17899.101000
strong,9,8,10000000,1250000,3.141934,15491.120000
strong,9,9,10000000,1111111,3.140985,13340.166000
strong,9,10,1000,100,3.224000,36.293000
strong,9,11,1000,90,3.054545,1570.317000
strong,9,12,1000,83,3.080321,7565.476000
strong,9,1,1000,1000,3.208000,48.468000
strong,9,2,1000,500,3.172000,84.164000
strong,9,3,1000,333,3.183183,50.714000
strong,9,4,1000,250,3.068000,692.657000
strong,9,5,1000,200,3.172000,57.277000
strong,9,6,1000,166,3.152610,108.586000
strong,9,7,1000,142,3.150905,56.538000
strong,9,8,1000,125,3.128000,55.913000
strong,9,9,1000,111,3.191191,48.319000
strong,10,10,100000000000,10000000000,3.141596,120144411.498000
strong,10,11,100000000000,9090909090,3.141580,108286658.730000
strong,10,12,100000000000,8333333333,3.141589,100343517.727000
strong,10,1,100000000000,100000000000,3.141592,1242802145.759000
strong,10,2,100000000000,50000000000,3.141589,595305506.155000
strong,10,3,100000000000,33333333333,3.141592,396110101.733000
strong,10,4,100000000000,25000000000,3.141593,297547590.383000
strong,10,5,100000000000,20000000000,3.141592,258849931.584000
strong,10,6,100000000000,16666666666,3.141592,198241661.152000
strong,10,7,100000000000,14285714285,3.141582,184991458.410000
strong,10,8,100000000000,12500000000,3.141600,148648205.733000
strong,10,9,100000000000,11111111111,3.141593,132152245.254000
strong,10,10,10000000,1000000,3.140870,11987.043000
strong,10,11,10000000,909090,3.141112,17491.251000
strong,10,12,10000000,833333,3.141236,10151.213000
strong,10,1,10000000,10000000,3.141243,119724.291000
strong,10,2,10000000,5000000,3.141707,59516.334000
strong,10,3,10000000,3333333,3.140426,39930.833000
strong,10,4,10000000,2500000,3.141824,30083.156000
strong,10,5,10000000,2000000,3.141662,23837.517000
strong,10,6,10000000,1666666,3.141418,19955.938000
strong,10,7,10000000,1428571,3.141383,17537.320000
strong,10,8,10000000,1250000,3.141524,15753.999000
strong,10,9,10000000,1111111,3.141808,13644.080000
strong,10,10,1000,100,3.164000,46.943000
strong,10,11,1000,90,3.175758,74.670000
strong,10,12,1000,83,3.120482,59.900000
strong,10,1,1000,1000,3.148000,49.358000
strong,10,2,1000,500,3.120000,69.552000
strong,10,3,1000,333,3.155155,44.610000
strong,10,4,1000,250,3.172000,8114.365000
strong,10,5,1000,200,3.112000,8140.027000
strong,10,6,1000,166,3.184739,80.213000
strong,10,7,1000,142,3.126761,745.176000
strong,10,8,1000,125,3.108000,49.835000
strong,10,9,1000,111,3.095095,42.254000
weak,1,10,1000000000000,100000000000,3.141594,1223976927.108000
weak,1,1,100000000000,100000000000,3.141586,1187653869.944000
weak,1,10,100000000,10000000,3.141642,135139.665000
weak,1,1,10000000,10000000,3.141171,119492.368000
weak,1,10,10000,1000,3.132800,50.754000
weak,1,1,1000,1000,3.160000,45.521000
weak,1,11,1100000000000,100000000000,3.141593,1240205188.714000
weak,1,11,110000000,10000000,3.141564,119585.050000
weak,1,11,11000,1000,3.133818,5112.541000
weak,1,12,1200000000000,100000000000,3.141593,1256177171.395000
weak,1,12,120000000,10000000,3.141690,121749.703000
weak,1,12,12000,1000,3.162000,168.034000
weak,1,2,200000000000,100000000000,3.141589,1188793789.295000
weak,1,2,20000000,10000000,3.141737,119681.198000
weak,1,2,2000,1000,3.152000,122.451000
weak,1,3,300000000000,100000000000,3.141597,1363969587.947000
weak,1,3,30000000,10000000,3.141288,119224.898000
weak,1,3,3000,1000,3.165333,51.646000
weak,1,4,400000000000,100000000000,3.141592,1187787897.684000
weak,1,4,40000000,10000000,3.141336,119144.312000
weak,1,4,4000,1000,3.123000,8470.300000
weak,1,5,500000000000,100000000000,3.141590,1188026874.879000
weak,1,5,50000000,10000000,3.141801,119376.374000
weak,1,5,5000,1000,3.141600,62.026000
weak,1,6,600000000000,100000000000,3.141592,1187958907.616000
weak,1,6,60000000,10000000,3.141704,123887.638000
weak,1,6,6000,1000,3.142667,291.043000
weak,1,7,700000000000,100000000000,3.141595,1189662110.992000
weak,1,7,70000000,10000000,3.141423,127056.853000
weak,1,7,7000,1000,3.133143,289.840000
weak,1,8,800000000000,100000000000,3.141591,1198468443.609000
weak,1,8,80000000,10000000,3.141214,119750.173000
weak,1,8,8000,1000,3.132500,91.732000
weak,1,9,900000000000,100000000000,3.141595,1209956604.501000
weak,1,9,90000000,10000000,3.141626,119675.974000
weak,1,9,9000,1000,3.135556,6162.416000
weak,2,10,1000000000000,100000000000,3.141591,1223341402.608000
weak,2,1,100000000000,100000000000,3.141577,1187801364.129000
weak,2,10,100000000,10000000,3.141798,119539.169000
weak,2,1,10000000,10000000,3.141456,119280.541000
weak,2,10,10000,1000,3.124000,45.439000
weak,2,1,1000,1000,3.080000,51.878000
weak,2,11,1100000000000,100000000000,3.141595,1247712972.731000
weak,2,11,110000000,10000000,3.141782,121977.670000
weak,2,11,11000,1000,3.102909,55.086000
weak,2,12,1200000000000,100000000000,3.141591,1256333963.246000
weak,2,12,120000000,10000000,3.141629,122085.979000
weak,2,12,12000,1000,3.130333,1033.328000
weak,2,2,200000000000,100000000000,3.141595,1188124779.123000
weak,2,2,20000000,10000000,3.141026,126783.498000
weak,2,2,2000,1000,3.128000,57.528000
weak,2,3,300000000000,100000000000,3.141598,1188976959.119000
weak,2,3,30000000,10000000,3.141371,118790.669000
weak,2,3,3000,1000,3.157333,1072.814000
weak,2,4,400000000000,100000000000,3.141599,1187976922.589000
weak,2,4,40000000,10000000,3.141767,122918.196000
weak,2,4,4000,1000,3.182000,306.485000
weak,2,5,500000000000,100000000000,3.141593,1322594308.518000
weak,2,5,50000000,10000000,3.141752,119601.898000
weak,2,5,5000,1000,3.092800,93.401000
weak,2,6,600000000000,100000000000,3.141595,1188089717.521000
weak,2,6,60000000,10000000,3.141299,119729.586000
weak,2,6,6000,1000,3.110000,276.693000
weak,2,7,700000000000,100000000000,3.141592,1189690594.272000
weak,2,7,70000000,10000000,3.141377,120468.890000
weak,2,7,7000,1000,3.142857,568.987000
weak,2,8,800000000000,100000000000,3.141590,1202291447.938000
weak,2,8,80000000,10000000,3.141380,128017.610000
weak,2,8,8000,1000,3.155500,223.755000
weak,2,9,900000000000,100000000000,3.141595,1210901601.214000
weak,2,9,90000000,10000000,3.141455,119033.011000
weak,2,9,9000,1000,3.149333,317.813000
weak,3,10,1000000000000,100000000000,3.141596,1225284369.881000
weak,3,1,100000000000,100000000000,3.141588,1187705188.051000
weak,3,10,100000000,10000000,3.141574,119418.147000
weak,3,1,10000000,10000000,3.141177,119373.441000
weak,3,10,10000,1000,3.183600,51.868000
weak,3,1,1000,1000,3.180000,55.820000
weak,3,11,1100000000000,100000000000,3.141594,1240165978.275000
weak,3,11,110000000,10000000,3.141490,121288.702000
weak,3,11,11000,1000,3.172727,60.760000
weak,3,12,1200000000000,100000000000,3.141598,1261501054.038000
weak,3,12,120000000,10000000,3.141753,121862.349000
weak,3,12,12000,1000,3.133000,59.982000
weak,3,2,200000000000,100000000000,3.141581,1187860312.211000
weak,3,2,20000000,10000000,3.141786,119672.440000
weak,3,2,2000,1000,3.140000,92.440000
weak,3,3,300000000000,100000000000,3.141593,1188188544.249000
weak,3,3,30000000,10000000,3.141625,119761.454000
weak,3,3,3000,1000,3.186667,85.815000
weak,3,4,400000000000,100000000000,3.141590,1294996557.636000
weak,3,4,40000000,10000000,3.141974,119620.742000
weak,3,4,4000,1000,3.168000,383.432000
weak,3,5,500000000000,100000000000,3.141595,1188362614.865000
weak,3,5,50000000,10000000,3.141384,119652.186000
weak,3,5,5000,1000,3.157600,95.796000
weak,3,6,600000000000,100000000000,3.141596,1188315188.756000
weak,3,6,60000000,10000000,3.141499,129384.735000
weak,3,6,6000,1000,3.211333,5738.164000
weak,3,7,700000000000,100000000000,3.141595,1190305825.320000
weak,3,7,70000000,10000000,3.141293,119741.061000
weak,3,7,7000,1000,3.125143,8569.482000
weak,3,8,800000000000,100000000000,3.141593,1198748060.192000
weak,3,8,80000000,10000000,3.141311,120284.648000
weak,3,8,8000,1000,3.107000,5114.252000
weak,3,9,900000000000,100000000000,3.141596,1321595968.485000
weak,3,9,90000000,10000000,3.141639,135044.826000
weak,3,9,9000,1000,3.130222,1341.519000
weak,4,10,1000000000000,100000000000,3.141593,1294424825.597000
weak,4,1,100000000000,100000000000,3.141599,1187250936.063000
weak,4,10,100000000,10000000,3.141599,119397.056000
weak,4,1,10000000,10000000,3.141903,119599.869000
weak,4,10,10000,1000,3.162000,3430.739000
weak,4,1,1000,1000,3.120000,44.515000
weak,4,11,1100000000000,100000000000,3.141594,1188736005.734000
weak,4,11,110000000,10000000,3.141504,119290.246000
weak,4,11,11000,1000,3.152727,359.061000
weak,4,12,1200000000000,100000000000,3.141594,1204801324.286000
weak,4,12,120000000,10000000,3.141576,120749.133000
weak,4,12,12000,1000,3.145667,46.748000
weak,4,2,200000000000,100000000000,3.141592,1188594509.839000
weak,4,2,20000000,10000000,3.141001,133070.191000
weak,4,2,2000,1000,3.116000,7691.522000
weak,4,3,300000000000,100000000000,3.141597,1188001697.587000
weak,4,3,30000000,10000000,3.141176,118812.904000
weak,4,3,3000,1000,3.177333,8297.356000
weak,4,4,400000000000,100000000000,3.141594,1242887879.218000
weak,4,4,40000000,10000000,3.141962,119700.285000
weak,4,4,4000,1000,3.156000,63.009000
weak,4,5,500000000000,100000000000,3.141591,1188186707.976000
weak,4,5,50000000,10000000,3.141646,135022.479000
weak,4,5,5000,1000,3.091200,8051.438000
weak,4,6,600000000000,100000000000,3.141592,1201379326.953000
weak,4,6,60000000,10000000,3.141914,118798.252000
weak,4,6,6000,1000,3.148667,83.265000
weak,4,7,700000000000,100000000000,3.141590,1189322294.284000
weak,4,7,70000000,10000000,3.141338,119319.522000
weak,4,7,7000,1000,3.142286,398.098000
weak,4,8,800000000000,100000000000,3.141596,1193775648.396000
weak,4,8,80000000,10000000,3.141508,119700.212000
weak,4,8,8000,1000,3.124000,7859.200000
weak,4,9,900000000000,100000000000,3.141594,1192137048.503000
weak,4,9,90000000,10000000,3.141433,118909.687000
weak,4,9,9000,1000,3.143111,60.299000
weak,5,10,1000000000000,100000000000,3.141592,1324136677.575000
weak,5,1,100000000000,100000000000,3.141603,1187939219.852000
weak,5,10,100000000,10000000,3.141320,120224.476000
weak,5,1,10000000,10000000,3.142370,119869.877000
weak,5,10,10000,1000,3.128000,269.664000
weak,5,1,1000,1000,3.080000,52.633000
weak,5,11,1100000000000,100000000000,3.141590,1190491268.115000
weak,5,11,110000000,10000000,3.141709,119296.229000
weak,5,11,11000,1000,3.144727,57.194000
weak,5,12,1200000000000,100000000000,3.141592,1293766233.400000
weak,5,12,120000000,10000000,3.141647,119700.222000
weak,5,12,12000,1000,3.167333,329.781000
weak,5,2,200000000000,100000000000,3.141591,1196976670.525000
weak,5,2,20000000,10000000,3.141519,119719.264000
weak,5,2,2000,1000,3.206000,51.276000
weak,5,3,300000000000,100000000000,3.141592,1188448558.620000
weak,5,3,30000000,10000000,3.141463,119077.279000
weak,5,3,3000,1000,3.089333,49.722000
weak,5,4,400000000000,100000000000,3.141594,1331792548.028000
weak,5,4,40000000,10000000,3.141714,119167.855000
weak,5,4,4000,1000,3.188000,72.992000
weak,5,5,500000000000,100000000000,3.141590,1187973876.451000
weak,5,5,50000000,10000000,3.141458,119679.274000
weak,5,5,5000,1000,3.128000,8110.125000
weak,5,6,600000000000,100000000000,3.141595,1188039832.321000
weak,5,6,60000000,10000000,3.141470,119950.402000
weak,5,6,6000,1000,3.132000,67.823000
weak,5,7,700000000000,100000000000,3.141591,1190197527.533000
weak,5,7,70000000,10000000,3.141701,126749.614000
weak,5,7,7000,1000,3.144000,10008.387000
weak,5,8,800000000000,100000000000,3.141589,1190125751.625000
weak,5,8,80000000,10000000,3.141933,119315.484000
weak,5,8,8000,1000,3.152500,81.408000
weak,5,9,900000000000,100000000000,3.141591,1188977873.668000
weak,5,9,90000000,10000000,3.141509,119687.559000
weak,5,9,9000,1000,3.104889,6798.941000
weak,6,10,1000000000000,100000000000,3.141590,1191560938.303000
weak,6,1,100000000000,100000000000,3.141591,1187477037.606000
weak,6,10,100000000,10000000,3.141633,124570.402000
weak,6,1,10000000,10000000,3.141547,119331.238000
weak,6,10,10000,1000,3.158000,346.804000
weak,6,1,1000,1000,3.208000,43.756000
weak,6,11,1100000000000,100000000000,3.141592,1197296384.728000
weak,6,11,110000000,10000000,3.141161,122573.059000
weak,6,11,11000,1000,3.121818,77.740000
weak,6,12,1200000000000,100000000000,3.141591,1203083138.754000
weak,6,12,120000000,10000000,3.141633,132211.916000
weak,6,12,12000,1000,3.147667,60.386000
weak,6,2,200000000000,100000000000,3.141593,1188026103.829000
weak,6,2,20000000,10000000,3.141345,127110.041000
weak,6,2,2000,1000,3.182000,69.683000
weak,6,3,300000000000,100000000000,3.141597,1187847019.841000
weak,6,3,30000000,10000000,3.141323,118928.570000
weak,6,3,3000,1000,3.126667,44.296000
weak,6,4,400000000000,100000000000,3.141590,1188079080.035000
weak,6,4,40000000,10000000,3.141202,123460.325000
weak,6,4,4000,1000,3.146000,73.119000
weak,6,5,500000000000,100000000000,3.141592,1195027105.328000
weak,6,5,50000000,10000000,3.141615,119758.547000
weak,6,5,5000,1000,3.140800,388.828000
weak,6,6,600000000000,100000000000,3.141594,1189161903.872000
weak,6,6,60000000,10000000,3.141777,119913.784000
weak,6,6,6000,1000,3.149333,538.420000
weak,6,7,700000000000,100000000000,3.141591,1190284359.835000
weak,6,7,70000000,10000000,3.141610,119791.827000
weak,6,7,7000,1000,3.111429,286.213000
weak,6,8,800000000000,100000000000,3.141593,1203098677.919000
weak,6,8,80000000,10000000,3.141936,119913.582000
weak,6,8,8000,1000,3.147000,52.584000
weak,6,9,900000000000,100000000000,3.141590,1193648221.465000
weak,6,9,90000000,10000000,3.141205,120888.855000
weak,6,9,9000,1000,3.142667,79.522000
weak,7,10,1000000000000,100000000000,3.141591,1195249782.457000
weak,7,1,100000000000,100000000000,3.141595,1197024075.275000
weak,7,10,100000000,10000000,3.141639,119644.895000
weak,7,1,10000000,10000000,3.141590,121301.849000
weak,7,10,10000,1000,3.138400,335.193000
weak,7,1,1000,1000,3.064000,50.533000
weak,7,11,1100000000000,100000000000,3.141594,1328210147.295000
weak,7,11,110000000,10000000,3.141699,119862.742000
weak,7,11,11000,1000,3.127636,5848.201000
weak,7,12,1200000000000,100000000000,3.141592,1198784871.109000
weak,7,12,120000000,10000000,3.141353,135224.650000
weak,7,12,12000,1000,3.142000,2534.937000
weak,7,2,200000000000,100000000000,3.141591,1195408648.644000
weak,7,2,20000000,10000000,3.141263,121567.514000
weak,7,2,2000,1000,3.096000,119.645000
weak,7,3,300000000000,100000000000,3.141592,1197064015.194000
weak,7,3,30000000,10000000,3.141489,121572.489000
weak,7,3,3000,1000,3.124000,73.699000
weak,7,4,400000000000,100000000000,3.141590,1196297364.729000
weak,7,4,40000000,10000000,3.141972,121074.904000
weak,7,4,4000,1000,3.123000,427.205000
weak,7,5,500000000000,100000000000,3.141593,1196274080.281000
weak,7,5,50000000,10000000,3.141563,127624.190000
weak,7,5,5000,1000,3.132000,242.619000
weak,7,6,600000000000,100000000000,3.141593,1301843824.074000
weak,7,6,60000000,10000000,3.141492,121394.106000
weak,7,6,6000,1000,3.138667,386.661000
weak,7,7,700000000000,100000000000,3.141592,1196345678.621000
weak,7,7,70000000,10000000,3.141332,120510.207000
weak,7,7,7000,1000,3.149143,109.178000
weak,7,8,800000000000,100000000000,3.141598,1196188557.322000
weak,7,8,80000000,10000000,3.141383,120873.316000
weak,7,8,8000,1000,3.131000,458.570000
weak,7,9,900000000000,100000000000,3.141593,1196142000.061000
weak,7,9,90000000,10000000,3.141472,121813.836000
weak,7,9,9000,1000,3.106667,127.302000
weak,8,10,1000000000000,100000000000,3.141594,1200207946.027000
weak,8,1,100000000000,100000000000,3.141601,1196614870.422000
weak,8,10,100000000,10000000,3.141360,119904.335000
weak,8,1,10000000,10000000,3.141348,121371.738000
weak,8,10,10000,1000,3.156400,56.283000
weak,8,1,1000,1000,3.184000,46.371000
weak,8,11,1100000000000,100000000000,3.141591,1196320470.323000
weak,8,11,110000000,10000000,3.141345,119844.930000
weak,8,11,11000,1000,3.132727,63.518000
weak,8,12,1200000000000,100000000000,3.141594,1196436286.648000
weak,8,12,120000000,10000000,3.141891,119622.767000
weak,8,12,12000,1000,3.165333,172.497000
weak,8,2,200000000000,100000000000,3.141590,1197057618.273000
weak,8,2,20000000,10000000,3.141354,122317.165000
weak,8,2,2000,1000,3.132000,20693.780000
weak,8,3,300000000000,100000000000,3.141596,1197014257.758000
weak,8,3,30000000,10000000,3.141935,119169.864000
weak,8,3,3000,1000,3.153333,53.040000
weak,8,4,400000000000,100000000000,3.141591,1195723127.791000
weak,8,4,40000000,10000000,3.141694,129900.006000
weak,8,4,4000,1000,3.120000,616.762000
weak,8,5,500000000000,100000000000,3.141591,1198571774.778000
weak,8,5,50000000,10000000,3.141783,119745.532000
weak,8,5,5000,1000,3.136000,104.333000
weak,8,6,600000000000,100000000000,3.141590,1197132931.917000
weak,8,6,60000000,10000000,3.141551,120103.369000
weak,8,6,6000,1000,3.185333,289.699000
weak,8,7,700000000000,100000000000,3.141591,1198478034.410000
weak,8,7,70000000,10000000,3.141710,120211.896000
weak,8,7,7000,1000,3.133714,291.172000
weak,8,8,800000000000,100000000000,3.141591,1363480782.265000
weak,8,8,80000000,10000000,3.141459,119131.474000
weak,8,8,8000,1000,3.102500,70.512000
weak,8,9,900000000000,100000000000,3.141593,1195864888.844000
weak,8,9,90000000,10000000,3.141690,120066.475000
weak,8,9,9000,1000,3.160444,272.378000
weak,9,10,1000000000000,100000000000,3.141593,1196480171.757000
weak,9,1,100000000000,100000000000,3.141601,1196633014.148000
weak,9,10,100000000,10000000,3.141460,121128.279000
weak,9,1,10000000,10000000,3.141004,118963.072000
weak,9,10,10000,1000,3.158400,73.056000
weak,9,1,1000,1000,3.196000,53.798000
weak,9,11,1100000000000,100000000000,3.141592,1352256320.892000
weak,9,11,110000000,10000000,3.141302,129871.489000
weak,9,11,11000,1000,3.146909,57.774000
weak,9,12,1200000000000,100000000000,3.141592,1350058077.764000
weak,9,12,120000000,10000000,3.141566,120956.456000
weak,9,12,12000,1000,3.110667,8067.953000
weak,9,2,200000000000,100000000000,3.141592,1195526696.355000
weak,9,2,20000000,10000000,3.141238,119267.353000
weak,9,2,2000,1000,3.156000,94.400000
weak,9,3,300000000000,100000000000,3.141591,1202726505.432000
weak,9,3,30000000,10000000,3.141396,119719.305000
weak,9,3,3000,1000,3.132000,55.781000
weak,9,4,400000000000,100000000000,3.141590,1196612806.316000
weak,9,4,40000000,10000000,3.141626,121602.682000
weak,9,4,4000,1000,3.146000,400.651000
weak,9,5,500000000000,100000000000,3.141591,1195874410.332000
weak,9,5,50000000,10000000,3.141260,121007.412000
weak,9,5,5000,1000,3.144000,65.089000
weak,9,6,600000000000,100000000000,3.141591,1197316390.764000
weak,9,6,60000000,10000000,3.141211,121516.617000
weak,9,6,6000,1000,3.142667,7514.993000
weak,9,7,700000000000,100000000000,3.141591,1197431839.314000
weak,9,7,70000000,10000000,3.141470,120345.018000
weak,9,7,7000,1000,3.121143,285.195000
weak,9,8,800000000000,100000000000,3.141592,1196397384.918000
weak,9,8,80000000,10000000,3.141789,121166.823000
weak,9,8,8000,1000,3.120500,111.221000
weak,9,9,900000000000,100000000000,3.141592,1195966277.929000
weak,9,9,90000000,10000000,3.141481,124627.231000
weak,9,9,9000,1000,3.116444,77.323000
weak,10,10,1000000000000,100000000000,3.141592,1194803961.320000
weak,10,1,100000000000,100000000000,3.141605,1197124167.440000
weak,10,10,100000000,10000000,3.141647,140548.515000
weak,10,1,10000000,10000000,3.141844,121847.730000
weak,10,10,10000,1000,3.189200,81.058000
weak,10,1,1000,1000,3.076000,38.608000
weak,10,11,1100000000000,100000000000,3.141594,1194700882.011000
weak,10,11,110000000,10000000,3.141947,135032.206000
weak,10,11,11000,1000,3.145091,63.861000
weak,10,12,1200000000000,100000000000,3.141592,1353611574.359000
weak,10,12,120000000,10000000,3.141677,120774.686000
weak,10,12,12000,1000,3.154000,716.121000
weak,10,2,200000000000,100000000000,3.141597,1196222915.396000
weak,10,2,20000000,10000000,3.141766,121135.430000
weak,10,2,2000,1000,3.120000,57.016000
weak,10,3,300000000000,100000000000,3.141595,1196388898.007000
weak,10,3,30000000,10000000,3.141582,120464.318000
weak,10,3,3000,1000,3.201333,53.378000
weak,10,4,400000000000,100000000000,3.141597,1196151365.712000
weak,10,4,40000000,10000000,3.141533,121280.470000
weak,10,4,4000,1000,3.148000,290.859000
weak,10,5,500000000000,100000000000,3.141593,1196589972.366000
weak,10,5,50000000,10000000,3.141196,120327.042000
weak,10,5,5000,1000,3.186400,64.085000
weak,10,6,600000000000,100000000000,3.141590,1242840538.143000
weak,10,6,60000000,10000000,3.141889,129413.930000
weak,10,6,6000,1000,3.094667,8393.935000
weak,10,7,700000000000,100000000000,3.141593,1299207862.936000
weak,10,7,70000000,10000000,3.141569,121109.170000
weak,10,7,7000,1000,3.171429,325.495000
weak,10,8,800000000000,100000000000,3.141591,1196819965.534000
weak,10,8,80000000,10000000,3.142061,121885.953000
weak,10,8,8000,1000,3.144000,58.953000
weak,10,9,900000000000,100000000000,3.141593,1196375807.901000
weak,10,9,90000000,10000000,3.141465,121127.924000
weak,10,9,9000,1000,3.134222,316.920000
```

## vCluster

```csv
type,series,proc_count,total_point_count,point_count,avg_pi,time
strong,1,10,1000000000,100000000,3.141617,2144333.000000
strong,1,11,1000000000,90909090,3.141510,2027422.500000
strong,1,12,1000000000,83333333,3.141611,1791207.000000
strong,1,1,1000000000,1000000000,3.141680,21182541.250000
strong,1,2,1000000000,500000000,3.141540,10673262.500000
strong,1,3,1000000000,333333333,3.141665,7120913.250000
strong,1,4,1000000000,250000000,3.141606,5346342.000000
strong,1,5,1000000000,200000000,3.141718,4285068.500000
strong,1,6,1000000000,166666666,3.141590,3579986.750000
strong,1,7,1000000000,142857142,3.141631,3093363.000000
strong,1,8,1000000000,125000000,3.141747,2913791.750000
strong,1,9,1000000000,111111111,3.141598,3149963.500000
strong,2,10,1000000000,100000000,3.141573,2143063.750000
strong,2,11,1000000000,90909090,3.141560,1963712.500000
strong,2,12,1000000000,83333333,3.141595,2677981.000000
strong,2,1,1000000000,1000000000,3.141554,21307038.000000
strong,2,2,1000000000,500000000,3.141557,10683793.250000
strong,2,3,1000000000,333333333,3.141538,7124629.250000
strong,2,4,1000000000,250000000,3.141652,5406593.500000
strong,2,5,1000000000,200000000,3.141595,4281785.250000
strong,2,6,1000000000,166666666,3.141458,3800419.000000
strong,2,7,1000000000,142857142,3.141652,3945960.750000
strong,2,8,1000000000,125000000,3.141534,2909761.500000
strong,2,9,1000000000,111111111,3.141641,2815980.500000
strong,3,10,1000000000,100000000,3.141511,2605205.000000
strong,3,11,1000000000,90909090,3.141593,1950668.000000
strong,3,12,1000000000,83333333,3.141621,1806070.000000
strong,3,1,1000000000,1000000000,3.141581,22882287.250000
strong,3,2,1000000000,500000000,3.141582,13577006.500000
strong,3,3,1000000000,333333333,3.141593,7115862.750000
strong,3,4,1000000000,250000000,3.141653,5344062.500000
strong,3,5,1000000000,200000000,3.141499,4279497.750000
strong,3,6,1000000000,166666666,3.141606,4563313.500000
strong,3,7,1000000000,142857142,3.141486,3093297.750000
strong,3,8,1000000000,125000000,3.141609,5238807.500000
strong,3,9,1000000000,111111111,3.141627,2611875.250000
strong,4,10,1000000000,100000000,3.141600,2380628.750000
strong,4,11,1000000000,90909090,3.141578,2476354.750000
strong,4,12,1000000000,83333333,3.141612,1788441.750000
strong,4,1,1000000000,1000000000,3.141597,21327380.000000
strong,4,2,1000000000,500000000,3.141698,10681068.500000
strong,4,3,1000000000,333333333,3.141566,7134543.750000
strong,4,4,1000000000,250000000,3.141554,5342921.500000
strong,4,5,1000000000,200000000,3.141626,5067051.500000
strong,4,6,1000000000,166666666,3.141575,3566932.250000
strong,4,7,1000000000,142857142,3.141602,3194734.250000
strong,4,8,1000000000,125000000,3.141523,2699353.750000
strong,4,9,1000000000,111111111,3.141619,3347306.750000
strong,5,10,1000000000,100000000,3.141596,2152209.250000
strong,5,11,1000000000,90909090,3.141627,2309830.000000
strong,5,12,1000000000,83333333,3.141677,1786114.000000
strong,5,1,1000000000,1000000000,3.141656,21369912.000000
strong,5,2,1000000000,500000000,3.141445,10673180.250000
strong,5,3,1000000000,333333333,3.141546,7124494.500000
strong,5,4,1000000000,250000000,3.141588,5345703.500000
strong,5,5,1000000000,200000000,3.141643,4266307.750000
strong,5,6,1000000000,166666666,3.141567,4563392.500000
strong,5,7,1000000000,142857142,3.141570,3066668.750000
strong,5,8,1000000000,125000000,3.141545,3568658.000000
strong,5,9,1000000000,111111111,3.141580,2381834.500000
weak,1,10,10000000000,1000000000,3.141591,22407565.000000
weak,1,1,1000000000,1000000000,3.141613,21299672.250000
weak,1,11,11000000000,1000000000,3.141563,22943742.250000
weak,1,12,12000000000,1000000000,3.141601,21339959.500000
weak,1,2,2000000000,1000000000,3.141569,21349621.000000
weak,1,3,3000000000,1000000000,3.141579,21371487.250000
weak,1,4,4000000000,1000000000,3.141568,27398989.500000
weak,1,5,5000000000,1000000000,3.141567,21402536.000000
weak,1,6,6000000000,1000000000,3.141603,22577782.250000
weak,1,7,7000000000,1000000000,3.141583,22345114.750000
weak,1,8,8000000000,1000000000,3.141586,22394141.750000
weak,1,9,9000000000,1000000000,3.141589,22367091.000000
weak,2,10,10000000000,1000000000,3.141557,22377132.250000
weak,2,1,1000000000,1000000000,3.141609,21318419.250000
weak,2,11,11000000000,1000000000,3.141579,22349012.750000
weak,2,12,12000000000,1000000000,3.141581,22330040.500000
weak,2,2,2000000000,1000000000,3.141549,21406043.250000
weak,2,3,3000000000,1000000000,3.141608,21359250.750000
weak,2,4,4000000000,1000000000,3.141578,21397692.000000
weak,2,5,5000000000,1000000000,3.141612,22526922.250000
weak,2,6,6000000000,1000000000,3.141544,21568548.750000
weak,2,7,7000000000,1000000000,3.141614,22558298.500000
weak,2,8,8000000000,1000000000,3.141592,22881259.750000
weak,2,9,9000000000,1000000000,3.141625,22743843.750000
weak,3,10,10000000000,1000000000,3.141574,22516374.750000
weak,3,1,1000000000,1000000000,3.141572,21408012.500000
weak,3,11,11000000000,1000000000,3.141596,22492941.500000
weak,3,12,12000000000,1000000000,3.141594,22358299.250000
weak,3,2,2000000000,1000000000,3.141597,21318052.250000
weak,3,3,3000000000,1000000000,3.141566,21370907.000000
weak,3,4,4000000000,1000000000,3.141558,22693829.750000
weak,3,5,5000000000,1000000000,3.141565,21627807.750000
weak,3,6,6000000000,1000000000,3.141598,22371327.750000
weak,3,7,7000000000,1000000000,3.141581,22395298.500000
weak,3,8,8000000000,1000000000,3.141606,22596734.750000
weak,3,9,9000000000,1000000000,3.141589,21920711.250000
weak,4,10,10000000000,1000000000,3.141560,41879956.750000
weak,4,1,1000000000,1000000000,3.141449,21359528.500000
weak,4,11,11000000000,1000000000,3.141596,22426457.000000
weak,4,12,12000000000,1000000000,3.141607,22348716.000000
weak,4,2,2000000000,1000000000,3.141571,21380944.750000
weak,4,3,3000000000,1000000000,3.141608,21354910.000000
weak,4,4,4000000000,1000000000,3.141570,21433499.000000
weak,4,5,5000000000,1000000000,3.141592,21387556.000000
weak,4,6,6000000000,1000000000,3.141594,22363028.000000
weak,4,7,7000000000,1000000000,3.141589,22374015.500000
weak,4,8,8000000000,1000000000,3.141593,23980480.500000
weak,4,9,9000000000,1000000000,3.141609,22577758.500000
weak,5,10,10000000000,1000000000,3.141593,22605543.750000
weak,5,1,1000000000,1000000000,3.141534,21346196.500000
weak,5,11,11000000000,1000000000,3.141589,22344693.000000
weak,5,12,12000000000,1000000000,3.141579,28218395.250000
weak,5,2,2000000000,1000000000,3.141591,21379651.250000
weak,5,3,3000000000,1000000000,3.141590,21374454.250000
weak,5,4,4000000000,1000000000,3.141610,21388320.000000
weak,5,5,5000000000,1000000000,3.141609,21385365.500000
weak,5,6,6000000000,1000000000,3.141613,22417799.000000
weak,5,7,7000000000,1000000000,3.141615,22371845.000000
weak,5,8,8000000000,1000000000,3.141564,22396748.250000
weak,5,9,9000000000,1000000000,3.141595,22544740.250000
```
