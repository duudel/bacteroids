
#include "HighScoreList.h"

#include "../String.h"

namespace bact
{

    HighScoreList::HighScoreList()
        : m_scoreCount(0)
    { }

    bool HighScoreList::Load()
    {

    }

    bool HighScoreList::Save()
    {

    }

    size_t HighScoreList::GetIndex(int score) const
    {
        for (size_t i = 0; i < m_scoreCount; i++)
        {
            if (score > m_scores[i].m_score)
                return i;
        }
        return m_scoreCount;
    }

    bool HighScoreList::IsHighScore(int score) const
    { return GetIndex(score) < MAX_SCORE_COUNT; }

    size_t HighScoreList::AddScore(const char * const name, int score)
    {
        const size_t index = GetIndex(score);

        const size_t last = (m_scoreCount < MAX_SCORE_COUNT)
            ? m_scoreCount : MAX_SCORE_COUNT - 1;

        for (size_t i = index; i < last; i++)
            m_scores[i + 1] = m_scores[i];

        rob::CopyString(m_scores[index].m_name, name);
        m_scores[index].m_score = score;
        return index;
    }

} // bact
