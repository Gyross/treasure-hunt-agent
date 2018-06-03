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

bool pos_equal( struct Pos p, struct Pos q ) {
    if ( p.x == q.x && p.y == q.y ) {
        return true;
    }
    return false;
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

    if ( wm == NULL ) {
        fprintf(stderr, "No memory for wm_create!\n");
        return NULL;
    }

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
    wm->treasure = false;
    wm->key      = false;
    wm->axe      = false;
    wm->raft     = false;
    wm->stones   = 0;

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

struct WorldModel* wm_copy(struct WorldModel* wm) {
    // Malloc the structure we need
    struct WorldModel* new_wm = malloc(sizeof(struct WorldModel));

    if ( new_wm == NULL ) {
        fprintf(stderr, "No memory for wm_copy!\n");
        return NULL;
    }

    // Copy the grid
    int i, j;
    for ( i = 0; i < GRID_SIZE; i++ ) {
        for ( j = 0; j < GRID_SIZE; j++ ) {
            new_wm->grid[i][j] = wm->grid[i][j];
        }
    }

    // Copy the rest
    new_wm->treasure = wm->treasure;
    new_wm->dir      = wm->dir;
    new_wm->pos      = wm->pos;
    new_wm->key      = wm->key;
    new_wm->axe      = wm->axe;
    new_wm->raft     = wm->raft;
    new_wm->stones   = wm->stones;

    return new_wm;
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

            //printf("Update grid[%d][%d] <- view[%d][%d]\n", cur_pos.y, cur_pos.x, i+VIEW_DIST, j+VIEW_DIST);

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




// DFS
bool wm_dfs(struct WorldModel* old_wm, struct Pos cur_pos, Goal goal,
         bool seen[GRID_SIZE][GRID_SIZE], int depth_limit, char* actions) {
    
    bool saved_seen[GRID_SIZE][GRID_SIZE];
    bool need_to_restore_seen = false;

    //fprintf(stderr, "Begin: (%d,%d)", cur_pos.y, cur_pos.x);

    if ( seen != NULL ) {
        seen[cur_pos.y][cur_pos.x] = true;
    }

    // Check if the tile is permissible with respect to the goal
    if ( !wm_walk_test_permissible(old_wm, cur_pos, goal) ) {
        return false;
    }

    struct WorldModel* wm = wm_copy(old_wm);
    if ( wm == NULL ) {
        fprintf( stderr, "DFS out of memory!\n" );
        return false;
    }

    if ( !pos_equal(cur_pos, wm->pos ) ) {
        // Make all the required turns to move into cur_pos
        // If we are already there this won't make any turns
        if ( pos_equal(cur_pos, pos_forward_rel(wm->pos, 1, dir_turn_left(wm->dir))) ) {
            actions[0] = ACTION_LEFT;
            actions++;
            wm_take_action(wm, ACTION_LEFT);
        } else if ( pos_equal(cur_pos, pos_forward_rel(wm->pos, 1, dir_turn_right(wm->dir))) ) {
            actions[0] = ACTION_RIGHT;
            actions++;
            wm_take_action(wm, ACTION_RIGHT);
        } else if ( pos_equal(cur_pos, pos_forward_rel(wm->pos, -1, wm->dir)) ) {
            actions[0] = ACTION_LEFT;
            actions++;
            actions[0] = ACTION_LEFT;
            actions++;
            wm_take_action(wm, ACTION_LEFT);
            wm_take_action(wm, ACTION_LEFT);
        }

        // If we need a chop or unlock action then take it
        if ( wm_get_tile(wm, cur_pos) == TILE_TREE ) {
            actions[0] = ACTION_CHOP;
            actions++;
            wm_take_action(wm, ACTION_CHOP);
        } else if ( wm_get_tile(wm, cur_pos) == TILE_DOOR ) {
            actions[0] = ACTION_UNLOCK;
            actions++;
            wm_take_action(wm, ACTION_UNLOCK);
        } 

        // Finally make the forward move
        actions[0] = ACTION_FORWARD;
        actions++;
        wm_take_action(wm, ACTION_FORWARD);
    
    } 


    // Test if we have found the goal
    if ( wm_walk_test_goal(wm, goal) ) {
        // We are at the goal, so we don't need
        // any more actions.
        //fprintf(stderr, "Goal: (%d,%d)", cur_pos.y, cur_pos.x);
        actions[0] = '\0';
        wm_destroy(wm);
        return true;
    } 

    // If we reached the depth limit, don't try any more tiles.
    if ( depth_limit == 0 ) {
        wm_destroy(wm);
        return false;
    }

    depth_limit--;

    // Now if we hit an obstacle or picked up an object we need to save the old seen
    // array and clear it. We restore it at the end of the function
    char old_tile = wm_get_tile(old_wm, cur_pos);
    if ( old_tile == TILE_KEY ||
         old_tile == TILE_WATER ||
         old_tile == TILE_TREE ||
         old_tile == TILE_DOOR ||
         old_tile == TILE_AXE ||
         old_tile == TILE_KEY ||
         old_tile == TILE_STONE ||
         old_tile == TILE_TREASURE ) {

        // save and clear seen
        int i, j;
        for( i = 0; i < GRID_SIZE; i++ ) {
            for ( j = 0; j < GRID_SIZE; j++ ) {
                saved_seen[i][j] = seen[i][j];
                seen[i][j] = false;
            }
        }

        need_to_restore_seen = true;

        // set the current position to seen
        seen[cur_pos.y][cur_pos.x] = true;
    }

    
    // Mark adjacent tiles as seen
    struct Pos pos_f = pos_forward_rel(cur_pos, 1, wm->dir);
    struct Pos pos_r = pos_forward_rel(cur_pos, 1, dir_turn_right(wm->dir));
    struct Pos pos_l = pos_forward_rel(cur_pos, 1, dir_turn_left(wm->dir));
    struct Pos pos_b = pos_forward_rel(cur_pos, -1, wm->dir);

    // Test Walking forward
    if ( !seen[pos_f.y][pos_f.x] ) {
        //fprintf(stderr, "Add: (%d,%d)", pos_f.y, pos_f.x);
        if ( wm_dfs(wm, pos_f, goal, seen, depth_limit, actions) ) {
            wm_destroy(wm);
            return true;
        }
    }

    // Test walking right
    if ( !seen[pos_r.y][pos_r.x] ) {
        //fprintf(stderr, "Add: (%d,%d)", pos_r.y, pos_r.x);
        if ( wm_dfs(wm, pos_r, goal, seen, depth_limit, actions) ) {
            wm_destroy(wm);
            return true;
        }
    }

    // Test walking left
    if ( !seen[pos_l.y][pos_l.x] ) {
        //fprintf(stderr, "Add: (%d,%d)", pos_l.y, pos_l.x);
        if ( wm_dfs(wm, pos_l, goal, seen, depth_limit, actions) ) {
            wm_destroy(wm);
            return true;
        }
    }

    // Test walking backward
    if ( !seen[pos_b.y][pos_b.x] ) {
        //fprintf(stderr, "Add: (%d,%d)", pos_b.y, pos_b.x);
        if ( wm_dfs(wm, pos_b, goal, seen, depth_limit, actions) ) {
            wm_destroy(wm);
            return true;
        }
    }

    wm_destroy(wm);

    // Restore seen to its old value
    if ( need_to_restore_seen ) {
        int i, j;
        for( i = 0; i < GRID_SIZE; i++ ) {
            for ( j = 0; j < GRID_SIZE; j++ ) {
                seen[i][j] = saved_seen[i][j];
            }
        }
    }
    
    return false;
}

bool wm_walk(struct WorldModel* wm, char* actions, Goal goal) {
    int depth;
    for( depth = 1; depth < 100; depth++ ) {
        bool seen[GRID_SIZE][GRID_SIZE] = {{0}};
        if ( wm_dfs(wm, wm->pos, goal, seen, depth, actions) ) {
            return true;
        }
    }

    actions[0] = '\0';

    return false;
}

 
bool wm_walk_test_permissible(struct WorldModel* wm, struct Pos pos, Goal goal) {
    char start_tile = wm_get_tile(wm, wm->pos);
    char cur_tile   = wm_get_tile(wm, pos);

    // Universal fails
    
    // We cant ever move into unknown or wall tiles
    if ( cur_tile == TILE_UNKNOWN || cur_tile == TILE_WALL ) {
        return false;
    }

    // We can't move into trees without an axe
    if ( cur_tile == TILE_TREE && !wm->axe ) {
        return false;
    }

    // We can't move into doors without a key
    if ( cur_tile == TILE_DOOR && !wm->key ) {
        return false;
    }

    // We can't move into water without a stone or a raft
    if ( cur_tile == TILE_WATER && !wm->raft &&
            wm->stones < 0 ) {
        return false;
    }
    
    // If we are exploring
    if ( goal == GOAL_EXPLORE || goal == GOAL_CHOP_GRAB) {
        // We can't enter or leave water
        if ( ( start_tile == TILE_WATER && cur_tile != TILE_WATER ) ||
             ( start_tile != TILE_WATER && cur_tile == TILE_WATER ) ) {
            return false;
        }

        // If we don't allow choppiing
        if ( goal == GOAL_EXPLORE && ( cur_tile == TILE_STONE || cur_tile == TILE_TREE ) ) {
            return false;
        }
    }

    return true;
};

bool wm_walk_test_goal(struct WorldModel* wm, Goal goal) {
    int i, j;

    switch(goal) {
        case GOAL_EXPLORE:
        case GOAL_CHOP_GRAB:
            for ( i = wm->pos.y - VIEW_DIST; i <= wm->pos.y + VIEW_DIST; i++ ) {
                for ( j = wm->pos.x - VIEW_DIST; j <= wm->pos.x + VIEW_DIST; j++ ) {
                    if ( wm->grid[i][j] == TILE_UNKNOWN ) {
                        return true;
                    }
                }
            }
            break;

        case GOAL_WIN:
            if ( wm->treasure && wm_get_tile(wm, wm->pos) == TILE_HOME ) {
                return true;
            }
            break;
    }
    return false;
}

