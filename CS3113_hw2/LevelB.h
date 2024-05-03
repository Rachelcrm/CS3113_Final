#include "Scene.h"

class LevelB : public Scene {
public:
    ~LevelB();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
