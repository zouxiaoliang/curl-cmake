#ifndef FTP_H
#define FTP_H

#include <string>
#include <list>
#include <vector>
#include <sys/stat.h>

extern "C" {
#include <curl/curl.h>
}
/**
 * @details 基于curl对ftp客户端接口的封装
 */
namespace easy { namespace utils { namespace ftp {

/**
 * @brief The Channel struct
 */
class Config
{
public:
    enum EMode
    {
        EN_ACTIVE = 0,
        EN_PASSIVITY
    };

public:
    /**
     * @brief Channel 构造及析构函数
     * @param ip IP地址
     * @param port 端口
     * @param name 用户名
     * @param passwd 密码
     * @param mode
     */
    Config();
    Config(const std::string &ip, uint16_t port, const std::string &username, const std::string &password, int32_t mode);
    ~Config();

public:
    /**
     * @brief ip 服务端IP地址
     */
    std::string ip;

    /**
     * @brief port 服务端监听端口ftp默认21，sftp默认22
     */
    uint16_t port;

    /**
     * @brief username 用户名
     */
    std::string username;

    /**
     * @brief password 密码
     */
    std::string password;

    /**
     * @brief root_url 连接跟路径：ftp:host:port
     */
    std::string root_url;

    /**
     * @brief connection_mode 连接模式，主动和被动模式
     */
    int32_t connection_mode;
};

/**
 * @brief mkdir 在ftp远端创建目录
 * @param config
 * @param dir_name 目录名称
 * @return
 */
bool mkdir(Config *config, const char *dir_name, int &curl_code);

/**
 * @brief rmdir 删除远端目录(目录内必须为空)
 * @param config
 * @param dir_name 目录名称
 * @return
 */
bool rmdir(Config *config, const char *dir_name);

/**
 * @brief rmfile 删除远端文件
 * @param config
 * @param file_name 文件名
 * @return
 */
bool rmfile(Config *config, const char *file_name);

/**
 * @brief cwd 切换ftp远端工作目录(暂不支持)
 * @param config
 * @return
 */
bool cwd(Config *config, const char *path, int &curl_code);

/**
 * @brief pwd 获取远端服务器工作路径
 * @param config
 * @param curl_code 错误码
 * @return 当前工作路径
 */
std::string pwd(Config *config, int &curl_code);

/**
 * @brief rn 重命名
 * @param config
 * @param oldpath 原路径
 * @param newpath 新路径
 * @return
 */
bool rename(Config *config, const char *oldpath, const char *newpath, int &curl_code);

/**
 * @brief ls 输出远端ftp服务目录所有文件名，默认根目录
 * @param config
 * @param path 远端路径路径
 * @return
 */
std::vector<std::string> ls(Config *config, const char *path = "/");

/**
 * @brief download
 * @param config
 * @param src
 * @param dst
 * @param curl_code
 * @return
 */
bool download(Config *config, const char *src, const char *dst, int &curl_code);

/**
 * @brief upload
 * @param config
 * @param src
 * @param dst
 * @param curl_code
 * @return
 */
bool upload(Config *config, const char *src, const char *dst, int &curl_code);

/**
 * @brief upload_and_rename
 * @param config
 * @param local_path
 * @param temp_path
 * @param remote_path
 * @param curl_code
 * @return
 */
bool upload_as(Config *config, const char *local_path, const char *temp_path, const char *remote_path, int &curl_code);

}}}
#endif // FTP_H
