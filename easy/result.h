/**
 * @author zouxiaoliang
 * @date 2022/1/24
 */
#ifndef CURLRET_H
#define CURLRET_H

#include <curl/curl.h>

namespace easy {
namespace utils {
namespace curl {

struct result {
    int curl_code;
    long response_code;

    inline bool is_curl_ok() {
        return CURLE_OK == curl_code;
    }
};

} // namespace curl
} // namespace utils
} // namespace easy
#endif // CURLRET_H
