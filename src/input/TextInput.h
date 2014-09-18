
#ifndef H_ROB_TEXT_INPUT_H
#define H_ROB_TEXT_INPUT_H

#include "../Types.h"

namespace rob
{

    // Works currently only on ASCII characters
    class TextInput
    {
        static const size_t MAX_LENGTH = 64;
        char m_text[MAX_LENGTH+1];
        size_t m_length;
        size_t m_cursor;
    public:
        TextInput();

        const char* GetText() const;
        size_t GetLength() const;
        size_t GetCursor() const;

        void MoveLeft();
        void MoveRight();

        void MoveWordLeft();
        void MoveWordRight();

        void MoveHome();
        void MoveEnd();

        void Insert(const char *str);

        void Delete();
        void DeleteLeft();

        void DeleteWord();
        void DeleteWordLeft();
    };

} // rob

#endif // H_ROB_TEXT_INPUT_H

