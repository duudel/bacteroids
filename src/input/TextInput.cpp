
#include "TextInput.h"
#include "../String.h"
#include <cctype>

namespace rob
{

    TextInput::TextInput()
        : m_length(0)
        , m_cursor(0)
    {
        m_text[MAX_LENGTH] = 0;
    }

    const char* TextInput::GetText() const
    { return m_text; }

    size_t TextInput::GetLength() const
    { return m_length; }

    size_t TextInput::GetCursor() const
    { return m_cursor; }

    bool TextInput::MoveLeft()
    {
        if (m_cursor > 0)
        {
            const char *s = m_text + m_cursor;
            SkipUtf8Left(s, m_text);
            m_cursor = s - m_text;
            return true;
        }
        return false;
    }

    bool TextInput::MoveRight()
    {
        if (m_cursor < m_length)
        {
            const char *s = m_text + m_cursor;
            SkipUtf8Right(s, m_text + MAX_LENGTH);
            m_cursor = s - m_text;
            return true;
        }
        return false;
    }

    void TextInput::MoveWordLeft()
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

    void TextInput::MoveWordRight()
    {
        MoveRight();
        while (m_cursor < m_length)
        {
            const char next = m_text[m_cursor++];
            if (::isspace(next) || ::ispunct(next))
                break;
        }
    }

    void TextInput::MoveHome()
    { m_cursor = 0; }

    void TextInput::MoveEnd()
    { m_cursor = m_length; }

    void TextInput::Insert(const char *str)
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

    void TextInput::Delete()
    {
        if (m_cursor < m_length)
        {
            const char *s = m_text + m_cursor;
            SkipUtf8Right(s, m_text + MAX_LENGTH);
            CopyString(&m_text[m_cursor], s, MAX_LENGTH - m_cursor);
            m_length -= (s - (m_text + m_cursor));
//            CopyString(&m_text[m_cursor], &m_text[m_cursor + 1], MAX_LENGTH - m_cursor);
//            m_length--;
        }
    }

    void TextInput::DeleteLeft()
    {
        if (m_cursor > 0)
        {
//            m_cursor--;
            if (MoveLeft())
                Delete();
        }
    }

} // rob