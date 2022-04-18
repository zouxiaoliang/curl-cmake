#include "AutoThreadsafe.h"
#include <openssl/err.h>

LibCurlThradMutex* easy::utils::curl::AutoThreadsafe::mutex_buf = NULL;
LibCurlThradMutex  easy::utils::curl::AutoThreadsafe::m_mutex;

easy::utils::curl::AutoThreadsafe::AutoThreadsafe() {}

void easy::utils::curl::AutoThreadsafe::locking_function(int mode, int n, const char* file, int line) {
    if (mode & CRYPTO_LOCK)
        mutex_buf[n].lock();
    else
        mutex_buf[n].unlock();
}

unsigned long easy::utils::curl::AutoThreadsafe::id_function() {
    return (unsigned long)pthread_self();
}

bool easy::utils::curl::AutoThreadsafe::setup() {
    LibCurlThradLock guard(m_mutex);
    if (nullptr == mutex_buf) {
        mutex_buf = new (std::nothrow) LibCurlThradMutex[CRYPTO_num_locks()];
        if (nullptr == mutex_buf)
            return false;
    }

    CRYPTO_set_id_callback(id_function);
    CRYPTO_set_locking_callback(locking_function);

    return true;
}

bool easy::utils::curl::AutoThreadsafe::cleanup() {
    LibCurlThradLock guard(m_mutex);
    if (nullptr == mutex_buf) {
        return false;
    }

    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);

    delete[] mutex_buf;
    mutex_buf = NULL;
    return true;
}
