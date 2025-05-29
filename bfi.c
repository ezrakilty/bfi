#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const int COLUMNAR = 0;
const int DEBUG_IP = 0;
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

void
simulate(state *state) {
  int n = 0;
  while (state->program[state->ip] && (n < MAX_STEPS)) {
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
      if (COLUMNAR)
        printf("               ");
      putchar(state->scratch[state->dp]);
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
  printf("Combined tape:\n");
  print_bf(bigtape, 2*len);

  char *inputtape = malloc(2*len);
  make_random(inputtape, 2*len);
  printf("Random input tape:\n");
  print_bf(inputtape, 2*len);

  char *scratch = malloc(30000);
  char *outape = malloc(2*len);

  state state = {
    program: bigtape,
    ip: 0,
    dp: 0,
    scratch: scratch,
    ini: 0,
    intape: inputtape,
    outi: 0,
    outape: outape
  };

  simulate(&state);
  printf("Result tape:\n");
  print_bf(state.outape, 2*len);

  memcpy(result1, state.outape, len);
  memcpy(result2, state.outape + len, len);
}

int ntapes = 12;

int tapesize = 80;

int
main() {
  char tapes[ntapes][tapesize];
  for (int i=0; i<ntapes; i++)
    make_random(tapes[i], tapesize);
  for (int i=0; i<ntapes; i++)
    print_bf(tapes[i], tapesize);

  int parent1 = 10;
  int parent2 = 11;

  char *temp1, *temp2;
  interact(tapes[parent1], tapes[parent2], tapesize, temp1, temp2);
  memcpy(tapes[parent1], temp1, tapesize);
  memcpy(tapes[parent2], temp2, tapesize);

  for (int i=0; i<ntapes; i++)
    print_bf(tapes[i], tapesize);

  return 0;
}
