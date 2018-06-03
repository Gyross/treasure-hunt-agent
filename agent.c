/*********************************************
 *  agent.c
 *  Sample Agent for Text-Based Adventure Game
 *  COMP3411 Artificial Intelligence
 *  UNSW Session 1, 2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pipe.h"
#include "worldmodel.h"

int   pipe_fd;
FILE* in_stream;
FILE* out_stream;

char view[5][5];

int action_index;
char actions[10000];

struct WorldModel* wm = NULL;

char get_action( char view[5][5] ) {

    if ( wm == NULL ) {
        wm = wm_create(view);
    } else {
        wm_update_view(wm, view);
    }

    wm_walk(wm, actions);

    if ( actions[0] != 0 ) {
        wm_take_action(wm, actions[0]);
        return actions[0];
    }

  // REPLACE THIS CODE WITH AI TO CHOOSE ACTION

  /*
  int ch=0;

  printf("Enter Action(s): ");

  while(( ch = getchar()) != -1 ) { // read character from keyboard

    switch( ch  ) { // if character is a valid action, return it
    case 'F': case 'L': case 'R': case 'C': case 'U': case 'B':
    case 'f': case 'l': case 'r': case 'c': case 'u': case 'b':
      return((char) ch);
    }
  }
  */
  return 0;
}

void print_view()
{
  int i,j;

  printf("\n+-----+\n");
  for( i=0; i < 5; i++ ) {
    putchar('|');
    for( j=0; j < 5; j++ ) {
      if(( i == 2 )&&( j == 2 )) {
        putchar( '^' );
      }
      else {
        putchar( view[i][j] );
      }
    }
    printf("|\n");
  }
  printf("+-----+\n");
}

int main( int argc, char *argv[] )
{
  char action;
  int sd;
  int ch;
  int i,j;

  struct WorldModel* wm = NULL;

  if ( argc < 3 ) {
    printf("Usage: %s -p port\n", argv[0] );
    exit(1);
  }

    // open socket to Game Engine
  sd = tcpopen("localhost", atoi( argv[2] ));

  pipe_fd    = sd;
  in_stream  = fdopen(sd,"r");
  out_stream = fdopen(sd,"w");

  while(1) {
      // scan 5-by-5 wintow around current location
    for( i=0; i < 5; i++ ) {
      for( j=0; j < 5; j++ ) {
        if( !(( i == 2 )&&( j == 2 ))) {
          ch = getc( in_stream );
          if( ch == -1 ) {
            exit(1);
          }
          view[i][j] = ch;
        }
      }
    }

    print_view(); // COMMENT THIS OUT BEFORE SUBMISSION
    action = get_action( view );
                                // (or maybe we use a different function)
    putc( action, out_stream );
    fflush( out_stream );
  }

  free(wm);

  return 0;
}
