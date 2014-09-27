
#ifndef H_BACT_BACTEROIDS_GAME_H
#define H_BACT_BACTEROIDS_GAME_H

#include "../application/Game.h"
#include "Bacteroids.h"

namespace bact
{

    class Bacteroids : public rob::Game
    {
    public:
        Bacteroids();

        bool Initialize() override;
        void OnKeyPress(rob::Keyboard::Key key, rob::Keyboard::Scancode scancode, uint32_t mods) override;

    protected:
        void HandleStateChange(int state) override;

    private:
        GameData m_gameData;
    };

} // bact

#endif // H_BACT_BACTEROIDS_GAME_H

