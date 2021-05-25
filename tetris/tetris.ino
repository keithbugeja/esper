#include "graphics.h"

/*
 * Pin definitions
 */
const int OP_BUZZER = 13;
const int IP_BUTTON_0 = 22;
const int IP_BUTTON_1 = 34;
const int IP_BUTTON_2 = 35;
const int IP_STICK_Y = 12;
const int IP_STICK_X = 14;

/*
 * Events
 */
bool paint_board;

/*
 * Game state
 */
int score_scheme[] = { 100, 300, 500, 800 };

uint8_t tetromino_x, 
  tetromino_y, 
  tetromino_r, 
  tetromino_d, 
  tetromino_id, 
  tetromino_next;

int game_level,
  game_score,
  game_lines;

int input_stick_x, 
  input_stick_y;

bool input_invert_x, 
  input_invert_y;

bool input_button_0, 
  input_button_1, 
  input_button_2;

int drop_cooldown, 
  movement_cooldown,
  rotation_cooldown;

int intersection_bits;

long delta_time, last_millis;

uint8_t za, zb, zc, zx;

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) rng()
{
  zx++;
  za = (za^zc^zx);
  zb = (zb+za);
  zc = ((zc+(zb>>1))^za);
  return zc;
}

void reset_game() 
{
  tetromino_x = 3;
  tetromino_y = 0;
  tetromino_r = 0;
  tetromino_d = 55;
  tetromino_id = rng() % 7;
  tetromino_next = rng() % 7;

  game_level = 1;
  game_score = 0;
  game_lines = 0;

  gfx_clear_board();
}

bool game_loop()
{
  delta_time = millis() - last_millis;
  last_millis = millis();

  if (rotation_cooldown > 0)
    rotation_cooldown -= delta_time;

  if (movement_cooldown > 0)
    movement_cooldown -= delta_time;

  if (drop_cooldown > 0)
    drop_cooldown -= delta_time;

  analogReadResolution(10);
  
  int stick_x = analogRead(IP_STICK_X); 
  if (stick_x < 512 - 128) stick_x = -1; 
  else if (stick_x > 512 + 128) stick_x = 1; 
  else { stick_x = 0; movement_cooldown = 0; }
  if (input_invert_x) stick_x =- stick_x;

  int stick_y = analogRead(IP_STICK_Y); 
  if (stick_y < 512 - 128) stick_y = -1; 
  else if (stick_y > 512 + 128) stick_y = 1;
  else { stick_y = 0; drop_cooldown = 0; }
  if (input_invert_y) stick_y =- stick_y;

  bool button_0 = digitalRead(IP_BUTTON_0);
  bool button_1 = digitalRead(IP_BUTTON_1);
  bool button_2 = digitalRead(IP_BUTTON_2);

  // Todo: Validate rotations
  if (rotation_cooldown <= 0) {
    if (button_1) {
      tetromino_r = (tetromino_r + 1) & 0x03;
      paint_board = true;
      rotation_cooldown = 200; 
    }

    if (button_2) {
      tetromino_r = (tetromino_r - 1) & 0x03;
      paint_board = true;
      rotation_cooldown = 200;
    }
  }

  // Check if we can move left or right
  intersection_bits = gfx_test_intersection(tetromino_x, tetromino_y, tetromino_r, tetrominoes + tetromino_id);

  if (movement_cooldown <= 0) {
    if (stick_x < 0) // digitalRead(button_on)) 
    {
      if ((intersection_bits & T_INTERSECT_WELL_RIGHT) == 0) 
      {
        tetromino_x++;
        paint_board = true;
        movement_cooldown = 200;
      }
    }
    else if (stick_x > 0) // digitalRead(button_off)) 
    {
      if ((intersection_bits & T_INTERSECT_WELL_LEFT) == 0) 
      {
        tetromino_x--;
        paint_board = true;
        movement_cooldown = 200;
      }
    }
  }

  // Refresh intersection bits just in case tetromino moved
  intersection_bits = gfx_test_intersection(tetromino_x, tetromino_y, tetromino_r, tetrominoes + tetromino_id);

  if (tetromino_d > 0) {
    if (stick_y > 0 && drop_cooldown <= 0) {
      tetromino_d = 0;
      drop_cooldown = 50;
    } else 
      tetromino_d--;
  } else {
    if (intersection_bits & T_INTERSECT_WELL_BOTTOM)
    {
      // Did player lose?
      if (gfx_stack_tetromino(tetromino_x, tetromino_y, tetromino_r, tetrominoes + tetromino_id)) 
        return false;

      int lines = gfx_remove_lines(tetromino_y);
      if (lines > 0) 
      {
        game_score += score_scheme[lines - 1] * game_level;
        game_lines += lines;
      }

      tetromino_x = 3;
      tetromino_y = 0;
      tetromino_r = 0;
      tetromino_id = tetromino_next;
      tetromino_next = rng() % 7;
    }
    else
      tetromino_y++;

    tetromino_d = 55 - game_level * 5;
    paint_board = true;
  }

  if (paint_board == true) 
  {
    gfx_draw_board();
    gfx_draw_score();
    gfx_update_score(game_score);
    gfx_update_lines(game_lines);
    gfx_update_level(game_level);
    gfx_draw_next(tetromino_next);

    draw_tetromino(tetromino_x, tetromino_y, tetromino_r, tetrominoes + tetromino_id);

    paint_board = false;
  }

  // if (buzzer_enabled) 
  // {
  //     digitalWrite(OP_BUZZER,HIGH);
  //     delay(1);//wait for 1ms
  //     digitalWrite(OP_BUZZER,LOW);
  //     delay(1);//wait for 1ms
  // }

  return true;
}

void setup() 
{
  Serial.begin(115200);

  gfx_init();

  pinMode(OP_BUZZER, OUTPUT); //initialize the buzzer pin as an output

  pinMode(IP_BUTTON_0, INPUT);
  pinMode(IP_BUTTON_1, INPUT);
  pinMode(IP_BUTTON_2, INPUT);

  input_invert_x = true;
  input_invert_y = true;

  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);

  reset_game();

  gfx_draw_title();
}

/*
 * Start game on title screen
 */
 
int game_state = 2;

void loop()
{
  if (game_state == 0) {
    // If game over, switch to game_over screen
    if (game_loop() == false) {
      gfx_prepare_gameover();
      
      game_state = 1;
    }
  } else {
    bool button_0 = digitalRead(IP_BUTTON_0);
    bool button_1 = digitalRead(IP_BUTTON_1);
    bool button_2 = digitalRead(IP_BUTTON_2);

    if (game_state == 1) {
      gfx_draw_gameover();

      if (button_1 || button_2) {
        gfx_prepare_title();

        game_state = 2;
        delay(100);
      }
    } else {
      gfx_draw_title();

      if (button_1 || button_2) {
        reset_game();

        game_state = 0;
        delay(50);
      }
    }
  }

  delay(15);
}