#pragma once

#define USE_DMA_TO_TFT

#include <SPI.h>
#include <TFT_eSPI.h>

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tft_spr_next = TFT_eSprite(&tft);

#define TFT_DARKERGREY  0x1042
#define TFT_DARKESTGREY  0x0821

#define T_BLOCK_SIZE  12
#define T_BOARD_WIDTH  10
#define T_BOARD_HEIGHT 20

uint8_t tetromino_layout[][16] = {  
  {0,0,0,0, 1,1,1,1, 0,0,0,0, 0,0,0,0}, {0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0}, {0,0,0,0, 0,0,0,0, 1,1,1,1, 0,0,0,0}, {0,1,0,0, 0,1,0,0, 0,1,0,0, 0,1,0,0}, // I
  {1,0,0,0, 1,1,1,0, 0,0,0,0, 0,0,0,0}, {0,1,1,0, 0,1,0,0, 0,1,0,0, 0,0,0,0}, {0,0,0,0, 1,1,1,0, 0,0,1,0, 0,0,0,0}, {0,1,0,0, 0,1,0,0, 1,1,0,0, 0,0,0,0}, // J
  {0,0,1,0, 1,1,1,0, 0,0,0,0, 0,0,0,0}, {0,1,0,0, 0,1,0,0, 0,1,1,0, 0,0,0,0}, {0,0,0,0, 1,1,1,0, 1,0,0,0, 0,0,0,0}, {1,1,0,0, 0,1,0,0, 0,1,0,0, 0,0,0,0}, // L
  {0,1,1,0, 0,1,1,0, 0,0,0,0, 0,0,0,0}, // O
  {0,1,1,0, 1,1,0,0, 0,0,0,0, 0,0,0,0}, {0,1,0,0, 0,1,1,0, 0,0,1,0, 0,0,0,0}, {0,0,0,0, 0,1,1,0, 1,1,0,0, 0,0,0,0}, {1,0,0,0, 1,1,0,0, 0,1,0,0, 0,0,0,0}, // S
  {0,1,0,0, 1,1,1,0, 0,0,0,0, 0,0,0,0}, {0,1,0,0, 0,1,1,0, 0,1,0,0, 0,0,0,0}, {0,0,0,0, 1,1,1,0, 0,1,0,0, 0,0,0,0}, {0,1,0,0, 1,1,0,0, 0,1,0,0, 0,0,0,0}, // T
  {1,1,0,0, 0,1,1,0, 0,0,0,0, 0,0,0,0}, {0,0,1,0, 0,1,1,0, 0,1,0,0, 0,0,0,0}, {0,0,0,0, 1,1,0,0, 0,1,1,0, 0,0,0,0}, {0,1,0,0, 1,1,0,0, 1,0,0,0, 0,0,0,0} // Z
};

struct tetromino_block {
  uint32_t block_colour,
    border_colour;
} tetromino_blocks[] = {
  { TFT_DARKERGREY, TFT_DARKESTGREY },
  { TFT_CYAN, TFT_BLUE },
  { TFT_BLUE, TFT_NAVY },
  { TFT_ORANGE, TFT_BROWN },
  { TFT_YELLOW, TFT_BROWN },
  { TFT_GREEN, TFT_DARKGREEN },
  { TFT_PURPLE, TFT_MAROON },
  { TFT_RED, TFT_MAROON }
};

struct tetromino 
{
  uint8_t layout[4];
  
  uint8_t block_id;
  uint8_t is_3x3;
  uint8_t is_flipped;
} tetrominoes[] = {
  {{0, 1, 2, 3}, 1, 0, 0 },
  {{4, 5, 6, 7}, 2, 1, 0 },
  {{8, 9, 10, 11}, 3, 0, 0},
  {{12, 12, 12, 12}, 4, 1, 0},
  {{13, 14, 15, 16}, 5, 1, 0},
  {{17, 18, 19, 20}, 6, 1, 0},
  {{21, 22, 23, 24}, 7, 1, 0}
};

uint8_t g_board[T_BOARD_WIDTH * T_BOARD_HEIGHT] = {0};

uint8_t gfx_get_board(uint8_t x, uint8_t y) {
  return g_board[y * T_BOARD_WIDTH + x];
}

void gfx_set_board(uint8_t x, uint8_t y, uint8_t v) 
{
  int address = y * T_BOARD_WIDTH + x;
  
  if (address >= 0 && address < T_BOARD_WIDTH * T_BOARD_HEIGHT)
    g_board[address] = v;
}

#define T_INTERSECT_NONE        0
#define T_INTERSECT_WELL_LEFT   1
#define T_INTERSECT_WELL_RIGHT  2
#define T_INTERSECT_WELL_BOTTOM 4

int gfx_test_intersection(uint8_t x, uint8_t y, uint8_t r, struct tetromino *t)
{
  uint8_t *_t = tetromino_layout[t->layout[r]];
  uint8_t size = (t->is_3x3) ? 3 : 4;

  int intersection_bits = T_INTERSECT_NONE;

  for (int _y = 0; _y < size; _y++) {
    for (int _x = 0; _x < size; _x++) {
      
      if (_t[_y * 4 + _x] != 0)
      {
        uint8_t __x = x + _x,
          __y = y + _y;        

        if (__x <= 0 || gfx_get_board(__x - 1, __y) != 0)
          intersection_bits |= T_INTERSECT_WELL_LEFT;
        
        if (__x >= T_BOARD_WIDTH - 1 || gfx_get_board(__x + 1, __y) != 0)
          intersection_bits |= T_INTERSECT_WELL_RIGHT;

        if (__y >= T_BOARD_HEIGHT - 1 || gfx_get_board(__x, __y + 1) != 0)
          intersection_bits |= T_INTERSECT_WELL_BOTTOM;
      }

    }
  }

  return intersection_bits;
}

void gfx_init() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  tft_spr_next.setColorDepth(16);
  tft_spr_next.createSprite(4 * T_BLOCK_SIZE, 4 * T_BLOCK_SIZE);
  tft_spr_next.fillSprite(TFT_BLACK);

  #ifdef USE_DMA_TO_TFT
  // DMA - should work with ESP32, STM32F2xx/F4xx/F7xx processors
  // NOTE: >>>>>> DMA IS FOR SPI DISPLAYS ONLY <<<<<<
  tft.initDMA(); // Initialise the DMA engine (tested with STM32F446 and STM32F767)
  #endif
}

void gfx_clear_board() {
  for (int i=0; i < T_BOARD_WIDTH * T_BOARD_HEIGHT; i++)
    g_board[i] = 0;

  tft.fillScreen(TFT_BLACK);
}

void gfx_prepare_gameover() {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("GAME OVER", tft.width()/2, tft.height()/2, 4);
}

void gfx_draw_gameover() {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("GAME OVER", tft.width()/2, tft.height()/2, 4);
}

void gfx_prepare_title() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("TETRIS!", tft.width()/2, tft.height()/2, 4);
  tft.drawString("Press any button to start!", tft.width()/2, tft.height()/2 + 64, 2);
}

void gfx_draw_title() {
  // tft.fillScreen(TFT_BLACK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("TETRIS!", tft.width()/2, tft.height()/2, 4);
  tft.drawString("Press any button to start!", tft.width()/2, tft.height()/2 + 64, 2);
}

void draw_block(uint8_t x, uint8_t y, uint8_t block_id) 
{
  tft.drawRect(x * T_BLOCK_SIZE, y * T_BLOCK_SIZE, T_BLOCK_SIZE, T_BLOCK_SIZE, tetromino_blocks[block_id].border_colour);
  tft.fillRect(x * T_BLOCK_SIZE + 2, y * T_BLOCK_SIZE + 2, T_BLOCK_SIZE - 4, T_BLOCK_SIZE - 4, tetromino_blocks[block_id].block_colour);
}

void gfx_draw_board() 
{
  for (int _y = 0; _y < T_BOARD_HEIGHT; _y++) {
    for (int _x = 0; _x < T_BOARD_WIDTH; _x++) {
      draw_block(_x, _y, gfx_get_board(_x, _y));
    }
  }
}

void draw_tetromino(uint8_t x, uint8_t y, uint8_t r, struct tetromino *t) 
{
  uint8_t *_t = tetromino_layout[t->layout[r]];
  uint8_t size = (t->is_3x3) ? 3 : 4;

  for (int _y = 0; _y < size; _y++) {
    for (int _x = 0; _x < size; _x++) {
      if (_t[_y * 4 + _x] != 0) {
        draw_block(x + _x, y + _y, t->block_id);
      }
    }
  }
}

void draw_next_tetromino(uint8_t r, struct tetromino *t) 
{
  tft_spr_next.fillSprite(TFT_BLACK);

  uint8_t *_t = tetromino_layout[t->layout[r]];

  for (int _y = 0; _y < 4; _y++) {
    for (int _x = 0; _x < 4; _x++) {
      if (_t[_y * 4 + _x] != 0) {
          tft_spr_next.drawRect(_x * T_BLOCK_SIZE, _y * T_BLOCK_SIZE, T_BLOCK_SIZE, T_BLOCK_SIZE, tetromino_blocks[t->block_id].border_colour);
          tft_spr_next.fillRect(_x * T_BLOCK_SIZE + 2, _y * T_BLOCK_SIZE + 2, T_BLOCK_SIZE - 4, T_BLOCK_SIZE - 4, tetromino_blocks[t->block_id].block_colour);
      } else {
          tft_spr_next.fillRect(_x * T_BLOCK_SIZE, _y * T_BLOCK_SIZE, T_BLOCK_SIZE, T_BLOCK_SIZE, TFT_BLACK);
      }
    }
  }
}

bool gfx_stack_tetromino(uint8_t x, uint8_t y, uint8_t r, struct tetromino *t) 
{
  uint8_t *_t = tetromino_layout[t->layout[r]];
  uint8_t size = (t->is_3x3) ? 3 : 4;

  bool game_lost = false;

  for (int _y = 0; _y < size; _y++) {
    for (int _x = 0; _x < size; _x++) {
      if (_t[_y * 4 + _x] != 0) {
        gfx_set_board(x + _x, y + _y, t->block_id);
        if (y + _y == 0) game_lost = true;
      }
    }
  }

  return game_lost;
}

int gfx_remove_lines(uint8_t y) 
{
  int lines_removed = 0;

  int max_y = min(y + 4, T_BOARD_HEIGHT);

  for (int _y = y; _y < max_y; _y++) 
  {
    int _x;
    
    for (_x = 0; _x < T_BOARD_WIDTH; _x++)
      if (gfx_get_board(_x, _y) == 0) break;

    if (_x == T_BOARD_WIDTH) {
      for (int __y = _y - 1; __y >= 0; __y--) {
        for (int __x = 0; __x < T_BOARD_WIDTH; __x++) {
          gfx_set_board(__x, __y + 1, gfx_get_board(__x, __y));
        }
      }
      
      for (int __x = 0; __x < T_BOARD_WIDTH; __x++)
        gfx_set_board(__x, 0, 0);

      lines_removed++;

      gfx_draw_board();

      delay(200);
    }
  }

  return lines_removed;
}

void gfx_draw_score()
{
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("Score", 140, 48 - 32, 2); 
  tft.drawString("Lines", 140, 96 - 32, 2); 
  tft.drawString("Level", 140, 144 - 32, 2);
  tft.drawString("Next", 140, 192 - 32, 2); 
}

void gfx_update_score(int score) {
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawNumber(score, 220, 40, 2); 
}

void gfx_update_lines(int lines) {
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawNumber(lines, 220, 88, 2); 
}

void gfx_update_level(int level) {
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawNumber(level, 220, 136, 2); 
}

void gfx_draw_next(uint8_t next)
{
  draw_next_tetromino(0, tetrominoes + next);
  tft_spr_next.pushSprite(160, 192);
}
