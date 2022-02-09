#ifndef HTTPCLI_H
#define HTTPCLI_H

#include <map>
#include <string>
#include <vector>

#include "result.h"

/**
 * 基于libcurl进行封装的http处理接口
 */
namespace easy { namespace utils { namespace httpcli {

/**
 * @brief The result enum Http status code
 */
enum result {
    CONTINUE                                = 100, // Continue
    SWITCHING_PROTOCOLS                     = 101, // Switching Protocols
    PROCESSING                              = 102, // Processing (WebDAV; RFC 2518)
    EARLY_HINTS                             = 103, // Early Hints (RFC 8297)
    OK                                      = 200, // OK
    CREATED                                 = 201, // Created
    ACCEPTED                                = 202, // Accepted
    NON_AUTHORITATIVE_INFORMATION           = 203, // Non-Authoritative Information (since HTTP/1.1)
    NO_CONTENT                              = 204, // No Content
    RESET_CONTENT                           = 205, // Reset Content
    PARTIAL_CONTENT                         = 206, // Partial Content (RFC 7233)
    MULTI_STATUS                            = 207, // Multi-Status (WebDAV; RFC 4918)
    ALREADY_REPORTED                        = 208, // Already Reported (WebDAV; RFC 5842)
    IM_USED                                 = 226, // IM Used (RFC 3229)
    MULTIPLE_CHOICES                        = 300, // Multiple Choices
    MOVED_PERMANENTLY                       = 301, // Moved Permanently
    FOUND                                   = 302, // Found (Previously "Moved temporarily")
    SEE_OTHER                               = 303, // See Other (since HTTP/1.1)
    NOT_MODIFIED                            = 304, // Not Modified (RFC 7232)
    USE_PROXY                               = 305, // Use Proxy (since HTTP/1.1)
    SWITCH_PROXY                            = 306, // Switch Proxy
    TEMPORARY_REDIRECT                      = 307, // Temporary Redirect (since HTTP/1.1)
    PERMANENT_REDIRECT                      = 308, // Permanent Redirect (RFC 7538)
    ERROR_ON_WIKIMEDIA                      = 404, // error on Wikimedia
    BAD_REQUEST                             = 400, // Bad Request
    UNAUTHORIZED_RFC_7235                   = 401, // Unauthorized (RFC 7235)
    PAYMENT_REQUIRED                        = 402, // Payment Required
    FORBIDDEN                               = 403, // Forbidden
    NOT_FOUND                               = 404, // Not Found
    METHOD_NOT_ALLOWED                      = 405, // Method Not Allowed
    NOT_ACCEPTABLE                          = 406, // Not Acceptable
    PROXY_AUTHENTICATION_REQUIRED           = 407, // Proxy Authentication Required (RFC 7235)
    REQUEST_TIMEOUT                         = 408, // Request Timeout
    CONFLICT                                = 409, // Conflict
    GONE                                    = 410, // Gone
    LENGTH_REQUIRED                         = 411, // Length Required
    PRECONDITION_FAILED                     = 412, // Precondition Failed (RFC 7232)
    PAYLOAD_TOO_LARGE                       = 413, // Payload Too Large (RFC 7231)
    URI_TOO_LONG                            = 414, // URI Too Long (RFC 7231)
    UNSUPPORTED_MEDIA_TYPE                  = 415, // Unsupported Media Type (RFC 7231)
    RANGE_NOT_SATISFIABLE                   = 416, // Range Not Satisfiable (RFC 7233)
    EXPECTATION_FAILED                      = 417, // Expectation Failed
    IM_A_TEAPOT                             = 418, // I'm a teapot (RFC 2324, RFC 7168)
    MISDIRECTED_REQUEST                     = 421, // Misdirected Request (RFC 7540)
    UNPROCESSABLE_ENTITY                    = 422, // Unprocessable Entity (WebDAV; RFC 4918)
    LOCKED                                  = 423, // Locked (WebDAV; RFC 4918)
    FAILED_DEPENDENCY                       = 424, // Failed Dependency (WebDAV; RFC 4918)
    TOO_EARLY                               = 425, // Too Early (RFC 8470)
    UPGRADE_REQUIRED                        = 426, // Upgrade Required
    PRECONDITION_REQUIRED                   = 428, // Precondition Required (RFC 6585)
    TOO_MANY_REQUESTS                       = 429, // Too Many Requests (RFC 6585)
    REQUEST_HEADER_FIELDS_TOO_LARGE         = 431, // Request Header Fields Too Large (RFC 6585)
    UNAVAILABLE_FOR_LEGAL_REASONS           = 451, // Unavailable For Legal Reasons (RFC 7725)
    INTERNAL_SERVER_ERROR                   = 500, // Internal Server Error
    NOT_IMPLEMENTED                         = 501, // Not Implemented
    BAD_GATEWAY                             = 502, // Bad Gateway
    SERVICE_UNAVAILABLE                     = 503, // Service Unavailable
    GATEWAY_TIMEOUT                         = 504, // Gateway Timeout
    HTTP_VERSION_NOT_SUPPORTED              = 505, // HTTP Version Not Supported
    VARIANT_ALSO_NEGOTIATES                 = 506, // Variant Also Negotiates (RFC 2295)
    INSUFFICIENT_STORAGE                    = 507, // Insufficient Storage (WebDAV; RFC 4918)
    LOOP_DETECTED                           = 508, // Loop Detected (WebDAV; RFC 5842)
    NOT_EXTENDED                            = 510, // Not Extended (RFC 2774)
    NETWORK_AUTHENTICATION_REQUIRED         = 511, // Network Authentication Required (RFC 6585)
    PAGE_EXPIRED                            = 419, // Page Expired (Laravel Framework)
    METHOD_FAILURE                          = 420, // Method Failure (Spring Framework)
    ENHANCE_YOUR_CALM                       = 420, // Enhance Your Calm (Twitter)
    REQUEST_HEADER_FIELDS_TOO_LARGE_Shopify = 430, // Request Header Fields Too Large (Shopify)
    BLOCKED_BY_WINDOWS_PARENTAL_CONTROLS    = 450, // Blocked by Windows Parental Controls (Microsoft)
    INVALID_TOKEN                           = 498, // Invalid Token (Esri)
    TOKEN_REQUIRED                          = 499, // Token Required (Esri)
    BANDWIDTH_LIMIT_EXCEEDED                = 509, // Bandwidth Limit Exceeded (Apache Web Server/cPanel)
    SITE_IS_OVERLOADED                      = 529, // Site is overloaded
    SITE_IS_FROZEN                          = 530, // Site is frozen
    NETWORK_READ_TIMEOUT_ERROR              = 598, // (Informal convention) Network read timeout error
    NETWORK_CONNECT_TIMEOUT_ERROR           = 599, // Network Connect Timeout Error
    LOGIN_TIME_OUT                          = 440, // Login Time-out
    RETRY_WITH                              = 449, // Retry With
    REDIRECT                                = 451, // Redirect
    NO_RESPONSE                             = 444, // No Response
    REQUEST_HEADER_TOO_LARGE                = 494, // Request header too large
    SSL_CERTIFICATE_ERROR                   = 495, // SSL Certificate Error
    SSL_CERTIFICATE_REQUIRED                = 496, // SSL Certificate Required
    HTTP_REQUEST_SENT_TO_HTTPS_PORT         = 497, // HTTP Request Sent to HTTPS Port
    CLIENT_CLOSED_REQUEST                   = 499, // Client Closed Request
    WEB_SERVER_RETURNED_AN_UNKNOWN_ERROR    = 520, // Web Server Returned an Unknown Error
    WEB_SERVER_IS_DOWN                      = 521, // Web Server Is Down
    CONNECTION_TIMED_OUT                    = 522, // Connection Timed Out
    ORIGIN_IS_UNREACHABLE                   = 523, // Origin Is Unreachable
    A_TIMEOUT_OCCURRED                      = 524, // A Timeout Occurred
    SSL_HANDSHAKE_FAILED                    = 525, // SSL Handshake Failed
    INVALID_SSL_CERTIFICATE                 = 526, // Invalid SSL Certificate
    RAILGUN_ERROR                           = 527, // Railgun Error
    UNAUTHORIZED                            = 561, // Unauthorized
    RESPONSE_IS_STALE                       = 110, // Response is Stale
    REVALIDATION_FAILED                     = 111, // Revalidation Failed
    DISCONNECTED_OPERATION                  = 112, // Disconnected Operation
    HEURISTIC_EXPIRATION                    = 113, // Heuristic Expiration
    MISCELLANEOUS_WARNING                   = 199, // Miscellaneous Warning
    TRANSFORMATION_APPLIED                  = 214, // Transformation Applied
    MISCELLANEOUS_PERSISTENT_WARNING        = 299, // Miscellaneous Persistent Warning
};

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
 * @return
 */
curl::result download(const char* url, const char* local_name, time_t timeout = 5, bool is_continue = false);

/**
 * @brief request 发送http请求
 * @param url 请求url路径
 * @param response 响应内容
 * @return true/false
 */
curl::result request(const char* url, std::string& response);
/**
 * @brief get
 * @param url
 * @param headers
 * @param args
 * @param response
 * @return
 */
curl::result get(const char* url, const std::vector<std::string>& headers, const std::map<std::string, std::string>& args, std::string& response);

/**
 * @brief post
 * @param url
 * @param headers
 * @param args
 * @param response
 * @return
 */
curl::result post(const char* url, const std::vector<std::string>& headers, const std::map<std::string, std::string>& args, std::string& response);

/**
 * @brief request 发送http请求
 * @param url 请求url路径
 * @param args 请求参数
 * @param response 响应内容
 * @param curl_code curl返回编码
 * @return true/false
 */
curl::result request(const char* url, const std::vector<std::string>& headers, std::string& response);
curl::result request(const char* url, const std::map<std::string, std::string>& args, std::string& response);
curl::result request(const char* url, const std::vector<std::string>& headers, const std::map<std::string, std::string>& args, std::string& response);
/**
 * @brief is_ok 判断http请求是否成功
 * @param r 结果
 * @return true 成功; false 失败
 */
inline bool is_ok(const curl::result& r) {
    return CURLE_OK == r.curl_code && result::OK == r.response_code;
}

/**
 * @brief is_download_ok 判断http下载文件是否成功
 * @param r 结果
 * @return true 成功; false 失败
 */
inline bool is_download_ok(const curl::result& r) {
    return CURLE_OK == r.curl_code && (result::OK == r.response_code || result::PARTIAL_CONTENT == r.response_code);
}
} } }

#endif // HTTPCLI_H
