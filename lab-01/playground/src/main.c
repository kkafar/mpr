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

  for (int i = 0; i < msg_sizes_arr_size; ++i) {
    MPI_Barrier(MPI_COMM_WORLD);
    int msg_size = msg_sizes_arr[i];
    if (g_rank == 0) printf("Doing computation for msg_size: %d\n", msg_size);
    
    start_time = MPI_Wtime() * S_TO_MS_FACTOR;
    for (int message_id = 0; i < ITERATION_COUNT; ++message_id) {
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
    end_time = MPI_Wtime() * S_TO_MS_FACTOR;

    if (g_rank == cping) {
      double elapsed_time = end_time - start_time;
      double single_send_time  = elapsed_time / (ITERATION_COUNT * 2);
      double throughput = msg_size * 8;
      fprintf(logfile, "%s,%d,%lf,%lf\n", cfg.name, msg_size, single_send_time, throughput);
    }
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

  MPI_Barrier(MPI_COMM_WORLD);
  experiment_throughput(throughput_cfg);

  MPI_Barrier(MPI_COMM_WORLD);
  experiment_delay(delay_cfg);

  // experiment_delay(MPI_Bsend, MPI_Recv, "Delay buff");

  teardown_global_state();
  MPI_Finalize();
  return 0;
}

