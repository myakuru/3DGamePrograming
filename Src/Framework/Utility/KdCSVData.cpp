#include "KdCSVData.h"

const std::vector<std::string> KdCSVData::c_nullDataList;

bool KdCSVData::Load(const std::string_view filename)
{
	if (filename.empty()) { return false; }

	m_filePass = filename.data();

	std::ifstream ifs(m_filePass);

	if (!ifs)
	{ 
		assert(0 && "CSVDataが見つかりません");

		return false;
	}

	// 行ごとに分けてデータ格納
	while (1)
	{
		std::string rawLineData;
		if (!getline(ifs, rawLineData)) { break; }

		// [,]で分けて単語ごとにデータ格納
		std::vector<std::string> lineData;
		CommaSeparatedValue(rawLineData, lineData);

		m_dataLines.push_back(lineData);
	}

	return true;
}

// 行データを取得
const std::vector<std::string>& KdCSVData::GetLine(size_t index) const
{
	if (index >= m_dataLines.size()) { return c_nullDataList; }

	return m_dataLines[index];
}

// [,]で分けて単語リスト作成
void KdCSVData::CommaSeparatedValue(std::string_view line, std::vector<std::string>& result)
{
	std::istringstream stream(line.data());
	std::string element;

	while (getline(stream, element, ','))
	{
		result.push_back(element);
	}
}
