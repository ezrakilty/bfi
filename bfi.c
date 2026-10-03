
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const int COLUMNAR = 0;
const int DEBUG_IP = 0;
const int DEBUG_INTERACT = 0;
const int ECHO_OUTPUT = 0;
const int INTERACTIVE = 0;
const int PRINT_HEX = 0;

const int TAPESIZE = 64;
const int MAX_STEPS = 1000;
const int SCRATCHSIZE = 30000;

typedef struct state {
  unsigned char *program;
  int ip;
  int head0;
  int head1;
  unsigned char *inouttape;
} state;

// Test programs
char *add_numbers = "[->+<]";
char *print_at = "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.";

char *print_decr_counter_4 = "++++ [-.]";
char *echo1 = ",.";
char *echo4 = "++++ [>,.<-]";

// Valid BF instructions
char *valid = "<>{}+-.,[]";

void
print_bf(unsigned char *program, int len) {
  if (PRINT_HEX) {
    for (int i=0; i<len/4; i += 1)
      printf("%8x|", ((int*)program)[i]);
    puts("\n");
  }
  printf("~~");
  for (int i=0; i<len; i += 1){
    int found = 0;
    if (program[i] == 0){
        printf("0");
        found = 1;
    } 
    for (int j=0; j<strlen(valid); j++)
      if (program[i] == valid[j]){
        printf("%c", program[i]);
        found = 1;
      }
    if (!found)
      putchar(' ');
  }
  puts("~~");
}

void
histogram(unsigned char tapes[][TAPESIZE], int ntapes) {
  unsigned checksum = 0;
  int jobber[256];
  for (int i = 0; i < 256; i++)
  jobber[i] = 0;
  for (int i = 0; i < ntapes; i++) {
    for (int j = 0; j < TAPESIZE; j++) {
      jobber[tapes[i][j]]++;
      checksum += tapes[i][j];
    }
  }

  int TOPSIZE = 32;
  int heapw[TOPSIZE];
  int heapv[TOPSIZE];
  for (int j = 0; j < TOPSIZE; j++) {
    heapw[j] = 0;
    heapv[j] = 0;
  }
  for (int i = 0; i < 256; i++) {
    int weight = jobber[i];
    for (int j = 0; j < TOPSIZE; j++) {
      if (weight > heapw[j]) {
        for (int k = TOPSIZE-1; k > j; k--) {
          heapw[k] = heapw[k-1];
          heapv[k] = heapv[k-1];
        }
        heapw[j] = weight;
        heapv[j] = i;
        break;
      }
    }
  }
  printf("Histogram of most frequent bytes:\n");
  int total = 0;
  int watershed = 0;
  int rows = 20;
  for (int j = 0; j < TOPSIZE; j++) {
    if (heapw[j]/10 != watershed) {
      printf("(%d) ", heapw[j]/10*10);
      rows--;
    }
    printf("%02hhx ", heapv[j]);
    total += heapw[j];
    watershed = heapw[j]/10;
  }
  printf("\n");
  printf("WEIGHT OF TOP %d BYTES: %d (checksum %d)\n", TOPSIZE, total, checksum);
}

// Returns true if anything was printed.
int
simulate(state *state) {
  int printed = 0;
  int n = 0;
  while (state->program[state->ip] && (n < MAX_STEPS)) {
    assert(state->ip >= 0);
    assert(state->ip < 2 * TAPESIZE);
    assert(state->head0 >= 0);
    assert(state->head1 >= 0);
    assert(state->head0 < 2 * TAPESIZE);
    assert(state->head1 < 2 * TAPESIZE);
    if (DEBUG_IP)
      printf("ip %12d\n", state->ip);
    switch (state->program[state->ip]) {
    case '>':
      state->head0++;
      state->head0 = state->head0 % (2*TAPESIZE);
      state->ip++;
      break;
    case '<':
      state->head0--;
      if (state->head0 < 0) state->head0 += 2*TAPESIZE;
      state->ip++;
      break;
    case '}':
      state->head1++;
      state->head1 = state->head1 % (2*TAPESIZE);
      state->ip++;
      break;
    case '{':
      state->head1--;
      if (state->head1 < 0) state->head1 += 2*TAPESIZE;
      state->ip++;
      break;
    case '+':
      state->inouttape[state->head0]++;
      state->ip++;
      break;
    case '-':
      state->inouttape[state->head0]--;
      state->ip++;
      break;
    case '.':
      printed++;
      if (ECHO_OUTPUT) {
        if (COLUMNAR)
          printf("               ");
        putchar(state->inouttape[state->head0]);
      }
      if (!INTERACTIVE)
        state->inouttape[state->head1] = state->inouttape[state->head0];
      state->ip++;
      break;
    case ',':
      if (INTERACTIVE)
        state->inouttape[state->head0] = getchar();
      else
        state->inouttape[state->head0] = state->inouttape[state->head1];
      state->ip++;
      break;
    case '[':
      if (0 == state->inouttape[state->head0]) {
        state->ip++;
        int match_count = 1;
        while (match_count && (state->ip < 2*TAPESIZE)) {
          if (state->program[state->ip] == '[')
            match_count++;
          else if (state->program[state->ip] == ']')
            match_count--;
          state->ip++;
        }
      } else {
        state->ip++;
      }
      break;
    case ']':
      if (0 != state->inouttape[state->head0]) {
        state->ip--;
        int match_count = 1;
        while (match_count && (state->ip > 0)) {
          if (state->program[state->ip] == '[')
            match_count--;
          else if (state->program[state->ip] == ']')
            match_count++;
          state->ip--;
        }
        state->ip++;  // instruction after
      } else {
        state->ip++;
      }
      break;
    default:
      state->ip++;
      break;
    }
    n++;
    if (state->ip >= 2 * TAPESIZE) break;
  }
  return printed;
}

void
make_random(unsigned char *tape, int len) {
  for (int i=0; i<len/4; i += 1)
    ((int*)tape)[i] = rand() % 0xFFFFFFFF;
}

unsigned
interact(unsigned char *tape1, unsigned char *tape2, int len, unsigned char *result1, unsigned char *result2) {
  /* if (DEBUG_INTERACT){ */
  /*   printf("============ ============ ============ ============ ============ ============ ============ ============ ============ ============ ============ ============ ====\n"); */
  /* } */
  unsigned char *bigtape = malloc(2*len);
  memcpy(bigtape, tape1, len);
  memcpy(bigtape + len, tape2, len);
  if (DEBUG_INTERACT){
    /* printf("Combined tape:\n"); */
    /* print_bf(bigtape, 2*len); */
  }

  state state = {
    .program = bigtape,
    .ip = 0,
    // .head0 = (rand() % (TAPESIZE*2)),
    .head0 = 0,
    //.head1 = (rand() % (TAPESIZE*2)),
    .head1 = 0,
    .inouttape = bigtape,
  };

  int worthwhile = simulate(&state);

  if (0 && (worthwhile > 10)) {
    printf("\nProgram that printed something:\n");
    print_bf(bigtape, 2*len);
  }

  if (DEBUG_INTERACT) {
    printf("Result tape:\n");
    print_bf(state.outape, 2*len);
  }

  memcpy(result1, state.inouttape, len);
  memcpy(result2, state.inouttape + len, len);

  free(bigtape);
  return worthwhile;
}

int ntapes = 8192;
int generation_limit = 24000000;

int tapesize = 80;

int
main() {
  unsigned seed;
  printf("Enter random seed:\n");
  scanf("%d", &seed);
  srand(seed);

  unsigned char tapes[ntapes][TAPESIZE];
  for (int i=0; i<ntapes; i++)
    make_random(tapes[i], tapesize);

  for (int generation=0; generation<generation_limit; generation++) {
    if (generation % 10000 == 0)
      printf("\nGeneration %d\n", generation);
    if (rand() < 0x000FFFF) {
      puts("~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~");
      for (int i=0; i<ntapes; i++)
        print_bf(tapes[i], tapesize);
    }

    int parent1 = rand() % ntapes;
    int parent2 = rand() % ntapes;

    if (DEBUG_INTERACT)
      printf("Crossing over %d + %d\n", parent1, parent2);
    char *temp1, *temp2;
    interact(tapes[parent1], tapes[parent2], tapesize, temp1, temp2);
    memcpy(tapes[parent1], temp1, tapesize);
    memcpy(tapes[parent2], temp2, tapesize);
  }
  return 0;
}
