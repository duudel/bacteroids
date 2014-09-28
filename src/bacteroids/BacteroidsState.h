
#ifndef H_BACT_BACTEROIDS_STATE_H
#define H_BACT_BACTEROIDS_STATE_H

#include "../application/GameState.h"
#include "Bacteroids.h"

#include "SoundPlayer.h"
#include "Uniforms.h"
#include "Input.h"
#include "Player.h"
#include "ObjectArray.h"
#include "FadeEffect.h"

#include "../input/TextInput.h"
#include "../math/Random.h"

namespace bact
{

    using namespace rob;

    class GameObject;
    class Player;
    class Bacter;
    class Projectile;

    class BacteroidsState : public GameState
    {
    public:
        BacteroidsState(GameData &gameData);
        ~BacteroidsState();

        bool Initialize() override;
        void RealtimeUpdate(const Time_t deltaMicroseconds) override;
        void Update(const GameTime &gameTime) override;
        void Render() override;

        void OnResize(int w, int h) override;

        void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override;
        void OnKeyDown(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override;
        void OnKeyUp(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override;

    private:
        void TogglePause();

        void SpawnBacter(float distMod = 1.0f);
        void SplitBacter(Bacter *bacter);

        void BacterCollision(Bacter *me, GameObject *obj, const vec2f &objToMe, float dist);
        void PlayerCollision(Player *me, GameObject *obj, const vec2f &objToMe, float dist);
        void ProjectileCollision(Projectile *me, GameObject *obj, const vec2f &objToMe, float dist);
        void DoCollision(GameObject *obj1, GameObject *obj2, const vec2f &from2To1, float dist);

        void DoCollisions();
        void UpdatePlayer(const GameTime &gameTime);

        void RenderPause();
        void RenderGameOver();

    private:
        GameData &m_gameData;
        Random m_random;

        ShaderProgramHandle m_playerShader;
        ShaderProgramHandle m_bacterShader;
        ShaderProgramHandle m_projectileShader;
        ShaderProgramHandle m_fontShader;
        BacteroidsUniforms m_uniforms;
        SoundPlayer m_soundPlayer;

        Input m_input;

        Player m_player;
        ObjectArray m_objects;
        GameObject **m_quadTree;

        int m_score;

        FadeEffect m_damageFade;
        FadeEffect m_pauseFade;

        View m_playView;

        TextInput m_textInput;
    };

} // bact

#endif // H_BACT_BACTEROIDS_STATE_H

