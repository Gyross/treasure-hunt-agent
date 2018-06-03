#include "worldmodel.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

struct Pos {
    int x;
    int y;
};

// Linked list for nodes
struct PosNode {
    struct Pos value;
    struct PosNode* next;
};

struct Pos pos_set( int x, int y ) {
    struct Pos pos;
    pos.x = x;
    pos.y = y;
    return pos;
}

// Relative forward increment
// Moves pos 'forward' one step relative to rel_direction
// e.g. if rel_dir is DIRECITON_LEFT, x coord will be decremented
struct Pos pos_forward_rel( struct Pos pos, int amount, Direction rel_dir ) {
    switch(rel_dir) {
        case DIRECTION_RIGHT:
            pos.x += amount;
            break;
        case DIRECTION_DOWN:
            pos.y += amount;
            break;
        case DIRECTION_LEFT:
            pos.x -= amount;
            break;
        case DIRECTION_UP:
            pos.y -= amount;
            break;
    }
    return pos;
}

Direction dir_turn_right( Direction dir ) {
    switch(dir) {
        case DIRECTION_UP:
            return DIRECTION_RIGHT;
        case DIRECTION_RIGHT:
            return DIRECTION_DOWN;
        case DIRECTION_DOWN:
            return DIRECTION_LEFT;
        case DIRECTION_LEFT:
            return DIRECTION_UP;
        default:
            return dir;
    }
}

Direction dir_turn_left( Direction dir ) { 
    switch(dir) {
        case DIRECTION_UP:
            return DIRECTION_LEFT;
        case DIRECTION_RIGHT:
            return DIRECTION_UP;
        case DIRECTION_DOWN:
            return DIRECTION_RIGHT;
        case DIRECTION_LEFT:
            return DIRECTION_DOWN;
        default:
            return dir;
    }
}

struct WorldModel { // The grid
    char grid[GRID_SIZE][GRID_SIZE];

    // The agent
    Direction dir;
    struct Pos pos;

    bool treasure;
    bool key;
    bool axe;
    bool raft;
    int stones;
};

struct WorldModel* wm_create(char view[VIEW_SIZE][VIEW_SIZE]) {
    // Malloc the structure we need
    struct WorldModel* wm = malloc(sizeof(struct WorldModel));

    // Initialize entire grid to '?' for unknown tile
    int i, j;
    for ( i = 0; i < GRID_SIZE; i++ ) {
        for ( j = 0; j < GRID_SIZE; j++ ) {
            wm->grid[i][j] = TILE_UNKNOWN;
        }
    }

    // Initialize agent position
    wm->dir   = DIRECTION_UP;
    wm->pos   = pos_set(HOME_POS, HOME_POS);

    // Initialize agent items
    wm->key    = false;
    wm->axe    = false;
    wm->raft   = false;
    wm->stones = 0;

    // update the grid with the initial view
    for ( i = -VIEW_DIST; i <= VIEW_DIST; i++ ) {
        for ( j = -VIEW_DIST; j <= VIEW_DIST; j++ ) {
            wm->grid[HOME_POS+i][HOME_POS+j] = view[VIEW_DIST+i][VIEW_DIST+j];
        }
    }

    // Replace the home position with the home tile
    wm->grid[HOME_POS][HOME_POS] = TILE_HOME;

    return wm;
}

void wm_destroy(struct WorldModel* wm) {
    // possibly need to do more freeing when we add
    // new data to the WorldModel struct
    free(wm);
}


void wm_take_action(struct WorldModel* wm, char action) {
    
    struct Pos forward_pos = pos_forward_rel(wm->pos, 1, wm->dir);
    char forward_tile = wm_get_tile(wm, forward_pos);

    switch(action) {
        case ACTION_FORWARD:
            // Depending on the tile, do certain actions
            // If we make a move we cannot make, abort the program,
            // since we intend never to make such moves
            switch(forward_tile) {
                case TILE_UNKNOWN:
                    // Not possible to enter an unkown tile
                    fprintf(stderr, "Move into unkown tile\n");
                    //assert(false);

                // No required extra action for these tiles
                case TILE_HOME:
                case TILE_LAND:
                case TILE_USED_STONE:
                    wm->pos = forward_pos;
                    break;

                // Need to check for use of stones and rafts
                case TILE_WATER:
                    // If we are just enetering the water, use a stone
                    // or the raft
                    if ( wm_get_tile(wm, wm->pos) != TILE_WATER ) {
                        if ( wm->stones >= 0 ) {
                            wm->stones--;
                            wm_set_tile(wm, forward_pos, TILE_USED_STONE);
                        } else if ( wm->raft ) {
                            wm->raft = false;
                        }
                    }
                    
                    wm->pos = forward_pos;
                    break;

                // Don't move into obstacles
                case TILE_TREE:
                case TILE_DOOR:
                case TILE_WALL:
                    break;

                // For object tiles, pick up the object
                case TILE_KEY:
                    wm->key = true;
                    wm_set_tile(wm, forward_pos, TILE_LAND);
                    wm->pos = forward_pos;
                    break;
                case TILE_STONE:
                    wm->stones++;
                    wm_set_tile(wm, forward_pos, TILE_LAND);
                    wm->pos = forward_pos;
                    break;
                case TILE_AXE:
                    wm->axe = true;
                    wm_set_tile(wm, forward_pos, TILE_LAND);
                    wm->pos = forward_pos;
                    break;
                case TILE_TREASURE:
                    wm->treasure = true;
                    wm_set_tile(wm, forward_pos, TILE_LAND);
                    wm->pos = forward_pos;
                    break;

                default:
                    fprintf(stderr, "Invalid tile!\n");
                    assert(false);
            }
            break;

        case ACTION_RIGHT:
            wm->dir = dir_turn_right(wm->dir);
            break;

        case ACTION_LEFT:
            wm->dir = dir_turn_left(wm->dir);
            break;

        case ACTION_CHOP:
            if ( wm_get_tile(wm, forward_pos) == TILE_TREE ) {
                wm_set_tile(wm, forward_pos, TILE_LAND);
                wm->raft = true;
            }
            break;

        case ACTION_UNLOCK:
            if ( wm_get_tile(wm, forward_pos) == TILE_DOOR ) {
                wm_set_tile(wm, forward_pos, TILE_LAND);
            }
            break;
    }
}

void wm_update_view(struct WorldModel* wm, char view[VIEW_SIZE][VIEW_SIZE]) {
    int i, j;
    struct Pos cur_pos;
    char view_tile;
    char grid_tile;

    for ( i = -VIEW_DIST; i <= VIEW_DIST; i++ ) {
        for ( j = -VIEW_DIST; j <= VIEW_DIST; j++ ) {
            // Calculate the grid index of the tile
            cur_pos = pos_forward_rel(wm->pos, -i, wm->dir);
            cur_pos = pos_forward_rel(cur_pos, j, dir_turn_right(wm->dir));

            printf("Update grid[%d][%d] <- view[%d][%d]\n", cur_pos.y, cur_pos.x, i+VIEW_DIST, j+VIEW_DIST);

            view_tile = view[i+VIEW_DIST][j+VIEW_DIST];
            grid_tile = wm_get_tile(wm, cur_pos);

            if ( grid_tile == TILE_UNKNOWN ) {
                wm_set_tile(wm, cur_pos, view_tile);
            }
        }
    }
}

char wm_get_tile(struct WorldModel* wm, struct Pos pos) {
    return wm->grid[pos.y][pos.x];
}

void wm_set_tile(struct WorldModel* wm, struct Pos pos, char tile_val) {
    wm->grid[pos.y][pos.x] = tile_val;
}

void wm_print(struct WorldModel* wm) {
    int i, j;
    char c;
    for ( i = 0; i < GRID_SIZE; i++ ) {
        for ( j = 0; j < GRID_SIZE; j++ ) {
            c = wm->grid[i][j];
            printf("%c", c);
        }
        printf("\n");
    }

    printf("Pos: (%d,%d)\n", wm->pos.y, wm->pos.x);
}


// Simple BFS on tiles in the grid
// This is used to find all the tiles we need to explore, as well as
// finding paths to items within a section of the map.
// We can't do a BFS or even IDS on the state space, since the state space grows
// exponentially. The grid has a fixed size, and therefore BFS is guaranteed to
// be fast.
void wm_walk(struct WorldModel* wm, char* actions) {
    bool tile_checked[GRID_SIZE][GRID_SIZE] = {{0}};
    struct PosQueue* pq = pq_create();
    struct Pos cur_pos, adj_pos;
    Direction dir_to_get_here[GRID_SIZE][GRID_SIZE];
    bool goal_found = false;

    // Before running BFS, check if the tile is right in front
    // of the agent. This is neccessary because otherwise 
    // walking behaviour will be erratic and inefficient
    cur_pos = pos_forward_rel(wm->pos, 1, wm->dir);
    if ( wm_walk_test_permissible(wm, cur_pos) && 
         wm_walk_test_goal(wm, cur_pos)) {
        dir_to_get_here[cur_pos.y][cur_pos.x] = wm->dir;
        fprintf(stderr, "One step!\n");
        goal_found = true;
    }
    // Otherwise run the BFS to find the next best option
    else {
        pq_push(pq, wm->pos);
        tile_checked[wm->pos.y][wm->pos.x] = true;
        while(!pq_empty(pq)) {
            cur_pos = pq_pop(pq);

            fprintf(stderr, "Search: (%d,%d)\n",cur_pos.y, cur_pos.x);
            if ( wm_walk_test_permissible(wm, cur_pos) ) {

                // If we need to explore the tile, then this 
                // is the tile we are looking for.
                if ( wm_walk_test_goal(wm, cur_pos) ) {
                    goal_found = true;
                    break;
                }

                // Generate the adjacent positions for all four directions
                // No easy way to put this in a loop, so do it case by case
                adj_pos = pos_forward_rel(cur_pos,1,DIRECTION_UP);
                if ( !tile_checked[adj_pos.y][adj_pos.x] ) {
                        fprintf(stderr, "Push: (%d,%d)\n",adj_pos.y, adj_pos.x);
                        tile_checked[adj_pos.y][adj_pos.x] = true;   
                        dir_to_get_here[adj_pos.y][adj_pos.x] = DIRECTION_UP;
                        pq_push(pq, adj_pos);
                }

                adj_pos = pos_forward_rel(cur_pos,1,DIRECTION_RIGHT);
                if ( !tile_checked[adj_pos.y][adj_pos.x] ) {
                        fprintf(stderr, "Push: (%d,%d)\n",adj_pos.y, adj_pos.x);
                        tile_checked[adj_pos.y][adj_pos.x] = true;   
                        dir_to_get_here[adj_pos.y][adj_pos.x] = DIRECTION_RIGHT;
                        pq_push(pq, adj_pos);
                }

                adj_pos = pos_forward_rel(cur_pos,1,DIRECTION_LEFT);
                if ( !tile_checked[adj_pos.y][adj_pos.x] ) {
                        fprintf(stderr, "Push: (%d,%d)\n",adj_pos.y, adj_pos.x);
                        tile_checked[adj_pos.y][adj_pos.x] = true;   
                        dir_to_get_here[adj_pos.y][adj_pos.x] = DIRECTION_LEFT;
                        pq_push(pq, adj_pos);
                }
                
                adj_pos = pos_forward_rel(cur_pos,1,DIRECTION_DOWN);
                if ( !tile_checked[adj_pos.y][adj_pos.x] ) {
                        fprintf(stderr, "Push: (%d,%d)\n",adj_pos.y, adj_pos.x);
                        tile_checked[adj_pos.y][adj_pos.x] = true;   
                        dir_to_get_here[adj_pos.y][adj_pos.x] = DIRECTION_DOWN;
                        pq_push(pq, adj_pos);
                }
            }
        }
    }

    // Done with the queue
    pq_destroy(pq);


    if ( goal_found ) {
        fprintf(stderr, "Goal found!\n");
        // Backtrack to construct a path to the goal
        int k = 0;
        Direction backtrack[GRID_SIZE*GRID_SIZE];
        // While cur_pos != wm->pos
        while( (cur_pos.x != wm->pos.x) || (cur_pos.y != wm->pos.y) ) {
            k++;
            // Put the direction in the array
            backtrack[k] = dir_to_get_here[cur_pos.y][cur_pos.x];

            switch(backtrack[k]) {
                case DIRECTION_UP:
                    fprintf(stderr, "UP\n");
                    break;
                case DIRECTION_LEFT:
                    fprintf(stderr, "LEFT\n");
                    break;
                case DIRECTION_DOWN:
                    fprintf(stderr, "DOWN\n");
                    break;
                case DIRECTION_RIGHT:
                    fprintf(stderr, "RIGHT\n");
                    break;
            }
            // Go in reverse
            cur_pos = pos_forward_rel(cur_pos, -1, backtrack[k]);
        }

        fprintf(stderr, "L1\n");
        // Construct the action sequence from the backtrack
        int j = 0;
        Direction cur_dir = wm->dir;
        while ( k > 0 ) {
            if ( cur_dir == backtrack[k] ) {
                actions[j] = 'f';
                j+=1;
            } else if ( dir_turn_right(cur_dir) == backtrack[k] ) {
                cur_dir = backtrack[k];
                actions[j]   = 'r';
                actions[j+1] = 'f';
                j+=2;
            } else if ( dir_turn_left(cur_dir) == backtrack[k] ) {
                cur_dir = backtrack[k];
                actions[j]   = 'l';
                actions[j+1] = 'f';
                j+=2;
            } else {
                actions[j]   = 'r';
                actions[j+1] = 'r';
                actions[j+2] = 'f';
                j+=3;
            }
            k--;
        }

        fprintf(stderr, "L2\n");
    } else {
        actions[0] = '\0';
    }

    fprintf(stderr, "Path: %s", actions);
}


bool wm_walk_test_permissible(struct WorldModel* wm, struct Pos pos) {
    char start_tile = wm_get_tile(wm, wm->pos);
    char cur_tile   = wm_get_tile(wm, pos);
    if ( start_tile == TILE_WATER ) {
        if ( cur_tile == TILE_WATER ) {
            return true;
        }
    } else if (cur_tile == TILE_HOME ||
               cur_tile == TILE_LAND ||
               cur_tile == TILE_USED_STONE ||
               cur_tile == TILE_AXE ||
               cur_tile == TILE_KEY ||
               cur_tile == TILE_TREASURE) {
        return true;
    }
    return false;
};

bool wm_walk_test_goal(struct WorldModel* wm, struct Pos pos) {
    int i, j;
    for ( i = pos.y - VIEW_DIST; i <= pos.y + VIEW_DIST; i++ ) {
        for ( j = pos.x - VIEW_DIST; j <= pos.x + VIEW_DIST; j++ ) {
            if ( wm->grid[i][j] == TILE_UNKNOWN ) {
                return true;
            }
        }
    }
    return false;
}





// -----------------------------------------------------------
// The PosQueue structure
// -----------------------------------------------------------


struct PosQueue {
    struct PosNode* front;
    struct PosNode* back;
};

struct PosQueue* pq_create() {
    struct PosQueue* pq = malloc(sizeof(struct PosQueue));
    pq->front = NULL;
    pq->back  = NULL;
    return pq;
}

void pq_destroy(struct PosQueue* pq) {
    //struct Pos throwaway;
    while(!pq_empty(pq)) {
        pq_pop(pq); // Use pop to free all nodes
    }
    free(pq);
}

bool pq_empty(struct PosQueue* pq) {
    if ( pq->front == NULL ) {
        return true;
    }
    return false;
}

void pq_push(struct PosQueue* pq, struct Pos pos) {
    struct PosNode* new_node = malloc(sizeof(struct PosNode));
    new_node->next = NULL;
    new_node->value = pos;
    if ( pq_empty(pq) ) {
        pq->front = new_node;
    } else {
        pq->back->next = new_node;
    }
    pq->back = new_node;
}

// Always check if the queue is empty before popping
// The program does not handle this case
struct Pos pq_pop(struct PosQueue* pq) {
    struct PosNode* temp = pq->front;
    struct Pos retval = temp->value;

    // If the queue will become empty after the pop
    if ( pq->front == pq->back ) { 
        pq->front = NULL;
        pq->back = NULL;
    } else {
        pq->front = pq->front->next;
    }

    free(temp);
    return retval;
}


