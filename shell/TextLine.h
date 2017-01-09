#pragma once
#include <string>
#include <vector>

// ------------------------------------------------------
// TextLine
// ------------------------------------------------------
class TextLine {

public:
	TextLine() {}
	TextLine(const std::string& str);
	~TextLine() {}
	void set(const std::string& str, const char delimiter = ',');
	const int find_pos(int field_index) const;
	const int get_int(int index) const;
	const char get_char(int index) const;
	const bool get_bool(int index) const;
	int get_string(int index, char* dest) const;
	const int num_tokens() const;
	void print() const;
private:
	int _num_delimiters;
	std::string _content;
	char _delimiter;
};