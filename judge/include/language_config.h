#ifndef LANGUAGE_CONFIG_H
#define LANGUAGE_CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

// 单门语言的编译/运行元数据与资源倍率
struct LanguageInfo {
    std::string id;
    std::string name;
    std::string version;
    std::string compiler;
    std::string compile_command;
    std::string source_extension;
    std::string output_extension;
    std::string execute_command;
    double time_limit_multiplier;
    double memory_limit_multiplier;
};

// 从 languages.json 加载并查询语言配置
class LanguageConfig {
public:
    LanguageConfig();
    
    ~LanguageConfig() = default;
    
    bool load(const std::string& config_path);

    bool loadFromJsonString(const std::string& json_content);
    
    const LanguageInfo* getLanguage(const std::string& language_id) const;
    
    std::vector<std::string> getSupportedLanguages() const;
    
private:
    std::unordered_map<std::string, LanguageInfo> languages_;
};

#endif // LANGUAGE_CONFIG_H
