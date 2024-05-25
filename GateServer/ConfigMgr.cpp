#include "ConfigMgr.h"

ConfigMgr::ConfigMgr()
{
	boost::filesystem::path current_path = boost::filesystem::current_path();   // ��ǰ����ִ�е�·��
	boost::filesystem::path config_path = current_path / "config.ini";          // ƴ�� config.ini ��·��
	std::cout << "config path is " << config_path << std::endl;

	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);                   // ����������ʽ�� ini �ļ�

    // ����INI�ļ��е�����section  
    for (const auto& section_pair : pt) {
        const std::string& section_name = section_pair.first;     // GateServer��VarifyServer
        const boost::property_tree::ptree& section_tree = section_pair.second;  // Ҳ��һ�����ṹ Port = 8080��Port = 50051

        // ����ÿ��section�����������е�key-value��  
        std::map<std::string, std::string> section_config;
        for (const auto& key_value_pair : section_tree) {
            const std::string& key = key_value_pair.first;  // Port
            const std::string& value = key_value_pair.second.get_value<std::string>();  // 8080
            section_config[key] = value;       // ������
        }
        SectionInfo sectionInfo;
        sectionInfo._section_datas = section_config; // ��ÿһ�� section ����ļ�ֵ�Թ�������
        // ��section��key-value�Ա��浽config_map��  
        _config_map[section_name] = sectionInfo;   // �����е� section ��������
    }

    // ������е�section��key-value��  
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
