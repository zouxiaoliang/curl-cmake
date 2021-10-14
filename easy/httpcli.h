#ifndef HTTPCLI_H
#define HTTPCLI_H

#include <string>
#include <map>

/**
 * 基于libcurl进行封装的http处理接口
 */
namespace easy { namespace utils { namespace httpcli {

/**
 * @brief filesize_for_url
 * @param url
 * @return
 */
size_t filesize_for_url(const std::string &url);

/**
 * @brief download 给定url将文件下载到本地的指定文件中，如果文件存在则覆盖文件，当前不创建目录
 * @param url 下载路径
 * @param local_name 本地文件名
 * @return true/false
 */
bool download(const char *url, const char *local_name, int &curl_code, time_t timeout = 5, bool is_continue = false);

/**
 * @brief request 发送http请求
 * @param url 请求url路径
 * @param response 响应内容
 * @param curl_code curl返回编码
 * @return true/false
 */
bool request(const char *url, std::string &response, int &curl_code);

/**
 * @brief request 发送http请求
 * @param url 请求url路径
 * @param args 请求参数
 * @param response 响应内容
 * @param curl_code curl返回编码
 * @return true/false
 */
bool request(const char *url, const std::map<std::string, std::string> &args, std::string &response, int &curl_code);
} } }

#endif // HTTPCLI_H
