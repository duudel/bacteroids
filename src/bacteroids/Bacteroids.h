
#ifndef H_BACT_BACTEROIDS_H
#define H_BACT_BACTEROIDS_H

#include "HighScoreList.h"

namespace bact
{

    enum
    {
        STATE_NO_CHANGE = 0,
        STATE_MENU,
        STATE_GAME,
        STATE_HIGH_SCORE
    };

    struct GameData
    {
        int             m_score;
        HighScoreList   m_highScores;
    };

} // bact

#endif // H_BACT_BACTEROIDS_H

