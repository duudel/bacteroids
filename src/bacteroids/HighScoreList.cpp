
#include "HighScoreList.h"

#include "../Log.h"
#include "../String.h"

#include <SDL2/SDL.h>
#include <string>
#include <fstream>

namespace bact
{

    HighScoreList::HighScoreList()
        : m_scoreCount(0)
    { }

    bool HighScoreList::Load()
    {
        char * const appPath = ::SDL_GetPrefPath("donkions", "Bacteroids");
        std::string filename;
        if (appPath)
        {
            filename = appPath;
            ::SDL_free(appPath);
        }
        filename += "highscore.lst";

        std::ifstream in(filename.c_str(), std::ios::binary);
        if (in.is_open()) return false;

        in.read(reinterpret_cast<char*>(&m_scoreCount), sizeof(uint32_t));
        if (m_scoreCount >= MAX_SCORE_COUNT)
        {
            rob::log::Error("Corrupt high score file: ", filename);
            return false;
        }

        for (size_t i = 0; i < m_scoreCount; i++)
        {
            if (!in)
            {
                rob::log::Error("Corrupt high score file, not enough scores: ", filename);
                return false;
            }
            in.read(reinterpret_cast<char*>(&m_scores[i]), sizeof(Score));
            // TODO: make sure the score is valid, i.e score > 0 and name ends with '\0'.
        }
        return true;
    }

    bool HighScoreList::Save()
    {
        char * const appPath = ::SDL_GetPrefPath("donkions", "Bacteroids");
        std::string filename;
        if (appPath)
        {
            filename = appPath;
            ::SDL_free(appPath);
        }
        filename += "highscore.lst";

        std::ofstream out(filename.c_str(), std::ios::binary);
        if (out.is_open()) return false;

        out.write(reinterpret_cast<const char*>(&m_scoreCount), sizeof(uint32_t));
        for (size_t i = 0; i < m_scoreCount; i++)
        {
            out.write(reinterpret_cast<const char*>(&m_scores[i]), sizeof(Score));
        }
        return true;
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
