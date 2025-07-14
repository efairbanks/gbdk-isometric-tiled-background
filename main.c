#include <gb/gb.h>
#include <stdint.h>
#include <string.h>
#include <rand.h>

#include "data.h"

// gets a tile from the map using east-west, south-north coordinates
// returns 0 if the tile is empty, 1 if the tile is solid
uint8_t get_map(int8_t ew, int8_t sn, int8_t ewOffset, int8_t snOffset) {
    ew += ewOffset;
    sn += snOffset;
    if(ew < 0 || sn < 0 || ew >= 32 || sn >= 32) return 0;
    uint16_t byteIndex = (sn * 4) + (ew >> 3);
    uint8_t bitPosition = 7 - (ew & 0x07);
    return (map[byteIndex] >> bitPosition) & 0x01;
}

// sets a tile in the map using east-west, south-north coordinates
// value should be 0 or 1
void set_map(int8_t ew, int8_t sn, uint8_t value) {
    if(ew < 0 || sn < 0 || ew >= 32 || sn >= 32) return;
    uint16_t byteIndex = (sn * 4) + (ew >> 3);
    uint8_t bitPosition = 7 - (ew & 0x07);
    uint8_t bitMask = (1 << bitPosition);
    if (value) {
        map[byteIndex] |= bitMask;
    } else {
        map[byteIndex] &= ~bitMask;
    }
}

/**
 * Generate a procedural map with rooms and paths between them
 * @param num_rooms Number of rooms to generate (1-8 recommended)
 * @param min_size Minimum room size (2-4 recommended)
 * @param max_size Maximum room size (3-6 recommended)
 */
void generate_map(uint8_t num_rooms, uint8_t min_size, uint8_t max_size) {
    // Clear the map first
    for(uint16_t i = 0; i < 4*32; i++) {
        map[i] = 0;
    }
    
    // Ensure parameters are in reasonable ranges
    if (num_rooms > 10) num_rooms = 10;
    if (min_size < 2) min_size = 2;
    if (max_size > 8) max_size = 8;
    if (min_size > max_size) min_size = max_size;
    
    // Store room positions and sizes for later path generation
    int8_t room_x[10], room_y[10], room_w[10], room_h[10];
    
    // Generate rooms
    for (uint8_t r = 0; r < num_rooms; r++) {
        // Room size
        uint8_t width = min_size + (rand() % (max_size - min_size + 1));
        uint8_t height = min_size + (rand() % (max_size - min_size + 1));
        
        // Room position (with some padding from edges)
        int8_t x = 2 + (rand() % (28 - width));
        int8_t y = 2 + (rand() % (28 - height));
        
        // Save room info
        room_x[r] = x;
        room_y[r] = y;
        room_w[r] = width;
        room_h[r] = height;
        
        // Fill the room (solid blocks)
        for (int8_t j = 0; j < height; j++) {
            for (int8_t i = 0; i < width; i++) {
                set_map(x + i, y + j, 1);
            }
        }
    }
    
    // Connect rooms with paths
    for (uint8_t r = 0; r < num_rooms - 1; r++) {
        // Calculate center points of current and next room
        int8_t x1 = room_x[r] + (room_w[r] / 2);
        int8_t y1 = room_y[r] + (room_h[r] / 2);
        int8_t x2 = room_x[r+1] + (room_w[r+1] / 2);
        int8_t y2 = room_y[r+1] + (room_h[r+1] / 2);
        
        // Create L-shaped path between rooms
        // Move horizontally first
        int8_t dx = (x1 < x2) ? 1 : -1;
        for (int8_t x = x1; x != x2; x += dx) {
            set_map(x, y1, 1);
        }
        
        // Then move vertically
        int8_t dy = (y1 < y2) ? 1 : -1;
        for (int8_t y = y1; y != y2 + dy; y += dy) {
            set_map(x2, y, 1);
        }
    }
    
    // Optional: Add some random noise/decorations
    for (uint8_t i = 0; i < 30; i++) {
        int8_t x = rand() % 32;
        int8_t y = rand() % 32;
        
        // Don't overwrite existing tiles with a 50% chance
        if (!get_map(x, y, 0, 0) && (rand() % 2)) {
            set_map(x, y, 1);
        }
    }
}

/*inline */void draw_tile(int8_t x, int8_t y, int8_t ewOffset, int8_t snOffset, int8_t bgXOffset, int8_t bgYOffset) {
    int8_t ew = (x>>2) - (y>>1);
    int8_t sn = (x>>2) + (y>>1);

    int8_t currentTile = get_map(ew, sn, ewOffset, snOffset);
    int8_t northTile = get_map(ew, sn-1, ewOffset, snOffset);
    int8_t eastTile = get_map(ew+1, sn, ewOffset, snOffset);
    int8_t westTile = get_map(ew-1, sn, ewOffset, snOffset);
    int8_t southTile = get_map(ew, sn+1, ewOffset, snOffset);

    x += bgXOffset;
    y += bgYOffset;

    // draw north (UL)
    if(currentTile != northTile) {
        if(currentTile > northTile) {
            set_bkg_tile_xy(x&0x1F, y&0x1F, 1);
            set_bkg_tile_xy((x+1)&0x1F, y&0x1F, 2);
        } else {
            set_bkg_tile_xy(x&0x1F, y&0x1F, 1+8);
            set_bkg_tile_xy((x+1)&0x1F, y&0x1F, 2+8);
        }
    } else {
        if(currentTile) {
            set_bkg_tile_xy(x&0x1F, y&0x1F, 0);
            set_bkg_tile_xy((x+1)&0x1F, y&0x1F, 0);
        } else {
            set_bkg_tile_xy(x&0x1F, y&0x1F, 17);
            set_bkg_tile_xy((x+1)&0x1F, y&0x1F, 17);
        }
    }
    // draw east (UR)
    if(currentTile != eastTile) {
        if(currentTile > eastTile) {
            set_bkg_tile_xy((x+2)&0x1F, y&0x1F, 3);
            set_bkg_tile_xy((x+3)&0x1F, y&0x1F, 4);
        } else {
            set_bkg_tile_xy((x+2)&0x1F, y&0x1F, 3+8);
            set_bkg_tile_xy((x+3)&0x1F, y&0x1F, 4+8);
        }
    } else {
        if(currentTile) {
            set_bkg_tile_xy((x+2)&0x1F, y&0x1F, 0);
            set_bkg_tile_xy((x+3)&0x1F, y&0x1F, 0);
        } else {
            set_bkg_tile_xy((x+2)&0x1F, y&0x1F, 17);
            set_bkg_tile_xy((x+3)&0x1F, y&0x1F, 17);
        }
    }
    // draw west (LR)
    if(currentTile != westTile) {
        if(currentTile > westTile) {
            set_bkg_tile_xy((x)&0x1F, (y+1)&0x1F, 5);
            set_bkg_tile_xy((x+1)&0x1F, (y+1)&0x1F, 6);
        } else {
            set_bkg_tile_xy((x)&0x1F, (y+1)&0x1F, 5+8);
            set_bkg_tile_xy((x+1)&0x1F, (y+1)&0x1F, 6+8);
        }
    } else {
        if(currentTile) {
            set_bkg_tile_xy((x)&0x1F, (y+1)&0x1F, 0);
            set_bkg_tile_xy((x+1)&0x1F, (y+1)&0x1F, 0);
        } else {
            set_bkg_tile_xy((x)&0x1F, (y+1)&0x1F, 17);
            set_bkg_tile_xy((x+1)&0x1F, (y+1)&0x1F, 17);
        }
    }
    // draw south (LL)
    if(currentTile != southTile) {
        if(currentTile > southTile) {
            set_bkg_tile_xy((x+2)&0x1F, (y+1)&0x1F, 7);
            set_bkg_tile_xy((x+3)&0x1F, (y+1)&0x1F, 8);
        } else {
            set_bkg_tile_xy((x+2)&0x1F, (y+1)&0x1F, 7+8);
            set_bkg_tile_xy((x+3)&0x1F, (y+1)&0x1F, 8+8);
        }
    } else {
        if(currentTile) {
            set_bkg_tile_xy((x+2)&0x1F, (y+1)&0x1F, 0);
            set_bkg_tile_xy((x+3)&0x1F, (y+1)&0x1F, 0);
        } else {
            set_bkg_tile_xy((x+2)&0x1F, (y+1)&0x1F, 17);
            set_bkg_tile_xy((x+3)&0x1F, (y+1)&0x1F, 17);
        }
    }
}

enum {
    TILE_UPDATE_NONE    = 0,
    TILE_UPDATE_RIGHT   = 1,
    TILE_UPDATE_LEFT    = 2,
    TILE_UPDATE_DOWN    = 4,
    TILE_UPDATE_UP      = 8,
    TILE_UPDATE_ALL     = 16
} _TILE_UPDATE_MASK;

void draw_tiles(int8_t tileUpdateMask, int8_t ewOffset, int8_t snOffset, int8_t bgXOffset, int8_t bgYOffset) {
    if(tileUpdateMask == TILE_UPDATE_NONE) return;
    if(tileUpdateMask == TILE_UPDATE_ALL) {
        for(int y=0; y<18; y+=2) {
            for(int x=0; x<20; x+=4) {
                draw_tile(x, y, ewOffset, snOffset, bgXOffset, bgYOffset);
            }
        }
    } else {
        if(tileUpdateMask & TILE_UPDATE_RIGHT) {
            for(int i=0; i<20; i+=2) {
                draw_tile(20, i, ewOffset, snOffset, bgXOffset, bgYOffset);
            }
        }
        if(tileUpdateMask & TILE_UPDATE_LEFT) {
            for(int i=0; i<20; i+=2) {
                draw_tile(0, i, ewOffset, snOffset, bgXOffset, bgYOffset);
            }
        }
        if(tileUpdateMask & TILE_UPDATE_DOWN) {
            for(int i=0; i<22; i+=4) {
                draw_tile(i, 18, ewOffset, snOffset, bgXOffset, bgYOffset);
            }
        }
        if(tileUpdateMask & TILE_UPDATE_UP) {
            for(int i=0; i<22; i+=4) {
                draw_tile(i, 0, ewOffset, snOffset, bgXOffset, bgYOffset);
            }
        }
    }
}

void main(void)
{
    uint8_t key = 0, lastKey = 0;

    int ewCamPos = 0, snCamPos = 0;
    uint8_t bgTileXOffset = 0, bgTileYOffset = 0;

    set_bkg_data(1, 16, isometric_blackbg_tiles);
    set_bkg_data(17, 1, black_tile);
    set_bkg_data(18, 36, numericTiles);
    for(int x=0; x<20; x++) {
        for(int y=0; y<18; y++) {
            set_bkg_tile_xy(x, y, 0);
        }
    }

    // initrand(66);
    // for(int i=0; i<4*32; i++) map[i] = rand();
    // for(int i=0; i<4*32; i++) map[i] = rand();

    generate_map(5, 2, 4);

    vsync();
    draw_tiles(TILE_UPDATE_ALL, 0, 0, 0, 0);

    // Loop forever
    while(1) {
        lastKey = key;
        key = joypad();
        
        // Generate new map when START is pressed
        if((key & J_START) && !(lastKey & J_START)) {
            // Use a different seed each time for variety
            initrand(DIV_REG);
            // Generate new map with slightly different parameters each time
            uint8_t num_rooms = 3 + (rand() % 5); // 3-7 rooms
            uint8_t min_size = 2 + (rand() % 2);   // 2-3 min size
            uint8_t max_size = 4 + (rand() % 3);   // 4-6 max size
            generate_map(num_rooms, min_size, max_size);
        }

        // Movement controls
        uint8_t tileUpdateMask = TILE_UPDATE_NONE;
        if(key & J_LEFT) {
            ewCamPos += -1;
            tileUpdateMask |= TILE_UPDATE_LEFT | TILE_UPDATE_DOWN;
            // bgTileXOffset = (bgTileXOffset-4)&0x1F;
            // bgTileYOffset = (bgTileYOffset+2)&0x1F;
        }
        if(key & J_RIGHT) {
            ewCamPos += 1;
            tileUpdateMask |= TILE_UPDATE_RIGHT | TILE_UPDATE_UP;
            // bgTileXOffset = (bgTileXOffset+4)&0x1F;
            // bgTileYOffset = (bgTileYOffset-2)&0x1F;
        }
        if(key & J_UP) {
            snCamPos += -1;
            tileUpdateMask |= TILE_UPDATE_LEFT | TILE_UPDATE_UP;
            // bgTileXOffset = (bgTileXOffset-4)&0x1F;
            // bgTileYOffset = (bgTileYOffset-2)&0x1F;
        }
        if(key & J_DOWN) {
            snCamPos += 1;
            tileUpdateMask |= TILE_UPDATE_RIGHT | TILE_UPDATE_DOWN;
            // bgTileXOffset = (bgTileXOffset+4)&0x1F;
            // bgTileYOffset = (bgTileYOffset+2)&0x1F;
        }

        vsync();


        bgTileXOffset = (ewCamPos<<1) + (snCamPos<<1);
        bgTileYOffset = -(ewCamPos) + (snCamPos);
        draw_tiles(tileUpdateMask, ewCamPos, snCamPos, bgTileXOffset, bgTileYOffset);
        move_bkg(bgTileXOffset*8, bgTileYOffset*8);

        // set_bkg_tile_xy(0, 0, get_map(0,0)+17);

        SHOW_BKG;
    }
}
