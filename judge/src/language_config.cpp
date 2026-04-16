#include "language_config.h"
#include <fstream>
#include "oj/log.h"

LanguageConfig::LanguageConfig() {
}

/**
 * @brief 加载语言配置
 * @param config_path 配置文件路径
 * @return 是否加载成功
 */
bool LanguageConfig::load(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        OJ_LOG_ERROR("Failed to open language config file: " + config_path);
        return false;
    }
    
    nlohmann::json config;
    try {
        config_file >> config;
    } catch (const std::exception& e) {
        OJ_LOG_ERROR("Failed to parse language config: " + std::string(e.what()));
        return false;
    }
    
    if (!config.contains("languages")) {
        OJ_LOG_ERROR("Invalid language config: missing 'languages' field");
        return false;
    }
    
    const auto& languages = config["languages"];
    for (const auto& [lang_id, lang_info] : languages.items()) {
        LanguageInfo info;
        info.id = lang_id;
        info.name = lang_info.value("name", "");
        info.version = lang_info.value("version", "");
        info.compiler = lang_info.value("compiler", "");
        info.compile_command = lang_info.value("compile_command", "");
        info.source_extension = lang_info.value("source_extension", "");
        info.output_extension = lang_info.value("output_extension", "");
        info.execute_command = lang_info.value("execute_command", "");
        info.time_limit_multiplier = lang_info.value("time_limit_multiplier", 1.0);
        info.memory_limit_multiplier = lang_info.value("memory_limit_multiplier", 1.0);
        
        languages_[lang_id] = info;
    }
    
    OJ_LOG_INFO("Loaded " + std::to_string(languages_.size()) + " languages");
    return true;
}

/**
 * @brief 获取语言信息
 * @param language_id 语言ID
 * @return 语言信息指针，不存在则返回nullptr
 */
const LanguageInfo* LanguageConfig::getLanguage(const std::string& language_id) const {
    auto it = languages_.find(language_id);
    if (it == languages_.end()) {
        return nullptr;
    }
    return &(it->second);
}

/**
 * @brief 获取支持的语言列表
 * @return 语言ID列表
 */
std::vector<std::string> LanguageConfig::getSupportedLanguages() const {
    std::vector<std::string> language_ids;
    for (const auto& [id, _] : languages_) {
        language_ids.push_back(id);
    }
    return language_ids;
}