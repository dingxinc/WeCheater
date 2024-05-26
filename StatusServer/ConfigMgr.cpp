#include "ConfigMgr.h"

ConfigMgr::ConfigMgr()
{
	boost::filesystem::path current_path = boost::filesystem::current_path();   // 当前程序执行的路径
	boost::filesystem::path config_path = current_path / "config.ini";          // 拼接 config.ini 的路径
	std::cout << "config path is " << config_path << std::endl;

	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);                   // 按照树的形式读 ini 文件

    // 遍历INI文件中的所有section  
    for (const auto& section_pair : pt) {
        const std::string& section_name = section_pair.first;     // GateServer、VarifyServer
        const boost::property_tree::ptree& section_tree = section_pair.second;  // 也是一个树结构 Port = 8080、Port = 50051

        // 对于每个section，遍历其所有的key-value对  
        std::map<std::string, std::string> section_config;
        for (const auto& key_value_pair : section_tree) {
            const std::string& key = key_value_pair.first;  // Port
            const std::string& value = key_value_pair.second.get_value<std::string>();  // 8080
            section_config[key] = value;       // 存起来
        }
        SectionInfo sectionInfo;
        sectionInfo._section_datas = section_config; // 将每一个 section 下面的键值对管理起来
        // 将section的key-value对保存到config_map中  
        _config_map[section_name] = sectionInfo;   // 将所有的 section 管理起来
    }

    // 输出所有的section和key-value对  
    for (const auto& section_entry : _config_map) {
        const std::string& section_name = section_entry.first;
        SectionInfo section_config = section_entry.second;
        std::cout << "[" << section_name << "]" << std::endl;
        for (const auto& key_value_pair : section_config._section_datas) {
            std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
        }
    }
}

ConfigMgr::ConfigMgr(const ConfigMgr& src)
{
	_config_map = src._config_map;
}

ConfigMgr& ConfigMgr::operator=(const ConfigMgr& src)
{
	if (this != &src) {
		_config_map = src._config_map;
	}
	return *this;
}
