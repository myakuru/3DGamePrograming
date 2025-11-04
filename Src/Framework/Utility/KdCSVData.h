#pragma once

struct KdCSVData
{
	KdCSVData() {}
	KdCSVData(const std::string_view filename) { Load(filename); }

	bool Load(const std::string_view filename);

	const std::vector<std::vector<std::string>>& GetLData() const { return m_dataLines; }

	const std::vector<std::string>& GetLine(size_t index) const;

	size_t GetLineSize() const { return m_dataLines.size(); }

private:

	void CommaSeparatedValue(std::string_view src, std::vector<std::string>& result);

	std::vector<std::vector<std::string>> m_dataLines;

	std::string m_filePass;

	static const std::vector<std::string> c_nullDataList;
};
