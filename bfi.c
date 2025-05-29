#include <string.h>
#include <stdio.h>

const int COLUMNAR = 0;
const int DEBUG_IP = 0;
const int INTERACTIVE = 0;

const int TAPESIZE = 64;
const int MAX_STEPS = 1000;

typedef struct state {
  char *program;
  int ip;
  int dp;
  char scratch[30000];
  int ini;
  char intape[TAPESIZE];
  int outi;
  char outape[TAPESIZE];
} state;

// Test programs
char *add_numbers = "[->+<]";
char *print_at = "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.";

char *print_decr_counter_4 = "++++ [-.]";
char *echo1 = ",.";
char *echo4 = "++++ [>,.<-]";

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

int
main() {
  state state = {
    0, 0, 0, 0, 0, 0, 0
  };
  state.program = "++++ [>,.<-]";
  memcpy(state.intape, "ezra456789,./:;'", 16);
  simulate(&state);
  putchar('\n');
}
