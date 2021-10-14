#ifndef AUTOTHREADSAFE_H
#define AUTOTHREADSAFE_H

#include <mutex>

typedef std::recursive_mutex LibCurlThradMutex;
typedef std::unique_lock<LibCurlThradMutex> LibCurlThradLock;

namespace easy { namespace utils { namespace curl {
class AutoThreadsafe
{
public:
    AutoThreadsafe();

    static void locking_function(int mode, int n, const char *file, int line);

    static unsigned long id_function(void);

    bool setup(void);

    bool cleanup(void);

private:
    static LibCurlThradMutex m_mutex;
    static LibCurlThradMutex *mutex_buf;
};

}}}

#endif // AUTOTHREADSAFE_H
