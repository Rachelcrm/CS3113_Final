#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 16
#define LEVEL_HEIGHT 10

unsigned int LEVELC_DATA[] =
{
    2, 0, 2, 2, 0, 2, 0, 0, 0, 2, 2, 0, 0, 2, 0, 2,
    2, 2, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 0, 2, 0, 2,
    0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 2,
    0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 0, 2, 0, 2, 2,
    2, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 0, 0, 0, 2, 2,
    2, 2, 2, 2, 0, 0, 2, 0, 2, 0, 0, 2, 2, 0, 2, 2,
    0, 2, 2, 0, 0, 2, 0, 2, 0, 0, 2, 0, 0, 2, 0, 2,
    0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
    0, 0, 0, 2, 2, 2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

LevelC::~LevelC()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    //Mix_FreeChunk(m_state.jump_sfx);
    //Mix_FreeMusic(m_state.bgm);
}

void LevelC::initialise()
{
    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("/Users/rachelchen/Desktop/CS3113_Hw3/tile.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 4, 1);
    
    const char  SPRITESHEET_FILEPATH[]  = "/Users/rachelchen/Desktop/CS3113_Hw3/ship.png",
                TREASURE_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/treasure.png";
    
    
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
    
    GLuint enemy_texture_id = Utility::load_texture(TREASURE_FILEPATH);
    m_state.enemies = new Entity[3];
    for (size_t ind = 0; ind < 3; ++ind) {
        m_state.enemies[ind].set_entity_type(ENEMY);
        m_state.enemies[ind].set_ai_type(WALKER);
        m_state.enemies[ind].set_ai_state(IDLE);
        m_state.enemies[ind].m_texture_id = enemy_texture_id;}
    m_state.enemies[0].set_position(glm::vec3(1.01f, -4.05f, 0.0f));
    m_state.enemies[1].set_position(glm::vec3(3.95f, -5.1f, 0.0f));
    m_state.enemies[2].set_position(glm::vec3(12.7f, -6.5f, 0.0f));
    
    /**
     BGM and SFX
     */
    //Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    //m_state.bgm = Mix_LoadMUS("assets/bgmusic.mp3");
    //m_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelC::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, 0, m_state.map);
    for (size_t ind = 0; ind < 3; ++ind) {
        m_state.enemies[ind].update(delta_time, m_state.player, 0, 0, m_state.map);}
}

void LevelC::render(ShaderProgram *program)
{
    m_state.map->render(program);
    m_state.player->render(program);
    for (size_t ind = 0; ind < 3; ++ind)
    m_state.enemies[ind].render(program);
}
