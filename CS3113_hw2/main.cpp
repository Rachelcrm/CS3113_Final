/**
* Author: Rachel Chen
* Assignment: Platformer
* Date due: 2024-04-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include </Users/rachelchen/Desktop/XcodeFrameworks/SDL2_mixer.framework/Versions/A/Headers/SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Menu.h"

// ––––– CONSTANTS ––––– //
bool endgame = false;
bool pause = false;
bool collide = false;

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.2f,
            BG_BLUE    = 0.2f,
            BG_GREEN   = 0.2f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

bool g_game_is_running = true;
bool pressing_left = false;
bool pressing_right = false;
bool pressing_up = false;
bool pressing_down = false;
bool collide_safe = false;
bool collide_danger = false;
bool collide_bullet = false;
bool collide_upper_wall = false,
     collide_lower_wall = false,
     collide_left_wall = false,
     collide_right_wall = false;
bool enemy1_go_left = true;
bool shoot_second = false;

float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;
float acceleration_to_right = 0.0f;
float acceleration_to_up = 0.0f;
int lives = 5;
//level1
int eat_all_legs = 3;
bool eat_leg1 = false;
bool eat_leg2 = false;
bool eat_leg3 = false;
//level2
int bullet_hold = 0; //if player absorbs a bullet and then dies, he will still hold the bullet
bool is_absorb = false;
bool attack = false;
bool boss_go_left = true;
bool enemy_left = 2;
bool boss_lives = 2;
bool absorbed_2 = false;
bool absorbed_3 = false;
bool absorbed_4 = false;
bool press_space = false;
int check_end = 0;

const int   CD_QUAL_FREQ    = 44100,
            AUDIO_CHAN_AMT  = 2,
            AUDIO_BUFF_SIZE = 4096;

Scene  *g_current_scene;
Menu *g_menu;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;
Scene  *g_levels[4];
SDL_Window* g_display_window;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

Mix_Music* g_music;
Mix_Chunk* g_bouncing_sfx;

const char  BGM_FILEPATH[]              = "/Users/rachelchen/Desktop/CS3113_Hw3/Paradise_Found.mp3",
            BOUNCING_SFX_FILEPATH[]     = "/Users/rachelchen/Desktop/CS3113_Hw3/attack.wav";
const int   LOOP_FOREVER     = -1;

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    
    g_display_window = SDL_CreateWindow("Welcome!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    g_menu = new Menu();
    g_levels[0] = g_menu;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    switch_to_scene(g_levels[3]);
    
    Mix_OpenAudio(
        CD_QUAL_FREQ,
        MIX_DEFAULT_FORMAT,
        AUDIO_CHAN_AMT,
        AUDIO_BUFF_SIZE
    );
    
    //Set BGM
    g_current_scene->m_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(Mix_LoadMUS(BGM_FILEPATH), LOOP_FOREVER);
    Mix_VolumeMusic(MIX_MAX_VOLUME);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //UI messages
    
}

void process_input()
{
    if (g_current_scene != g_menu){
        // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
        g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));
        
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
                    // End game
                case SDL_QUIT:
                case SDL_WINDOWEVENT_CLOSE:
                    g_game_is_running = false;
                    break;
                    
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_q:
                            // Quit the game with a keystroke
                            g_game_is_running = false;
                            break;
                            
                        default:
                            break;
                    }
                    
                default:
                    break;
            }
        }
        
        const Uint8* key_state = SDL_GetKeyboardState(NULL);
        
        if (key_state[SDL_SCANCODE_LEFT])
        {
            g_current_scene->m_state.player->m_movement.x = -1.0f;
            g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->LEFT];
            pressing_left = true;
            pressing_up = false;
            pressing_down = false;
            pressing_right = false;
        }
        else if (key_state[SDL_SCANCODE_RIGHT])
        {
            g_current_scene->m_state.player->m_movement.x = 1.0f;
            g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->RIGHT];
            pressing_left = false;
            pressing_up = false;
            pressing_down = false;
            pressing_right = true;
        }
        else if (key_state[SDL_SCANCODE_UP])
        {
            g_current_scene->m_state.player->m_movement.y = 2.0f;
            g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->UP];
            pressing_left = false;
            pressing_up = true;
            pressing_down = false;
            pressing_right = false;
        }
        else if (key_state[SDL_SCANCODE_DOWN])
        {
            g_current_scene->m_state.player->m_movement.y = -2.0f;
            g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->DOWN];
            pressing_left = false;
            pressing_up = false;
            pressing_down = true;
            pressing_right = false;
        }
        if (key_state[SDL_SCANCODE_A])
        {
            if (g_current_scene == g_levels[2]){
                is_absorb = true;
            }
        }
        if (key_state[SDL_SCANCODE_SPACE]){
            Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
            attack = true;
        }
        // This makes sure that the player can't move faster diagonally
        if (glm::length(g_current_scene->m_state.player->m_movement) > 1.0f)
        {
            g_current_scene->m_state.player->m_movement = glm::normalize(g_current_scene->m_state.player->m_movement);
        }
    }
}

void update()
{
    if (!endgame){
            // ————— DELTA TIME ————— //
            float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
            float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
            g_previous_ticks = ticks;
            
            // ————— FIXED TIMESTEP ————— //
            // STEP 1: Keep track of how much time has passed since last step
            delta_time += g_time_accumulator;
            
            // STEP 2: Accumulate the ammount of time passed while we're under our fixed timestep
            if (delta_time < FIXED_TIMESTEP)
            {
                g_time_accumulator = delta_time;
                return;
            }
            
            // STEP 3: Once we exceed our fixed timestep, apply that elapsed time into the objects' update function invocation
            while (delta_time >= FIXED_TIMESTEP)
            {
                g_current_scene->update(FIXED_TIMESTEP);
                delta_time -= FIXED_TIMESTEP;
            }
            
            // Update acceleration based on time
            if (pressing_up) {
                g_current_scene -> m_state.player->set_velocity(glm::vec3(0.0f, 2.0f, 0.0f));
            }
            else if (pressing_down) {
                g_current_scene -> m_state.player->set_velocity(glm::vec3(0.0f, -2.0f, 0.0f));
            }
            if (pressing_left) {
                g_current_scene -> m_state.player->set_velocity(glm::vec3(-1.0f, 0.0f, 0.0f));
            }
            else if (pressing_right) {
                g_current_scene -> m_state.player->set_velocity(glm::vec3(1.0f, 0.0f, 0.0f));
            }
        
        if (g_current_scene == g_levels[1]) {
            g_current_scene -> m_state.enemies[4].set_velocity(glm::vec3(0.0f, -2.0f, 0.0f));
            if ((g_current_scene -> m_state.enemies[5].get_position().x == -10.0f)&&(g_current_scene -> m_state.enemies[4].get_position().y < -3.0f)){
                g_current_scene -> m_state.enemies[5].set_position(g_current_scene -> m_state.enemies[0].get_position());
                g_current_scene -> m_state.enemies[5].set_velocity(glm::vec3(0.0f, -2.0f, 0.0f));
            }
            if ((g_current_scene -> m_state.enemies[6].get_position().x == -10.0f)&&(g_current_scene -> m_state.enemies[5].get_position().y < -1.0f)){
                g_current_scene -> m_state.enemies[6].set_position(g_current_scene -> m_state.enemies[0].get_position());
                g_current_scene -> m_state.enemies[6].set_velocity(glm::vec3(0.0f, -2.0f, 0.0f));
            }
            
            for (size_t ind = 4; ind < 7; ++ind){
                if (g_current_scene -> m_state.enemies[ind].get_position().y <= -5.5f) {g_current_scene -> m_state.enemies[ind].set_position(g_current_scene -> m_state.enemies[0].get_position());
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(0.0f, -2.0f, 0.0f));}
                if (g_current_scene -> m_state.enemies[ind].get_position().x >= 1.0f && g_current_scene -> m_state.enemies[ind].get_position().x < 4.0f &&  g_current_scene -> m_state.enemies[ind].get_position().y < -0.9f) {
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(-10.0f, 0.0f, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(0.0f, -2.0f, 0.0f));
                }
            }
            
            for (size_t ind = 1; ind < 7-3; ++ind) {
                if (g_current_scene -> m_state.enemies[ind].get_position().y <-3.5f) {
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(g_current_scene -> m_state.enemies[ind].get_velocity().x, 1.5f, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(g_current_scene -> m_state.enemies[ind].get_position().x, -3.45f, 0.0f));
                    if (g_current_scene ->m_state.enemies[ind].get_velocity().x < 0) g_current_scene -> m_state.enemies[ind].m_speed = -1.0f;
                    else g_current_scene -> m_state.enemies[ind].m_speed = 1.0f;
                }
                if (g_current_scene -> m_state.enemies[ind].get_position().y > 3.5f) {
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(g_current_scene -> m_state.enemies[ind].get_velocity().x, -1.5f, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(g_current_scene -> m_state.enemies[ind].get_position().x, 3.45f, 0.0f));
                    if (g_current_scene ->m_state.enemies[ind].get_velocity().x < 0) g_current_scene -> m_state.enemies[ind].m_speed = -1.0f;
                    else g_current_scene -> m_state.enemies[ind].m_speed = 1.0f;
                }
                if (g_current_scene -> m_state.enemies[ind].get_position().x > 4.6f) {
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(-1.0f, g_current_scene -> m_state.enemies[ind].get_velocity().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(4.55f, g_current_scene -> m_state.enemies[ind].get_position().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].m_speed = -1.0f;
                    
                }
                if (g_current_scene -> m_state.enemies[ind].get_position().x < -4.6f) {
                    g_current_scene -> m_state.enemies[ind].set_movement(glm::vec3(1.0f, g_current_scene -> m_state.enemies[ind].get_velocity().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(-4.55f, g_current_scene -> m_state.enemies[ind].get_position().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].m_speed = 1.0f;
                }
            }
            
            for (size_t ind = 4; ind < 7; ++ind){
                if (abs(g_current_scene -> m_state.player->get_position().x - g_current_scene -> m_state.enemies[ind].get_position().x) < 0.5
                    && abs(g_current_scene -> m_state.player->get_position().y - g_current_scene -> m_state.enemies[ind].get_position().y) < 0.5) {
                    collide_bullet = true;
                    lives -= 1;
                    g_current_scene -> m_state.player->set_position(glm::vec3(2.0f, -3.0f, 0.0f));
                }
            }
            
            if (abs(g_current_scene -> m_state.enemies[1].get_position().x-g_current_scene -> m_state.player->get_position().x)<0.5
                && abs(g_current_scene -> m_state.enemies[1].get_position().y-g_current_scene -> m_state.player->get_position().y)<0.5
                && eat_leg1 == false) {
                eat_all_legs -= 1;
                g_current_scene -> m_state.enemies[1].deactivate();
                eat_leg1 = true;
            }
            
            if (abs(g_current_scene -> m_state.enemies[2].get_position().x-g_current_scene -> m_state.player->get_position().x)<0.5
                && abs(g_current_scene -> m_state.enemies[2].get_position().y-g_current_scene -> m_state.player->get_position().y)<0.5
                && eat_leg2 == false) {
                eat_all_legs -= 1;
                g_current_scene -> m_state.enemies[2].deactivate();
                eat_leg2 = true;
            }
            if (abs(g_current_scene -> m_state.enemies[3].get_position().x-g_current_scene -> m_state.player->get_position().x)<0.5
                && abs(g_current_scene -> m_state.enemies[3].get_position().y-g_current_scene -> m_state.player->get_position().y)<0.5
                && eat_leg3 == false) {
                eat_all_legs -= 1;
                g_current_scene -> m_state.enemies[3].deactivate();
                eat_leg3 = true;
            }
            
            g_time_accumulator = delta_time;
            
            if (g_current_scene -> m_state.enemies[0].get_position().x < -4.0f) {
                g_current_scene -> m_state.enemies[0].set_velocity(glm::vec3(1.0f, 0.0f, 0.0f));
                g_current_scene -> m_state.enemies[0].m_speed = 1.5f;
            }
            else if (g_current_scene -> m_state.enemies[0].get_position().x > 4.0f) {
                g_current_scene -> m_state.enemies[0].set_velocity(glm::vec3(-1.0f, 0.0f, 0.0f));
                g_current_scene -> m_state.enemies[0].m_speed = -1.5f;
            }
            if (eat_all_legs > 0 && g_current_scene -> m_state.player->check_collision(g_current_scene -> m_state.enemies)) {
                lives -= 1;
                g_current_scene -> m_state.player->set_position(glm::vec3(2.0f, -3.0f, 0.0f));
            }
            if (eat_all_legs == 0 && g_current_scene -> m_state.player->check_collision(g_current_scene -> m_state.enemies)) {
                LOG("here");
                switch_to_scene(g_levels[2]);
            }
            if (lives == 0) {endgame = true;}
        }
        
        else if (g_current_scene == g_levels[2]) {
            if (g_current_scene ->m_state.enemies[5].get_position().y > 4.5f) {
                g_current_scene ->m_state.enemies[5].set_position(glm::vec3(-10.0f, -10.0f, 0.0f));
                g_current_scene ->m_state.enemies[5].set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
            }
            
            if (g_current_scene -> m_state.enemies[0].get_position().x < -4.0f) {
                g_current_scene -> m_state.enemies[0].set_velocity(glm::vec3(1.0f, 0.0f, 0.0f));
                g_current_scene -> m_state.enemies[0].m_speed = 1.5f;
            }
            else if (g_current_scene -> m_state.enemies[0].get_position().x > 4.0f) {
                g_current_scene -> m_state.enemies[0].set_velocity(glm::vec3(-1.0f, 0.0f, 0.0f));
                g_current_scene -> m_state.enemies[0].m_speed = -1.5f;}
            
            //bat
            if (g_current_scene -> m_state.enemies[1].get_position().y <-3.5f) {
                g_current_scene -> m_state.enemies[1].set_velocity(glm::vec3(g_current_scene -> m_state.enemies[1].get_velocity().x, 1.5f, 0.0f));
                g_current_scene -> m_state.enemies[1].set_position(glm::vec3(g_current_scene -> m_state.enemies[1].get_position().x, -3.45f, 0.0f));
                if (g_current_scene ->m_state.enemies[1].get_velocity().x < 0) g_current_scene -> m_state.enemies[1].m_speed = -1.0f;
                else g_current_scene -> m_state.enemies[1].m_speed = 1.0f;
            }
            if (g_current_scene -> m_state.enemies[1].get_position().y > 3.5f) {
                g_current_scene -> m_state.enemies[1].set_velocity(glm::vec3(g_current_scene -> m_state.enemies[1].get_velocity().x, -1.5f, 0.0f));
                g_current_scene -> m_state.enemies[1].set_position(glm::vec3(g_current_scene -> m_state.enemies[1].get_position().x, 3.45f, 0.0f));
                if (g_current_scene ->m_state.enemies[1].get_velocity().x < 0) g_current_scene -> m_state.enemies[1].m_speed = -1.0f;
                else g_current_scene -> m_state.enemies[1].m_speed = 1.0f;
            }
            if (g_current_scene -> m_state.enemies[1].get_position().x > 4.6f) {
                g_current_scene -> m_state.enemies[1].set_velocity(glm::vec3(-1.0f, g_current_scene -> m_state.enemies[1].get_velocity().y, 0.0f));
                g_current_scene -> m_state.enemies[1].set_position(glm::vec3(4.55f, g_current_scene -> m_state.enemies[1].get_position().y, 0.0f));
                g_current_scene -> m_state.enemies[1].m_speed = -1.0f;
                
            }
            if (g_current_scene -> m_state.enemies[1].get_position().x < -4.6f) {
                g_current_scene -> m_state.enemies[1].set_movement(glm::vec3(1.0f, g_current_scene -> m_state.enemies[1].get_velocity().y, 0.0f));
                g_current_scene -> m_state.enemies[1].set_position(glm::vec3(-4.55f, g_current_scene -> m_state.enemies[1].get_position().y, 0.0f));
                g_current_scene -> m_state.enemies[1].m_speed = 1.0f;
            }
            
            //3 bullets
            for (size_t ind = 2; ind < 5; ind++) {
                if (g_current_scene -> m_state.enemies[ind].get_position().y <-3.5f) {
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(g_current_scene -> m_state.enemies[ind].get_velocity().x, 1.5f, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(g_current_scene -> m_state.enemies[ind].get_position().x, -3.45f, 0.0f));
                    if (g_current_scene ->m_state.enemies[ind].get_velocity().x < 0) g_current_scene -> m_state.enemies[ind].m_speed = -1.0f;
                    else g_current_scene -> m_state.enemies[ind].m_speed = 1.0f;
                }
                if (g_current_scene -> m_state.enemies[ind].get_position().y > 3.5f) {
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(g_current_scene -> m_state.enemies[ind].get_velocity().x, -1.5f, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(g_current_scene -> m_state.enemies[ind].get_position().x, 3.45f, 0.0f));
                    if (g_current_scene ->m_state.enemies[ind].get_velocity().x < 0) g_current_scene -> m_state.enemies[ind].m_speed = -1.0f;
                    else g_current_scene -> m_state.enemies[ind].m_speed = 1.0f;
                }
                if (g_current_scene -> m_state.enemies[ind].get_position().x > 4.6f) {
                    g_current_scene -> m_state.enemies[ind].set_velocity(glm::vec3(-1.0f, g_current_scene -> m_state.enemies[ind].get_velocity().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(4.55f, g_current_scene -> m_state.enemies[ind].get_position().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].m_speed = -1.0f;
                    
                }
                if (g_current_scene -> m_state.enemies[ind].get_position().x < -4.6f) {
                    g_current_scene -> m_state.enemies[ind].set_movement(glm::vec3(1.0f, g_current_scene -> m_state.enemies[ind].get_velocity().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].set_position(glm::vec3(-4.55f, g_current_scene -> m_state.enemies[ind].get_position().y, 0.0f));
                    g_current_scene -> m_state.enemies[ind].m_speed = 1.0f;
                }
            }
            
                //absorb bullets!
            for (size_t ind = 0; ind < 5; ++ind) {
                if (ind != 0 && ind != 1 && is_absorb){
                    if (abs(g_current_scene -> m_state.enemies[ind].get_position().x-g_current_scene -> m_state.player->get_position().x < 1.0f)
                        && abs(g_current_scene -> m_state.enemies[ind].get_position().y-g_current_scene -> m_state.player->get_position().y < 1.0f)) {
                        g_current_scene -> m_state.enemies[ind].deactivate();
                        if (ind == 2 && !absorbed_2) {
                            bullet_hold += 1;
                            absorbed_2 = true;
                        }
                        else if (ind == 3 && !absorbed_3) {
                            bullet_hold += 1;
                            absorbed_3 = true;
                        }
                        else if (ind == 4 && !absorbed_4) {
                            bullet_hold += 1;
                            absorbed_4 = true;
                        }
                    }
                }
                else {
                    if ((g_current_scene -> m_state.enemies[ind].m_is_active)&& abs(g_current_scene -> m_state.enemies[ind].get_position().x-g_current_scene -> m_state.player->get_position().x < 0.5f)
                        && abs(g_current_scene -> m_state.enemies[ind].get_position().y-g_current_scene -> m_state.player->get_position().y < 0.5f) && ind != 1) {
                        lives -= 1;
                        break;
                        g_current_scene -> m_state.player->set_position(glm::vec3(-2.0f, -3.0f, 0.0f));}
                    if (g_current_scene -> m_state.enemies[1].check_collision(g_current_scene -> m_state.player)) {
                        g_current_scene -> m_state.player->set_position(glm::vec3(-2.0f, -3.0f, 0.0f));
                        lives -= 1;}
                    }
            }
            
            if (attack && bullet_hold > 0 && g_current_scene ->m_state.enemies[5].get_position().x == -10.0f) {
                g_current_scene ->m_state.enemies[5].set_position(g_current_scene -> m_state.player->get_position());
                g_current_scene ->m_state.enemies[5].set_velocity(glm::vec3(0.0f, 2.5f, 0.0f));
                g_current_scene ->m_state.enemies[5].set_movement(glm::vec3(0.0f, 2.5f, 0.0f));
                bullet_hold -= 1;
            }
            if ((abs(g_current_scene -> m_state.enemies[5].get_position().x-g_current_scene -> m_state.enemies[0].get_position().x) < 1.0f) && (abs(g_current_scene -> m_state.enemies[5].get_position().y-g_current_scene -> m_state.enemies[0].get_position().y) < 1.0f)) {
                g_current_scene -> m_state.enemies[0].deactivate();
            }
            if ((abs(g_current_scene -> m_state.enemies[5].get_position().x-g_current_scene -> m_state.enemies[1].get_position().x) < 0.5f) && (abs(g_current_scene -> m_state.enemies[5].get_position().y-g_current_scene -> m_state.enemies[1].get_position().y) < 0.5f)) {
                g_current_scene -> m_state.enemies[1].deactivate();
            }
            if ((bullet_hold == 0 && (g_current_scene -> m_state.enemies[0].m_is_active || g_current_scene -> m_state.enemies[1].m_is_active) && g_current_scene -> m_state.enemies[5].get_position().x == -10.0f)
                && !(g_current_scene -> m_state.enemies[2].m_is_active) && !(g_current_scene -> m_state.enemies[3].m_is_active) && !(g_current_scene -> m_state.enemies[4].m_is_active))
            {
                endgame = true;}
            is_absorb = false;
            if (!g_current_scene ->m_state.enemies[0].m_is_active && !g_current_scene ->m_state.enemies[1].m_is_active && !endgame){
                switch_to_scene(g_levels[3]);
            }
        }
        
        if (g_current_scene == g_levels[3]){
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-0.0068, 0.0035, 0));
            for (size_t ind = 0; ind < 3; ++ind){
                if (abs(g_current_scene -> m_state.player->get_position().x-g_current_scene -> m_state.enemies[ind].get_position().x) < 0.5f && abs(g_current_scene -> m_state.player->get_position().y-g_current_scene -> m_state.enemies[ind].get_position().y) < 0.5f) {
                    g_current_scene -> m_state.enemies[ind].deactivate();
                }
            }
            if (!g_current_scene -> m_state.enemies[0].m_is_active && !g_current_scene -> m_state.enemies[1].m_is_active && !g_current_scene -> m_state.enemies[2].m_is_active) {endgame = true;}
            check_end += 1;
            if (check_end > 1800) {endgame = true;}
        }
        pressing_left = false;
        pressing_right = false;
        pressing_up = false;
        pressing_down = false;
        attack = false;
    }
}

void render()
{
    // ————— GENERAL ————— //
    g_shader_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint text_id = Utility::load_texture("/Users/rachelchen/Desktop/CS3113_Hw3/font1.png");
    if (g_current_scene == g_levels[1]){
        Utility::draw_text(&g_shader_program, text_id, "Be strong, ", 0.35, 0, glm::vec3(-2.0, 3.2, 0.0));
        Utility::draw_text(&g_shader_program, text_id, "then attack from sides! ", 0.4, 0, glm::vec3(-4.2, 2.7, 0.0));
        Utility::draw_text(&g_shader_program, text_id, "Lives: " + std::to_string(lives), 0.4, 0, glm::vec3(1.5, -1.0, 0.0));
        if (!eat_leg1){
            g_current_scene -> m_state.enemies[1].render(&g_shader_program);}
        if (!eat_leg2) {
            g_current_scene -> m_state.enemies[2].render(&g_shader_program);}
        if (!eat_leg3) {
            g_current_scene -> m_state.enemies[3].render(&g_shader_program);}
        for (size_t ind = 4; ind < 7; ++ind){
            g_current_scene -> m_state.enemies[ind].render(&g_shader_program);}
    }
    
    else if(g_current_scene == g_levels[2]){
        Utility::draw_text(&g_shader_program, text_id, "Absorb bullets and shoot!", 0.3, 0, glm::vec3(-4, 3.0, 0.0));
        Utility::draw_text(&g_shader_program, text_id, "Bullets: " + std::to_string(bullet_hold), 0.4, 0, glm::vec3(1.0, 2.0, 0.0));
        for (size_t ind = 0; ind < 6; ++ind){
            g_current_scene -> m_state.enemies[ind].render(&g_shader_program);}
    }
    else if(g_current_scene == g_levels[3]){
        Utility::draw_text(&g_shader_program, text_id, "find all treasures, quickly", 0.4, 0, glm::vec3(-4.0, 2.5, 0.0));
    }

    
    
    
    if (endgame) {
        if (g_current_scene == g_levels[1]){
            if (collide_danger) {
                if (g_current_scene -> m_state.enemies[0].check_collision(g_current_scene -> m_state.player)){
                    if (eat_all_legs != 0) {
                        Utility::draw_text(&g_shader_program, text_id, "You Lose :(", 0.4, 0, glm::vec3(2.0, 3.0, 0.0));
                    }
                }
            }
        }
        
        else if (g_current_scene == g_levels[2]){
            Utility::draw_text(&g_shader_program, text_id, "You Lose :(", 0.4, 0, glm::vec3(1.6, 0.0, 0.0));
        }
        
        else if (g_current_scene == g_levels[3]){
            if (!g_current_scene -> m_state.enemies[0].m_is_active && !g_current_scene -> m_state.enemies[1].m_is_active && !g_current_scene -> m_state.enemies[2].m_is_active) {
                Utility::draw_text(&g_shader_program, text_id, "Congrats, ", 0.4, 0, glm::vec3(10.0, -7.8, 0.0));
                Utility::draw_text(&g_shader_program, text_id, "You Win! ", 0.4, 0, glm::vec3(10.0, -8.3, 0.0));}
            else {Utility::draw_text(&g_shader_program, text_id, "Treasure left, ", 0.35, 0, glm::vec3(10.0, -7.8, 0.0));
                Utility::draw_text(&g_shader_program, text_id, "You Lose! ", 0.4, 0, glm::vec3(10.0, -8.3, 0.0));}
        }
    }


    // ————— GENERAL ————— //
    glUseProgram(g_shader_program.get_program_id());
    g_current_scene->render(&g_shader_program);
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    Mix_FreeChunk(g_bouncing_sfx);
    Mix_FreeMusic(g_music);
    
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    SDL_Quit(); }

// ————— DRIVER GAME LOOP ————— /
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        if (g_current_scene->m_state.next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->m_state.next_scene_id]);
        render();
    }

    shutdown();
    return 0;
}
