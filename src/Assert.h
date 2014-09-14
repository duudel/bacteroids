
#ifndef H_ROB_ASSERT_H
#define H_ROB_ASSERT_H

#ifdef _DEBUG
#define ROB_ASSERT(x) if (!(x)) rob::Assert(#x, __FILE__, __LINE__)
#else
#define ROB_ASSERT(x)
#endif // _DEBUG

namespace rob
{

    void Assert(const char * const x, const char * const file, const int line);

} // rob

#endif // H_ROB_ASSERT_H

