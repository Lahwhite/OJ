#include "language_config.h"
#include <fstream>
#include <iostream>

namespace {

// 从 JSON 的 languages 节点填充语言元数据表
bool fillLanguagesFromJson(const nlohmann::json& config, std::unordered_map<std::string, LanguageInfo>& languages) {
    if (!config.contains("languages")) {
        std::cerr << "Invalid language config: missing 'languages' field" << std::endl;
        return false;
    }

    languages.clear();
    const auto& json_languages = config["languages"];
    // 遍历 languages 对象，每个 key 为语言 ID
    for (const auto& [lang_id, lang_info] : json_languages.items()) {
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

        languages[lang_id] = info;
    }

    std::cout << "Loaded " << languages.size() << " languages" << std::endl;
    return true;
}
}  // namespace

LanguageConfig::LanguageConfig() {
}

// 从磁盘 JSON 文件加载语言配置
bool LanguageConfig::load(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open language config file: " << config_path << std::endl;
        return false;
    }
    
    nlohmann::json config;
    try {
        config_file >> config;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse language config: " << e.what() << std::endl;
        return false;
    }
    
    return fillLanguagesFromJson(config, languages_);
}

// 从内置 JSON 字符串加载（配置文件缺失时的回退方案）
bool LanguageConfig::loadFromJsonString(const std::string& json_content) {
    nlohmann::json config;
    try {
        config = nlohmann::json::parse(json_content);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse language config from string: " << e.what() << std::endl;
        return false;
    }

    return fillLanguagesFromJson(config, languages_);
}

// 按语言 ID 查询配置，未找到返回 nullptr
const LanguageInfo* LanguageConfig::getLanguage(const std::string& language_id) const {
    auto it = languages_.find(language_id);
    if (it == languages_.end()) {
        return nullptr;
    }
    return &(it->second);
}

// 返回已加载的全部语言 ID 列表
std::vector<std::string> LanguageConfig::getSupportedLanguages() const {
    std::vector<std::string> language_ids;
    for (const auto& [id, _] : languages_) {
        language_ids.push_back(id);
    }
    return language_ids;
}
