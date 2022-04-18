/**
 * @author zouxiaoliang
 * @date 2022/03/10
 */

#include <easy/ftp.h>
#include <iostream>

enum ARG_ID {
    HOST = 1,
    USER,
    PASSWD,
    SRC,
    DST,

    MAX
};
int main(int argc, char* argv[]) {
    std::cout << "Hello world!!!" << std::endl;
    if (argc < MAX) {
        std::cout << argv[0] << " host user passwd src dst" << std::endl;
        return 1;
    }

    const char* host   = argv[HOST];
    const char* user   = argv[USER];
    const char* passwd = argv[PASSWD];
    const char* src    = argv[SRC];
    const char* dst    = argv[DST];

    int curl_code;

    easy::utils::ftp::Config config(host, 22, user, passwd, easy::utils::ftp::Config::EN_PASSIVITY, true);

    auto r = easy::utils::ftp::upload(&config, src, dst, curl_code);
    if (!r) {
        std::cout << "upload failed, local: " << src << ", remote: " << dst << ", what: " << curl_easy_strerror(static_cast<CURLcode>(curl_code)) << std::endl;
    }

    return 0;
}