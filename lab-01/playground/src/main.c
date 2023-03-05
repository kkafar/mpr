#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#ifndef CONFIG_CODE
#define CONFIG_CODE -1
#endif

#ifndef ITERATION_COUNT
#define ITERATION_COUNT 10
#endif

#define STRING_UNKNOWN_LENGTH -1

#define S_TO_MS_FACTOR 1e6


typedef int (*FnSendHandle)(const void *, int, MPI_Datatype, int, int, MPI_Comm);
typedef int (*FnRecvHanlde)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);

typedef struct String {
  char * data;
  size_t len;
  size_t capacity;
} String;

bool string_init(String *str, char *mem, const size_t len, const size_t capacity) {
  str->data = mem;
  str->len = len;
  str->capacity = capacity;
  return true;
}

bool string_dealloc(String *str) {
  free(str);
  return true;
}

String g_hostname;
int g_rank, g_size;

bool init_global_state(void) {
  assert((MPI_Comm_rank(MPI_COMM_WORLD, &g_rank) == MPI_SUCCESS) && "Valid rank returned");
  assert((MPI_Comm_size(MPI_COMM_WORLD, &g_size) == MPI_SUCCESS) && "Valid size returned");

  char *mem = (char *) malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME);
  if (mem == NULL) {
    printf("Failed to allocate memory for processor name in process %d\n", g_rank);
    return false;
  }
  string_init(&g_hostname, mem, STRING_UNKNOWN_LENGTH, MPI_MAX_PROCESSOR_NAME);

  assert((MPI_Get_processor_name(g_hostname.data, (int *)&g_hostname.len) == MPI_SUCCESS) && "Valid processor name");

  return true;
}

void print_process_info(void) {
  printf("Hostname: %s, Process: %d, Total Process Count: %d, Config codename: %d\n", g_hostname.data, g_rank, g_size, CONFIG_CODE);
  fflush(stdout);
}

// pomiary przepustowości w zależności od długości komunikatów
void experiment_throughput(FnSendHandle send_fn, FnRecvHanlde recv_fn) {
  printf("Starting EXP 1 on process: %d\n", g_rank);
  printf("Ending   EXP 1 on process: %d\n", g_rank);
}

// pomiary opóźnienia (przepustowość przy małym komunikacie)
void experiment_delay(FnSendHandle send_fn, FnRecvHanlde recv_fn) {
  printf("Starting EXP 2 on process %d\n", g_rank);

  const int cping = 0;
  const int cpong = 1;
  const char communication_type[] = "Standard";

  char payload = 'a';

  const int iteration_count = ITERATION_COUNT;

  double start_time, end_time;

  // Synchronizujemy wszystkie procesy tego komunkatora
  MPI_Barrier(MPI_COMM_WORLD);

  start_time = MPI_Wtime() * S_TO_MS_FACTOR;
  for (int i = 0; i < iteration_count; ++i) {
    if (g_rank == cping) {
      send_fn(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD);
      recv_fn(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (g_rank == cpong) {
      recv_fn(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      send_fn(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD);
    } else {
      printf("Dunno what happened\n");
    }
  }
  end_time = MPI_Wtime() * S_TO_MS_FACTOR;

  if (g_rank == cping) {
    double elapsed_time = end_time - start_time;
    double single_send_time = elapsed_time / (ITERATION_COUNT * 2);

    if (g_rank == cping) {
      printf("Single send time: %lf [ms]\n", single_send_time);
    }
  }

  printf("Ending   EXP 2 on process: %d\n", g_rank);
}


int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);
  assert((init_global_state() == true) && "Global state initialized");
  print_process_info();

  MPI_Barrier(MPI_COMM_WORLD);
  experiment_throughput(MPI_Send, MPI_Recv);

  MPI_Barrier(MPI_COMM_WORLD);
  experiment_delay(MPI_Send, MPI_Recv);


  MPI_Finalize();
  return 0;
}

