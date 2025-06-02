#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const int COLUMNAR = 0;
const int DEBUG_IP = 0;
const int DEBUG_INTERACT = 0;
const int ECHO_OUTPUT = 1;
const int INTERACTIVE = 0;
const int PRINT_HEX = 0;

const int TAPESIZE = 64;
const int MAX_STEPS = 1000;

typedef struct state {
  char *program;
  int ip;
  int dp;
  char *scratch;
  int ini;
  char *intape;
  int outi;
  char *outape;
} state;

// Test programs
char *add_numbers = "[->+<]";
char *print_at = "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.";

char *print_decr_counter_4 = "++++ [-.]";
char *echo1 = ",.";
char *echo4 = "++++ [>,.<-]";

// Valid BF instructions
char *valid = "<>+-.,[]";

// Returns true if anything was printed.
void
simulate(state *state) {
  int n = 0;
  while (state->program[state->ip] && (n < MAX_STEPS)) {
    if (state->dp < 0 || state->dp > SCRATCHSIZE) {
      break;
    }
    if (state->ini < 0 || state->ini > 160) {
      break;
    }
    if (state->outi < 0 || state->outi > 160) {
      break;
    }
    if (DEBUG_IP)
      printf("ip %12d\n", state->ip);
    switch (state->program[state->ip]) {
    case '>':
      state->dp++;
      state->ip++;
      break;
    case '<':
      state->dp--;
      state->ip++;
      break;
    case '+':
      state->scratch[state->dp]++;
      state->ip++;
      break;
    case '-':
      state->scratch[state->dp]--;
      state->ip++;
      break;
    case '.':
      if (ECHO_OUTPUT) {
        if (COLUMNAR)
          printf("               ");
        putchar(state->scratch[state->dp]);
      }
      if (!INTERACTIVE)
	      state->outape[state->outi++] = state->scratch[state->dp];
      state->ip++;
      break;
    case ',':
      if (INTERACTIVE)
	state->scratch[state->dp] = getchar();
      else
	state->scratch[state->dp] = state->intape[state->ini++];
      state->ip++;
      break;
    case '[':
      if (0 == state->scratch[state->dp]) {
        state->ip++;
        int match_count = 1;
        while (match_count &&(state->ip < strlen(state->program))) {
          //printf("  Jumping... %2d\n", state->ip);
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
      if (0 != state->scratch[state->dp]) {
	      state->ip--;
	      int match_count = 1;
	      while (match_count && (state->ip > 0)) {
	        //printf("  Jumping... %2d\n", state->ip);
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
  }
}

void
make_random(char *tape, int len) {
  for (int i=0; i<len/4; i += 1)
    ((int*)tape)[i] = rand() % 0xFFFFFFFF;
}

void
print_bf(char *program, int len) {
  if (PRINT_HEX) {
    for (int i=0; i<len/4; i += 1)
      printf("%8x|", ((int*)program)[i]);
    puts("\n");
  }
  printf("~~");
  for (int i=0; i<len; i += 1){
    int found = 0;
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
interact(char *tape1, char *tape2, int len, char *result1, char *result2) {
  char *bigtape = malloc(2*len);
  memcpy(bigtape, tape1, len);
  memcpy(bigtape + len, tape2, len);
  if (DEBUG_INTERACT){
    printf("Combined tape:\n");
    print_bf(bigtape, 2*len);
  }

  char *input_tape = malloc(2*len);
  make_random(input_tape, 2*len);
  if (DEBUG_INTERACT){
    printf("Random input tape:\n");
    print_bf(input_tape, 2*len);
  }

  char *scratch = malloc(30000);

  char *output_tape = malloc(2*len);
  make_random(output_tape, 2*len);
  if (DEBUG_INTERACT){
    printf("Random output tape:\n");
    print_bf(output_tape, 2*len);
  }

  state state = {
    program: bigtape,
    ip: 0,
    dp: 0,
    scratch: scratch,
    ini: 0,
    intape: input_tape,
    outi: 0,
    outape: output_tape
  };

  simulate(&state);

  // if (DEBUG_INTERACT){
  //   if (worthwhile) {
  //     printf("Combined tape:\n");
  //     print_bf(bigtape, 2*len);
  //   }
  // }

  if (DEBUG_INTERACT) {
    printf("Result tape:\n");
    print_bf(state.outape, 2*len);
  }

  memcpy(result1, state.outape, len);
  memcpy(result2, state.outape + len, len);

  free(input_tape);
  free(scratch);
  free(output_tape);
}

int ntapes = 8192;
int generation_limit = 100000;

int tapesize = 80;

int
main() {
  unsigned seed;
  scanf("%d", &seed);
  srand(seed);

  char tapes[ntapes][tapesize];
  for (int i=0; i<ntapes; i++)
    make_random(tapes[i], tapesize);

  for (int generation=0; generation<generation_limit; generation++) {
    if (generation % 100 == 0)
      printf("Generation %d\n", generation);
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
