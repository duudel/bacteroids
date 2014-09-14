
#ifndef H_ROB_LOG_H
#define H_ROB_LOG_H

namespace rob
{
namespace log
{

    void PrintValue(char value);
    void PrintValue(signed char value);
    void PrintValue(unsigned char value);
    void PrintValue(short value);
    void PrintValue(unsigned short value);
    void PrintValue(int value);
    void PrintValue(unsigned int value);
    void PrintValue(long long value);
    void PrintValue(unsigned long long value);
    void PrintValue(float value);
    void PrintValue(double value);
    void PrintValue(const char *value);
    void PrintValue(char *value);
    void PrintValue(const void *value);
    void PrintValue(void *value);

    template <class T>
    void Print(T&& value)
    {
        PrintValue(value);
    }

    template <class T, class... Args>
    void Print(T&& value, Args&& ...args)
    {
        PrintValue(value);
        Print(args...);
    }

    template <class... Args>
    void Info(Args&& ...args)
    {
        PrintValue("Info: ");
        Print(args...);
        PrintValue('\n');
    }

    template <class... Args>
    void Error(Args&& ...args)
    {
        PrintValue("Error: ");
        Print(args...);
        PrintValue('\n');
    }

} // log
} // rob

#endif // H_ROB_LOG_H

