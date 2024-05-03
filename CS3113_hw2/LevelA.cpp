#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

//bool eat_all_legs = 3;
unsigned int LEVELA_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

LevelA::~LevelA()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete [] m_state.platforms;
    delete    m_state.map;
    //Mix_FreeChunk(m_state.jump_sfx);
    //Mix_FreeMusic(m_state.bgm);
}

void LevelA::initialise()
{
    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("/Users/rachelchen/Desktop/CS3113_Hw3/tile.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);
    
    const char  SPRITESHEET_FILEPATH[]  = "/Users/rachelchen/Desktop/CS3113_Hw3/ship.png",
                BULLET_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/bullet.png",
                MONSTER_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/monster.png",
                LEG_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/leg.png";
    
    // Code from main.cpp's initialise()
    // Existing
    m_state.player = new Entity();
    m_state.player->set_position(glm::vec3(-2.0f, -3.5f,0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->m_speed = 2.0f;
    m_state.player->m_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    // Walking
    m_state.player->m_walking[m_state.player->LEFT]   = new int[4] { 1, 5, 9,  13 };
    m_state.player->m_walking[m_state.player->RIGHT]  = new int[4] { 3, 7, 11, 15 };
    m_state.player->m_walking[m_state.player->UP]     = new int[4] { 2, 6, 10, 14 };
    m_state.player->m_walking[m_state.player->DOWN]   = new int[4] { 0, 4, 8,  12 };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];  // start George looking right
    m_state.player->m_animation_frames  = 4;
    m_state.player->m_animation_index   = 0;
    m_state.player->m_animation_time    = 0.0f;
    m_state.player->m_animation_cols    = 4;
    m_state.player->m_animation_rows    = 4;
    m_state.player->set_height(0.9f);
    m_state.player->set_width(0.9f);

    // Jumping
    m_state.player->m_jumping_power = 3.0f;

    // ————— PLATFORM ————— //


    GLuint enemy_texture_id = Utility::load_texture(MONSTER_FILEPATH);
    GLuint leg_texture_id = Utility::load_texture(LEG_FILEPATH);
    
    m_state.enemies = new Entity[7];
    m_state.enemies[0].set_entity_type(ENEMY);
    m_state.enemies[0].set_ai_type(GUARD);
    m_state.enemies[0].set_ai_state(IDLE);
    m_state.enemies[0].m_texture_id = enemy_texture_id;
    m_state.enemies[0].set_position(glm::vec3(-1.0f, 2.0f, 0.0f));
    m_state.enemies[0].set_velocity(glm::vec3(1.0f, 0.0f, 0.0f));
    m_state.enemies[0].m_speed = 1.5f;
    
    m_state.enemies[1].set_entity_type(ENEMY);
    m_state.enemies[1].set_ai_type(WALKER);
    m_state.enemies[1].set_ai_state(IDLE);
    m_state.enemies[1].m_texture_id = leg_texture_id;
    m_state.enemies[1].set_position(glm::vec3(-3.0f, 1.8f, 0.0f));
    
    m_state.enemies[2].set_entity_type(ENEMY);
    m_state.enemies[2].set_ai_type(WALKER);
    m_state.enemies[2].set_ai_state(IDLE);
    m_state.enemies[2].m_texture_id = leg_texture_id;
    m_state.enemies[2].set_position(glm::vec3(2.0f, 1.0f, 0.0f));
    
    m_state.enemies[3].set_entity_type(ENEMY);
    m_state.enemies[3].set_ai_type(WALKER);
    m_state.enemies[3].set_ai_state(IDLE);
    m_state.enemies[3].m_texture_id = leg_texture_id;
    m_state.enemies[3].set_position(glm::vec3(-3.0f, 0.6f, 0.0f));
    
    m_state.enemies[1].set_velocity(glm::vec3(1.0f, -1.5f, 0.0f));
    m_state.enemies[1].m_speed = 1.0f;
    m_state.enemies[2].set_velocity(glm::vec3(-1.0f, -1.5f, 0.0f));
    m_state.enemies[2].m_speed = 1.0f;
    m_state.enemies[3].set_velocity(glm::vec3(-1.0f, 1.5f, 0.0f));
    m_state.enemies[3].m_speed = 1.0f;
    
    GLuint bullet_texture_id = Utility::load_texture(BULLET_FILEPATH);
    
    m_state.enemies[4].set_entity_type(ENEMY);
    m_state.enemies[4].set_ai_type(WALKER);
    m_state.enemies[4].set_ai_state(IDLE);
    m_state.enemies[4].m_texture_id = bullet_texture_id;
    m_state.enemies[4].set_position(m_state.enemies[0].get_position());
    
    m_state.enemies[5].set_entity_type(ENEMY);
    m_state.enemies[5].set_ai_type(WALKER);
    m_state.enemies[5].set_ai_state(IDLE);
    m_state.enemies[5].m_texture_id = bullet_texture_id;
    m_state.enemies[5].set_position(glm::vec3(-10.0f, 0.0f, 0.0f));
    
    m_state.enemies[6].set_entity_type(ENEMY);
    m_state.enemies[6].set_ai_type(WALKER);
    m_state.enemies[6].set_ai_state(IDLE);
    m_state.enemies[6].m_texture_id = bullet_texture_id;
    m_state.enemies[6].set_position(glm::vec3(-10.0f, 2.0f, 0.0f));
    
    
    /**
     BGM and SFX
     */
    //Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    //m_state.bgm = Mix_LoadMUS("assets/bgmusic.mp3");
    //m_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelA::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, 7, m_state.map);
    for (size_t ind = 0; ind < 7; ++ind) {
        m_state.enemies[ind].update(delta_time, m_state.player, 0, 0, m_state.map);}
}

void LevelA::render(ShaderProgram *program)
{
    m_state.map->render(program);
    m_state.player->render(program);
    for (size_t ind = 0; ind < 7; ++ind)
    m_state.enemies[ind].render(program);
}
