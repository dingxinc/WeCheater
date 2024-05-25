#pragma once
#include "global.h"

struct SectionInfo {  // Section ��ʾһ��Ƭ�Σ�ÿһ�� Section ���涼�м�ֵ�ԣ����磬GateServer��һ�� Section, port �� key, 8080 �� value
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
	// ������ﵽ session[key] = value ��Ч������Ҫ���� [] �����
	std::string operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";   // û���ҵ������ؿ�
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
	std::map<std::string, SectionInfo> _config_map;  // key �� section �����֣�value ������ section ������
};