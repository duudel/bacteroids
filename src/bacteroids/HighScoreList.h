
#ifndef H_BACT_HIGH_SCORE_LIST_H
#define H_BACT_HIGH_SCORE_LIST_H

#include "../Types.h"

namespace bact
{

    class HighScoreList
    {
    public:
        static const size_t_32 MAX_NAME_LENGTH = 16;
        static const size_t_32 MAX_SCORE_COUNT = 10;
        static const size_t_32 INVALID_INDEX = MAX_SCORE_COUNT;
    public:
        HighScoreList();

        bool Load();
        bool Save();

        bool IsHighScore(int score) const;

        /// Adds the new high score and returns it's index.
        size_t_32 AddScore(int score);
        void SetName(size_t_32 index, const char * const name);

        size_t_32 GetIndex(int score) const;

        const char* GetName(size_t_32 index) const;
        int GetScore(size_t_32 index) const;
        size_t_32 GetScoreCount() const;

    private:
        struct Score
        {
            char m_name[MAX_NAME_LENGTH];
            int m_score;
        };
        Score m_scores[MAX_SCORE_COUNT];
        size_t_32 m_scoreCount;
    };

} // bact

#endif // H_BACT_HIGHSCORE_LIST_H

