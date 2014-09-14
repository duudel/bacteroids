
#ifndef H_ROB_WINDOW_H
#define H_ROB_WINDOW_H

namespace rob
{

    class Game;
    struct WindowData;

    class Window
    {
    public:
        Window();
        ~Window();

        bool HandleEvents(Game *game);

        void SwapBuffers();

        void GetSize(int *w, int *h) const;

    private:
        static const unsigned SIZE_OF_WIN_DATA_BUFFER = 6 * sizeof(void*);
        char m_windowDataBuffer[SIZE_OF_WIN_DATA_BUFFER];

        WindowData *m_data;
    };

} // rob

#endif // H_ROB_WINDOW_H

