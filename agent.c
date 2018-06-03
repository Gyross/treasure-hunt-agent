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


struct WorldModel* wm = NULL;

int path_index = 0;
char path[10000000];
bool win = false;
bool explore = false;
bool deep = false;

char get_action( char view[5][5] ) {

    char action = '\0';

    if ( wm == NULL ) {
        wm = wm_create(view);
    } else {
        wm_update_view(wm, view);
    }

    
    // If we already have a path just continue on that path
    if ( win || (deep && path_index <=2) ) {
        action = path[path_index];
        path_index++;
    } else {
        path_index = 0;
        deep = false;
        // Try to find a winning path
        win = wm_walk(wm, path, GOAL_WIN, 0);

        // If we found a winning path
        if ( win ) {
            action = path[path_index];
            path_index++;
        } else {
            // Otherwise try to reveal tiles to find a winning
            // path
            explore = wm_walk(wm, path, GOAL_EXPLORE, 0);

            if ( explore ) {
                action = path[0];
            } else {
                // If there are no more paths to reveal, we have to make a choice
                // Choose a run of depth 3, else 2, else 1
                int depth;
                for ( depth = 10; depth > 0; depth-- ) {
                    if ( wm_walk(wm, path, GOAL_DEPTH, depth) ) {
                        break;
                    }
                }

                action = path[0];
            }
        }
    }
        
    // Take the specified actions
    if ( action != '\0' ) {
        wm_take_action(wm, action);
    }

    return action;
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

    //print_view(); // COMMENT THIS OUT BEFORE SUBMISSION
    action = get_action( view );
    putc( action, out_stream );
    fflush( out_stream );
  }

  free(wm);

  return 0;
}
