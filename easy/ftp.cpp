#include "ftp.h"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <libgen.h>
#include <memory>


/**
 * @brief join_args 字符串拼接函数
 * @param result[out] 拼接完成的字符串
 * @param s[in] 分隔符
 * @param first[in] 第一字符
 * @param args[in] 字符列表
 */
template<typename S>
void join_args(std::string &, S){}
template<typename S, typename T, typename ... Args>
void join_args(std::string &result, S s, T first, Args ... args)
{
    if(!result.empty())
    {
        result += s;
    }
    result += first;
    join_args(result, s, args ...);
}

void split(std::string content, std::vector<std::string> &r, const char *sep)
{
    const char* curr = ::strtok((char *)content.c_str(), sep);

    while (curr) {
        r.push_back(curr);
        curr = ::strtok(NULL, sep);
    }
}

/**
 * @brief format C++11 的字符串格式化函数
 * @param format 格式
 * @param args 参数列表
 */
template<typename ... Args>
std::string format(const std::string& format, Args ... args)
{
    size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[ size ] );
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}

using namespace easy::utils;

size_t write2string(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (nullptr == stream  || nullptr == ptr || 0 == size)
        return 0;

    size_t realsize = size * nmemb;
    std::string *buffer = (std::string*)stream;
    if (nullptr != buffer)
    {
        buffer->append((const char *)ptr, realsize);
    }
    return realsize;
}

size_t write2file(void *contents, size_t size, size_t nmemb, void *userp)
{
    int *handle = (int*) userp;
    write(*handle, (char*)contents, size * nmemb);
    return size * nmemb;
}

size_t read_from_file(void *ptr, size_t size, size_t nmemb, void *userp)
{
    //std::cout << "==> size: " << size << "nmemb: " << nmemb << std::endl;
    ssize_t retcode;
    int *handle = (int *)userp;
    retcode = read(*handle, ptr, (size_t)(size * nmemb));
    return retcode;
}

curlioerr ftp_ioctl(CURL *handle, curliocmd cmd, void *userp)
{
    std::cout << "==> " << std::endl;

    int *fdp = (int *)userp;
    int fd = *fdp;

    (void)handle; /* not used in here */

    switch(cmd) {
    case CURLIOCMD_RESTARTREAD:
        /* mr libcurl kindly asks as to rewind the read data stream to start */
        if(-1 == lseek(fd, 0, SEEK_SET))
            /* couldn't rewind */
            return CURLIOE_FAILRESTART;

        break;

    default: /* ignore unknown commands */
        return CURLIOE_UNKNOWNCMD;
    }

    return CURLIOE_OK; /* success! */
}

ftp::Config::Config() : ip(""), port(0), username(""), password(""), connection_mode(0) {}

ftp::Config::Config(const std::string &ip, uint16_t port, const std::string &username, const std::string &password, int32_t mode)
    :ip(ip), port(port), username(username), password(password), connection_mode(mode)
{
    root_url = format("ftp://%s:%d/", ip.c_str(), port);
}

ftp::Config::~Config()
{

}

/**
 * @brief command 执行ftp命令(自定义请求方式)
 * @param ftp_handle[in] ftp句柄
 * @param command   [in] ftp命令行
 * @param reponse   [out] ftp命令输出结果
 * @param connect_timeout [in] 接超时时间
 * @param command_timeout [in] command执行超时时间
 * @return
 */
int _command(ftp::Config *config, const char *command, std::string &reponse, int connect_timeout, int command_timeout)
{
    if (nullptr == config || nullptr == command)
    {
        return -1;
    }

    auto curl = curl_easy_init();
    if (nullptr == curl) return  -1;

    reponse.clear();
    std::string userkey;
    userkey.append(config->username).append(":").append(config->password);

    curl_easy_setopt(curl, CURLOPT_URL, config->root_url.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, nullptr);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userkey.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write2string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&reponse);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, command_timeout);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);	//目录不存在时，上传文件时，先创建目录
    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    return res;
}

bool ftp::mkdir(ftp::Config *config, const char *dir_name, int &curl_code)
{
    if (nullptr == config || nullptr == dir_name)
    {
        return false;
    }

    std::string command;
    std::string response;

    // 拼接ftp命令，以空格分割
    join_args(command, " ", "MKD", dir_name);
    curl_code = _command(config, (char *)command.c_str(), response, 1, 1);
    return CURLE_OK == curl_code || CURLE_FTP_COULDNT_RETR_FILE == curl_code;
}

bool ftp::rmdir(ftp::Config *config, const char *dir_name)
{
    if (nullptr == config ||nullptr == dir_name)
    {
        return false;
    }

    std::string command;
    std::string response;

    // 拼接ftp命令，以空格分割
    join_args(command, " ", "RMD", dir_name);
    int res = _command(config, (char *)command.c_str(), response, 1, 1);
    // CURLE_FTP_COULDNT_RETR_FILE 目录操作时，错误码为完成的传输尺寸为零字节
    return CURLE_OK == res || CURLE_FTP_COULDNT_RETR_FILE == res;
}

bool ftp::rmfile(ftp::Config *config, const char *file_name)
{
    if (nullptr == config ||nullptr == file_name)
    {
        return false;
    }

    std::string command;
    std::string response;

    // 拼接ftp命令，以空格分割
    join_args(command, " ", "DELE", file_name);
    int res = _command(config, (char *)command.c_str(), response, 1, 1);
    return CURLE_OK == res || CURLE_FTP_COULDNT_RETR_FILE == res;
}

std::vector<std::string> ftp::ls(ftp::Config *config, const char *path)
{
    std::vector<std::string> files;
    std::string command;
    std::string response;

    if (nullptr == config ||nullptr == path)
    {
        return files;
    }

    // 拼接ftp命令，以空格分割
    join_args(command, " ", "NLST", path);
    if (CURLE_OK == _command(config, (char *)command.c_str(), response, 1, 1))
    {
        split(response, files, "\r\n");
    }
    return files;
}

bool ftp::download(ftp::Config *config, const char *src, const char *dst, int &curl_code)
{
    if (nullptr == config || nullptr == src || nullptr == dst)
    {
        curl_code = -1;
        return false;
    }

    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        curl_code = -1;
        return false;
    }

    int write_handle = ::open(dst, O_WRONLY | O_CREAT, 0600);
    if (-1 == write_handle)
    {
        curl_easy_cleanup(curl);
        curl_code = -2;
        return false;
    }

    std::string path;
    join_args(path, "/", config->root_url.c_str(), src);
    std::string userkey;
    userkey.append(config->username).append(":").append(config->password);

    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, userkey.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write2file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_handle);

    curl_code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    ::close(write_handle);
    return CURLE_OK == curl_code;
}

bool ftp::upload(ftp::Config *config, const char *src, const char *dst, int &curl_code)
{
    CURLcode res;
    curl_code = -1;
    struct stat file_info;

    auto curl = curl_easy_init();
    if (NULL == curl)
    {
        curl_code = -1;
        return false;
    }

    int read_handle = ::open(src, O_RDONLY);
    if (-1 == read_handle)
    {
        curl_easy_cleanup(curl);
        curl_code = -2;
        return false;
    }

    std::string path = config->root_url + dst;
    std::string userkey;
    userkey.append(config->username).append(":").append(config->password);

    fstat(read_handle, &file_info);
    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, userkey.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_from_file);
    curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&read_handle);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, (curl_off_t)file_info.st_size);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

    curl_code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    ::close(read_handle);
    return CURLE_OK == curl_code;
}

bool ftp::cwd(ftp::Config *config, const char *path, int &curl_code)
{
    if (nullptr == config || nullptr == path)
    {
        return false;
    }

    std::string command;

    // 拼接ftp命令，以空格分割
    join_args(command, " ", "CWD", path);
    std::string response;
    curl_code = _command(config, (char *)command.c_str(), response, 1, 1);
    return CURLE_OK == curl_code;
}

bool ftp::rename(ftp::Config *config, const char *oldpath, const char *newpath, int &curl_code)
{
    if (nullptr == config || nullptr == oldpath || nullptr == newpath)
    {
        return false;
    }

    auto curl = curl_easy_init();
    // 拼接ftp命令，以空分割
    std::string rnfp, rnto;
    join_args(rnfp, " ", "RNFR", oldpath);
    join_args(rnto, " ", "RNTO", newpath);
    std::string userkey;
    userkey.append(config->username).append(":").append(config->password);

    struct curl_slist *slist = nullptr;
    slist = curl_slist_append(slist, rnfp.c_str());
    slist = curl_slist_append(slist, rnto.c_str());
    std::string buffer;

    curl_easy_setopt(curl, CURLOPT_URL, config->root_url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, userkey.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, slist);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, nullptr);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write2string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);	//目录不存在时，上传文件时，先创建目录

    curl_code = curl_easy_perform(curl);
    if (nullptr != slist)
    {
        curl_slist_free_all(slist);
    }
    curl_easy_cleanup(curl);

    return CURLE_OK == curl_code || CURLE_FTP_COULDNT_RETR_FILE == curl_code;
}

std::string ftp::pwd(ftp::Config *config, int &curl_code)
{
    if (nullptr == config)
    {
        curl_code = CURLE_FAILED_INIT;
        return "";
    }

    std::string response;

    // 拼接ftp命令，以空格分割
    curl_code = _command(config, "PWD", response, 1, 1);
    return response;
}

bool ftp::upload_as(ftp::Config *config, const char *local_path, const char *upload_file_as, const char *rename_file_to, int &curl_code)
{

    struct stat file_info;

    if (nullptr == config
        || nullptr == local_path
        || nullptr == upload_file_as
        || nullptr == rename_file_to)
    {
        curl_code = -1;
        return false;
    }

    auto curl = curl_easy_init();
    if (nullptr == curl)
    {
        curl_code = -1;
        return false;
    }

    int read_handle = ::open(local_path, O_RDONLY);
    if (read_handle < 0)
    {
        curl_code = -2;
        return false;
    }

//    url.append(config->root_url).append(upload_file_as);
    std::string url = format("ftp://%s:%d/%s", config->ip.c_str(), config->port, upload_file_as);

    fstat(read_handle, &file_info);

    std::string rnfp, rnto;
    join_args(rnfp, " ", "RNFR", ::basename(const_cast<char *>(upload_file_as))/*boost::filesystem::basename(upload_file_as).c_str()*/);
    join_args(rnto, " ", "RNTO", rename_file_to);

    struct curl_slist *slist = nullptr;
    slist = curl_slist_append(slist, rnfp.c_str());
    slist = curl_slist_append(slist, rnto.c_str());
    std::string buffer;


    std::string userkey;
    userkey.append(config->username).append(":").append(config->password);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_from_file);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, userkey.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, slist);
    curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&read_handle);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

    curl_code = curl_easy_perform(curl);

    if (nullptr != slist)
    {
        curl_slist_free_all(slist);
    }
    curl_easy_cleanup(curl);

    ::close(read_handle);
    return CURLE_OK == curl_code;
}
