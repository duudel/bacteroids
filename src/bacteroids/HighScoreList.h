
#ifndef H_BACT_HIGH_SCORE_LIST_H
#define H_BACT_HIGH_SCORE_LIST_H

#include "../Types.h"

namespace bact
{

    class HighScoreList
    {
    public:
        static const size_t MAX_NAME_LENGTH = 16;
        static const size_t MAX_SCORE_COUNT = 10;
    public:
        HighScoreList();

        bool Load();
        bool Save();

        bool IsHighScore(int score) const;

        /// Adds the new high score and returns it's index.
        size_t AddScore(const char * const name, int score);

        size_t GetIndex(int score) const;

    private:
        struct Score
        {
            char m_name[MAX_NAME_LENGTH];
            int m_score;
        };
        Score m_scores[MAX_SCORE_COUNT];
        size_t m_scoreCount;
    };

} // bact

#endif // H_BACT_HIGHSCORE_LIST_H

