
#ifndef H_ROB_TEXT_INPUT_H
#define H_ROB_TEXT_INPUT_H

#include "../String.h"

namespace rob
{

    // Works currently only on ASCII characters
    struct TextInput
    {
        static const size_t MAX_LENGTH = 64;
        char m_text[MAX_LENGTH+1];
        size_t m_length;
        size_t m_cursor;

        TextInput()
            : m_length(0)
            , m_cursor(0)
        {
            m_text[MAX_LENGTH] = 0;
        }

        void MoveLeft()
        {
            if (m_cursor > 0)
                m_cursor--;
        }

        void MoveRight()
        {
            if (m_cursor < m_length)
                m_cursor++;
        }

        void MoveWordLeft()
        {
            MoveLeft();
            while (m_cursor > 0)
            {
                const char prev = m_text[m_cursor - 1];
                if (::isspace(prev) || ::ispunct(prev))
                    break;
                m_cursor--;
            }
        }

        void MoveWordRight()
        {
            MoveRight();
            while (m_cursor < m_length)
            {
                const char next = m_text[m_cursor++];
                if (::isspace(next) || ::ispunct(next))
                    break;
            }
        }

        void Insert(const char *str)
        {
            const size_t slen = StringLength(str);
            if (m_cursor < m_length)
            {
                for (size_t i = m_length; i >= m_cursor; i--)
                {
                    if (i + slen < MAX_LENGTH)
                        m_text[i + slen] = m_text[i];
                    if (i == 0) break;
                }
            }
            while (m_cursor < MAX_LENGTH && *str)
                m_text[m_cursor++] = *str++;
            m_length += slen;
        }

        void Delete()
        {
            if (m_cursor < m_length)
            {
                CopyString(&m_text[m_cursor], &m_text[m_cursor + 1], MAX_LENGTH - m_cursor);
                m_length--;
            }
        }

        void DeleteLeft()
        {
            if (m_cursor > 0)
            {
                m_cursor--;
                Delete();
            }
        }
    };

} // rob

#endif // H_ROB_TEXT_INPUT_H

