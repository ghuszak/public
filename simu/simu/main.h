#if !defined(__MAIN_H__)
#define __MAIN_H__

#define __FUNC             __FUNCTION__
#define __FILE             __FILE__
#define __LINE             __LINE__

// Change X_DEBUG to _DEBUG to enable full assertion in debug build
#if defined(_DEBUG)
#define ASSERT(e, pid)     _assert(e, pid, __FUNC, __FILE, __LINE)
#define _DO_ASSERT
#else // if defined(_DEBUG)
#define ASSERT(e, pid)
#under _DO_ASSERT
#endif // else if defined(_DEBUG)

#define MSG_OK(pid, msg)   print(stdout, pid, 0u, msg)
#define MSG_ERR(pid, msg)  print(stderr, pid, 0u, msg)

bool _assert(uint expression, uint pid, const char *p_func, const char *p_file, int line);
void print(FILE *f, uint pid, uint pid_filter, const char *p_str, ...);

#endif // if !defined(__MAIN_H__)
