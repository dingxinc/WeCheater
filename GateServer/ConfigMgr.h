#pragma once
#include "global.h"

struct SectionInfo {  // Section 表示一个片段，每一个 Section 下面都有键值对，例如，GateServer是一个 Section, port 是 key, 8080 是 value
	SectionInfo() {}
	~SectionInfo() { _section_datas.clear(); }

	SectionInfo(const SectionInfo& src) { _section_datas = src._section_datas; }
	SectionInfo& operator=(const SectionInfo& src) {
		if (this != &src) {
			_section_datas = src._section_datas;
		}
		return *this;
	}

	std::map<std::string, std::string> _section_datas;
	// 我们想达到 session[key] = value 的效果，需要重载 [] 运算符
	std::string operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";   // 没有找到，返回空
		}
		return _section_datas[key];
	}
};


class ConfigMgr
{
public:
	ConfigMgr();
	~ConfigMgr() { _config_map.clear(); }
	ConfigMgr(const ConfigMgr& src);
	ConfigMgr& operator=(const ConfigMgr& src);

	SectionInfo operator[](const std::string& section) {
		if (_config_map.find(section) == _config_map.end()) {
			return SectionInfo();
		}
		return _config_map[section];
	}

private:
	std::map<std::string, SectionInfo> _config_map;  // key 是 section 的名字，value 是整个 section 的数据
};