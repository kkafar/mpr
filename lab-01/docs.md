---
Title: Metody programowania równoległego - laboratorium 1
Author: Kacper Kafara
---

# Metody programowania równoległego, laboratorium 1 - MPI

## Zadanie

Celem zadania było przetestowanie dwóch równych typów komunikacji `point to point` w `MPI`, poprzez
zmierzenie przepustowości oraz opóźnienia komunikacyjnego (narzut) w dwóch konfiguracjach:

1. komunikacja pomiędzy dwoma procesorami tego samego węzła (nazywana dalej `single node`)
2. komunikacja pomiędzy dwoma procesorami z różnych węzłów (nazywana dalej `two nodes`)


## Realizacja

### Tech stack

* Kod do przeprowadzania testów został zaimplementowany w `C`
* Rysowanie wykresów i analiza w `Python 3.11` z wykorzystaniem pakietów
  `numpy`, `pandas`, `matplotlib`
* Implementacja MPI -- taka jak na `vCluster` -- prawdopodobnie `OpenMPI` ale bez pewności

### Kod programu testowego

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

### Makefile do automatyzacji + pliki maszynowe

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

#### Single node

```machinefile
vnode-04.dydaktyka.icsr.agh.edu.pl:2
```

#### Two nodes

```machinefile
vnode-09.dydaktyka.icsr.agh.edu.pl:1
vnode-10.dydaktyka.icsr.agh.edu.pl:1
```

### Rysowanie wykresów

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

## Przeprowadzone eksperymenty

Eksperymenty zostały przeprowadzone w obydwu konfiguracjach opisanych w poleceniu.

Testowane typy komunikacji:

1. Wysyłanie `MPI_Send`, odbieranie `MPI_Recv` (dalej nazywany `std`)
2. Wysyłanie buferowane `MPI_Bsend`, odbieranie `MPI_Recv` (dalej nazywany `buff`)

W każdej z konfiguracji rozłożenie węzłów `x` typ komunikacji testowane były przepustowość i opóźnienie.

### Testowanie opóźnienia

W celu oszacowania opóźnienia przygotowano eksperyment z dwoma komunikującymi się procesami.
Przebieg komunikacji został zilustrowany poniżej.

![Eksperyment - opóźnienie](playground/src/experiment_delay.png)

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

![przepustowość - single node](playground/src/single_node_Mb_s_low.png)

*Rysunek 2 - Przepustowość w zależności od rozmiaru przesyłanej wiadomości -- konfiguracja `single node`*


![przepustowość - two nodes](playground/src/two_nodes_Mb_s_low.png)

*Rysunek 3 -Przepustowość w zależności od rozmiaru przesyłanej wiadomości -- konfiguracja `two nodes`*

## Wnioski

1. Wbrew moim oczekiwaniom znacznie szybsza jest komunikacja poprzez `MPI_Send` -- [**prawdopodobnie**](https://www.mcs.anl.gov/research/projects/mpi/sendmode.html)
synchroniczna. Być może moment `flush` bufora powoduje duże opóźnienia (dużo danych do przesyłu na raz?) -- ta hipoteza wymagałaby dalszych pomiarów.

2. Jest ogromna przepaść pomiędzy (~18x) pomiędzy komunikacją w ramach jednego węzła, a różnymi węzłami
3. Na wykresie 2 widzimy (wypłaszczenie wykresu od rozmiaru wiadomości ~400 KB), że przepustowość systemu wynosi ~63 Gb/s dla komunikacji niebuforowanej
   ~41 Gb/s dla komunikacji buferowanej. Ciężko określić jak dobrym przybliżeniem fizycznej przepustowości łącza jest ~63 Gb/s.

	 Początkowy wzrost przepustowości powodowany jest faktem, że system nie napotkał jeszcze ograniczeń i "na bieżąco" obsługiwał wiadomości.

4. W przypadku komunikacji pomiędzy dwoma węzłami (wykres 3) widzimy większą niestabliność przepustowości (policzyć odchylenie). Tutaj również lepiej
   działa komunikacja synchroniczna.
