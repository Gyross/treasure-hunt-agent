#include <stdbool.h>

// We define home as the center of the grid
#define HOME_POS 80
#define GRID_SIZE 2*HOME_POS + 1
#define VIEW_DIST 2
#define VIEW_SIZE 2*VIEW_DIST + 1

// Tiles
#define TILE_UNKNOWN    '?'
#define TILE_HOME       'H'
#define TILE_LAND       ' '
#define TILE_USED_STONE 'O'
#define TILE_WATER      '~'
#define TILE_TREE       'T'
#define TILE_DOOR       '-'
#define TILE_WALL       '*'
#define TILE_AXE        'a'
#define TILE_KEY        'k'
#define TILE_STONE      'o'
#define TILE_TREASURE   '$'

// Action
#define ACTION_FORWARD 'F'
#define ACTION_LEFT    'L'
#define ACTION_RIGHT   'R'
#define ACTION_CHOP    'C'
#define ACTION_UNLOCK  'U'

// Direction
enum Direction{ DIRECTION_UP, 
                 DIRECTION_LEFT, 
                 DIRECTION_DOWN, 
                 DIRECTION_RIGHT };

typedef int Direction;

Direction dir_turn_right( Direction dir );
Direction dir_turn_left( Direction dir );

struct Pos;

struct Pos pos_set( int x, int y );
struct Pos pos_forward_rel( struct Pos pos, Direction rel_dir );

// World model
struct WorldModel;

struct WorldModel* wm_create(char view[VIEW_SIZE][VIEW_SIZE]);
void wm_destroy(struct WorldModel* wm);

void wm_take_action(struct WorldModel* wm, char action);

char wm_get_tile(struct WorldModel* wm, struct Pos pos);
void wm_set_tile(struct WorldModel* wm, struct Pos pos, char tile_val);