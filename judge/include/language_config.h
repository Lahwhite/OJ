/**
 * @file language_config.h
 * @brief 语言配置管理
 * @author OJ Team
 * @date 2024-01-01
 */
#ifndef LANGUAGE_CONFIG_H
#define LANGUAGE_CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

/**
 * @struct LanguageInfo
 * @brief 语言信息结构体
 */
struct LanguageInfo {
    std::string id;                    ///< 语言ID
    std::string name;                  ///< 语言名称
    std::string version;               ///< 语言版本
    std::string compiler;              ///< 编译器名称
    std::string compile_command;       ///< 编译命令
    std::string source_extension;      ///< 源文件扩展名
    std::string output_extension;      ///< 输出文件扩展名
    std::string execute_command;       ///< 执行命令
    double time_limit_multiplier;      ///< 时间限制倍数
    double memory_limit_multiplier;    ///< 内存限制倍数
};

/**
 * @class LanguageConfig
 * @brief 语言配置管理类
 */
class LanguageConfig {
public:
    /**
     * @brief 构造函数
     */
    LanguageConfig();
    
    /**
     * @brief 析构函数
     */
    ~LanguageConfig() = default;
    
    /**
     * @brief 加载语言配置
     * @param config_path 配置文件路径
     * @return 是否加载成功
     */
    bool load(const std::string& config_path);

    /**
     * @brief 从 JSON 字符串加载语言配置
     * @param json_content JSON 内容
     * @return 是否加载成功
     */
    bool loadFromJsonString(const std::string& json_content);
    
    /**
     * @brief 获取语言信息
     * @param language_id 语言ID
     * @return 语言信息指针，不存在则返回nullptr
     */
    const LanguageInfo* getLanguage(const std::string& language_id) const;
    
    /**
     * @brief 获取支持的语言列表
     * @return 语言ID列表
     */
    std::vector<std::string> getSupportedLanguages() const;
    
private:
    std::unordered_map<std::string, LanguageInfo> languages_; ///< 语言信息映射
};

#endif // LANGUAGE_CONFIG_H