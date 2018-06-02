#include "worldmodel.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

struct Pos {
    int x;
    int y;
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
struct Pos pos_forward_rel( struct Pos pos, Direction rel_dir ) {
    switch(rel_dir) {
        case DIRECTION_UP:
            pos.y++;
            break;
        case DIRECTION_RIGHT:
            pos.x++;
            break;
        case DIRECTION_DOWN:
            pos.y--;
            break;
        case DIRECTION_LEFT:
            pos.x--;
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
    struct Pos forward_pos;
    char forward_tile;

    switch(action) {
        case ACTION_FORWARD:
            forward_pos = pos_forward_rel(wm->pos, wm->dir);
            char forward_tile = wm_get_tile(wm, forward_pos);

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
            forward_pos = pos_forward_rel(wm->pos,wm->dir);
            if ( wm_get_tile(wm, forward_pos) == TILE_TREE ) {
                wm_set_tile(wm, forward_pos, TILE_LAND);
                wm->raft = true;
            }
            break;

        case ACTION_UNLOCK:
            forward_pos = pos_forward_rel(wm->pos,wm->dir);
            if ( wm_get_tile(wm, forward_pos) == TILE_DOOR ) {
                wm_set_tile(wm, forward_pos, TILE_LAND);
            }
            break;
    }
}

char wm_get_tile(struct WorldModel* wm, struct Pos pos) {
    return wm->grid[pos.x][pos.y];
}

void wm_set_tile(struct WorldModel* wm, struct Pos pos, char tile_val) {
    wm->grid[pos.x][pos.y] = tile_val;
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

    printf("Pos: (%d,%d)\n", wm->pos.x, wm->pos.y);
}
