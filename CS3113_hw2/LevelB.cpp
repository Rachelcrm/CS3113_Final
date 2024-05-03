#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

unsigned int LEVELB_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

LevelB::~LevelB()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelB::initialise()
{
    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("/Users/rachelchen/Desktop/CS3113_Hw3/tile.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 4, 1);
    const char  SPRITESHEET_FILEPATH[]  = "/Users/rachelchen/Desktop/CS3113_Hw3/ship.png",
                BULLET1_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/bullet1.png",
                BULLET2_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/bullet2.png",
                WITCH_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/witch.png",
    BAT_FILEPATH[] = "/Users/rachelchen/Desktop/CS3113_Hw3/bat.png";
    
    
    GLuint witch_texture_id = Utility::load_texture(WITCH_FILEPATH);
    GLuint bat_texture_id = Utility::load_texture(BAT_FILEPATH);
    GLuint bullet_texture_id = Utility::load_texture(BULLET1_FILEPATH);
    GLuint ownbullet_texture_id = Utility::load_texture(BULLET2_FILEPATH);
    GLuint dragon_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
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
    
    //boss
    m_state.enemies = new Entity[6]; //0 is the boss, 1 is the smaller monster, 2-4 are bullets (can be attracted), 5 is own bullet
    m_state.enemies[0].set_entity_type(ENEMY);
    m_state.enemies[0].set_ai_type(GUARD);
    m_state.enemies[0].set_ai_state(IDLE);
    m_state.enemies[0].m_texture_id = witch_texture_id;
    m_state.enemies[0].set_position(glm::vec3(-1.0f, 2.0f, 0.0f));
    m_state.enemies[0].set_velocity(glm::vec3(1.0f, 0.0f, 0.0f));
    m_state.enemies[0].m_speed = 1.5f;
    
    //smaller monster
    m_state.enemies[1].set_entity_type(ENEMY);
    m_state.enemies[1].set_ai_type(WALKER);
    m_state.enemies[1].set_ai_state(IDLE);
    m_state.enemies[1].m_texture_id = bat_texture_id;
    m_state.enemies[1].set_position(glm::vec3(-3.0f, 1.8f, 0.0f));
    m_state.enemies[1].set_velocity(glm::vec3(1.0f, -1.0f, 0.0f));
    
    //bullets
    m_state.enemies[2].set_entity_type(ENEMY);
    m_state.enemies[2].set_ai_type(WALKER);
    m_state.enemies[2].set_ai_state(IDLE);
    m_state.enemies[2].m_texture_id = bullet_texture_id;
    m_state.enemies[2].set_position(glm::vec3(-10.0f, 0.0f, 0.0f));
    m_state.enemies[2].set_velocity(glm::vec3(1.5f, -0.8f, 0.0f));
    
    m_state.enemies[3].set_entity_type(ENEMY);
    m_state.enemies[3].set_ai_type(WALKER);
    m_state.enemies[3].set_ai_state(IDLE);
    m_state.enemies[3].m_texture_id = bullet_texture_id;
    m_state.enemies[3].set_position(glm::vec3(3.0f, -2.0f, 0.0f));
    m_state.enemies[3].set_velocity(glm::vec3(1.8f, -0.3f, 0.0f));
    
    m_state.enemies[4].set_entity_type(ENEMY);
    m_state.enemies[4].set_ai_type(WALKER);
    m_state.enemies[4].set_ai_state(IDLE);
    m_state.enemies[4].m_texture_id = bullet_texture_id;
    m_state.enemies[4].set_position(glm::vec3(-10.0f, 2.0f, 0.0f));
    m_state.enemies[4].set_velocity(glm::vec3(0.8f, -1.3f, 0.0f));
    
    m_state.enemies[5].set_entity_type(ENEMY);
    m_state.enemies[5].set_ai_type(WALKER);
    m_state.enemies[5].set_ai_state(IDLE);
    m_state.enemies[5].m_texture_id = ownbullet_texture_id;
    m_state.enemies[5].set_position(glm::vec3(-10.0f, -10.0f, 0.0f));
    m_state.enemies[5].set_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_state.jump_sfx = Mix_LoadWAV("/Users/rachelchen/Desktop/CS3113_Hw3/attack.wav");
}

void LevelB::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, 5, m_state.map);
    for (size_t ind = 0; ind < 6; ++ind) {
        m_state.enemies[ind].update(delta_time, m_state.player, 0, 0, m_state.map);}
}

void LevelB::render(ShaderProgram *program)
{
    m_state.map->render(program);
    m_state.player->render(program);
    m_state.enemies[0].render(program);
    m_state.enemies[1].render(program);
    m_state.enemies[2].render(program);
    m_state.enemies[3].render(program);
    m_state.enemies[4].render(program);
    m_state.enemies[5].render(program);
}
