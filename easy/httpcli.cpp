#include "httpcli.h"
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include <curl/curl.h>
}

size_t writedata_s(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t writedata_f(void* contents, size_t size, size_t nmemb, void* userp) {
    int* handle = (int*)userp;
    write(*handle, (char*)contents, size * nmemb);
    return size * nmemb;
}

size_t _save_header(void* ptr, size_t size, size_t nmemb, void* data) {
    return (size_t)(size * nmemb);
}

size_t _filesize_for_url(CURL* curl, const char* url) {
    if (nullptr == curl || nullptr == url) {
        return 0;
    }

    double filesize = 0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1); //只要求header头
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1); //不需求body
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _save_header);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    CURLcode code = curl_easy_perform(curl);
    if (code == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
    }
    curl_easy_reset(curl);
    return (size_t)filesize;
}

size_t easy::utils::httpcli::filesize_for_url(const std::string& url) {
    CURL*  curl        = curl_easy_init();
    size_t remote_size = _filesize_for_url(curl, url.c_str());
    curl_easy_cleanup(curl);
    return remote_size;
}

easy::utils::curl::result easy::utils::httpcli::download(const char* url, const char* local_name, time_t timeout, bool is_continue) {
    off_t seek = 0;
    CURL* curl = curl_easy_init();
    if (NULL == curl) {
        return {.curl_code = CURLE_FAILED_INIT, .response_code = -1};
    }

    int write_handle = ::open(local_name, O_WRONLY | O_CREAT /*| O_TRUNC*/, 0600);
    if (-1 == write_handle) {
        curl_easy_cleanup(curl);
        return {.curl_code = -2, .response_code = -1};
    }

    // 如果开启断点续传，取文件当前大小做本地文件写入偏移量以及远端下载起始位置
    if (is_continue) {
        struct stat st;
        ::memset(&st, 0x0, sizeof(st));
        ::fstat(write_handle, &st);
        seek = st.st_size;
    } else {
        ftruncate(write_handle, 0);
    }
    std::cout << "seek size: " << seek << std::endl;
    ::lseek(write_handle, seek, SEEK_SET);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata_f);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_handle);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //    curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0);
    std::string range(std::to_string(seek).append("-").c_str());
    curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());

    CURLcode res           = curl_easy_perform(curl);
    long     response_code = 200;
    if (CURLE_OK == res) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }
    curl_easy_cleanup(curl);
    ::close(write_handle);
    // return CURLE_OK == res && (200 == response_code || 206 == response_code);
    return {.curl_code = res, .response_code = response_code};
}

easy::utils::curl::result easy::utils::httpcli::request(const char* url, std::string& response) {
    CURL* curl = curl_easy_init();
    if (NULL == curl) {
        return {.curl_code = CURLE_FAILED_INIT, .response_code = -1};
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata_s);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //    curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0);

    CURLcode res           = curl_easy_perform(curl);
    long     response_code = 200;
    if (CURLE_OK == res) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }
    curl_easy_cleanup(curl);

    return {.curl_code = res, .response_code = response_code};
}

easy::utils::curl::result easy::utils::httpcli::request(const char* url, const std::map<std::string, std::string>& args, std::string& response) {
    std::string request_url = url;
    request_url.append("?");
    std::string split_char = "";
    for (const auto& kv : args) {
        request_url.append(split_char);
        request_url.append(kv.first);
        request_url.append("=");
        request_url.append(kv.second);
        split_char = "&";
    }

    return request(request_url.c_str(), response);
}

easy::utils::curl::result easy::utils::httpcli::request(const char* url, const std::vector<std::string>& headers, const std::map<std::string, std::string>& args, std::string& response) {
    std::string request_url = url;
    request_url.append("?");
    std::string split_char = "";
    for (const auto& kv : args) {
        request_url.append(split_char);
        request_url.append(kv.first);
        request_url.append("=");
        request_url.append(kv.second);
        split_char = "&";
    }

    return request(request_url.c_str(), headers, response);
}

easy::utils::curl::result easy::utils::httpcli::request(const char* url, const std::vector<std::string>& headers, std::string& response) {
    CURL* curl = curl_easy_init();
    if (NULL == curl) {
        return {.curl_code = CURLE_FAILED_INIT, .response_code = -1};
    }

    struct curl_slist* chunk = nullptr;
    for (const auto& item : headers) {
        chunk = curl_slist_append(chunk, item.c_str());
    }
    if (nullptr != chunk) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata_s);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //    curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0);

    CURLcode res           = curl_easy_perform(curl);
    long     response_code = 200;
    if (CURLE_OK == res) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

    return {.curl_code = res, .response_code = response_code};
}

easy::utils::curl::result easy::utils::httpcli::get(const char* url, const std::vector<std::string>& headers, const std::map<std::string, std::string>& args, std::string& response) {
    CURL* curl = curl_easy_init();
    if (NULL == curl) {
        return {.curl_code = CURLE_FAILED_INIT, .response_code = -1};
    }

    std::string request_url = url;

    if (!args.empty()) {
        request_url.append("?");
        std::string split_char = "";
        for (const auto& kv : args) {
            request_url.append(split_char);
            request_url.append(kv.first);
            request_url.append("=");
            request_url.append(kv.second);
            split_char = "&";
        }
    }

    struct curl_slist* chunk = nullptr;
    for (const auto& item : headers) {
        chunk = curl_slist_append(chunk, item.c_str());
    }
    if (nullptr != chunk) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    }
    curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata_s);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //    curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0);

    CURLcode res           = curl_easy_perform(curl);
    long     response_code = 200;
    if (CURLE_OK == res) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

    return {.curl_code = res, .response_code = response_code};
}

easy::utils::curl::result easy::utils::httpcli::post(const char* url, const std::vector<std::string>& headers, const std::map<std::string, std::string>& args, std::string& response) {
    CURL* curl = curl_easy_init();
    if (NULL == curl) {
        return {.curl_code = CURLE_FAILED_INIT, .response_code = -1};
    }

    std::string postfields;

    if (!args.empty()) {
        std::string split_char = "";
        for (const auto& kv : args) {
            postfields.append(split_char);
            postfields.append(kv.first);
            postfields.append("=");
            postfields.append(kv.second);
            split_char = "&";
        }
    }

    struct curl_slist* chunk = nullptr;
    for (const auto& item : headers) {
        chunk = curl_slist_append(chunk, item.c_str());
    }
    if (nullptr != chunk) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata_s);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //    curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0);

    CURLcode res           = curl_easy_perform(curl);
    long     response_code = 200;
    if (CURLE_OK == res) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

    return {.curl_code = res, .response_code = response_code};
}
