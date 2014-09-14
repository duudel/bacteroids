
#ifndef H_BACT_BACTEROIDS_GAME_H
#define H_BACT_BACTEROIDS_GAME_H

#include "../application/Game.h"

namespace bact
{

    class Bacteroids : public rob::Game
    {
    public:
        bool Initialize() override;
    };

} // bact

#endif // H_BACT_BACTEROIDS_GAME_H

