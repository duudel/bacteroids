
#ifndef H_BACT_BACTEROIDS_GAME_H
#define H_BACT_BACTEROIDS_GAME_H

#include "../application/Game.h"
#include "HighScoreList.h"

namespace bact
{

    class Bacteroids : public rob::Game
    {
    public:
        bool Initialize() override;
        void OnKeyPress(rob::Keyboard::Key key, rob::Keyboard::Scancode scancode, uint32_t mods) override;

    protected:
        void HandleStateChange(int state) override;

        HighScoreList m_highScores;
    };

} // bact

#endif // H_BACT_BACTEROIDS_GAME_H

