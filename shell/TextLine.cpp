#include "TextLine.h"

// ------------------------------------------------------
// CSVLine
// ------------------------------------------------------
TextLine::TextLine(const std::string& str) : _delimiter(',') {
	set(str);
}

void TextLine::set(const std::string& str, const char delimiter) {
	_content = str;
	_num_delimiters = 0;
	_delimiter = delimiter;
	for (int i = 0; i < _content.length(); ++i) {
		if (_content[i] == delimiter) {
			++_num_delimiters;
		}
	}
}

const int TextLine::num_tokens() const {
	return _num_delimiters + 1;
}

void TextLine::print() const {
	printf("%s\n", _content.c_str());
}
// ------------------------------------------------------
// find pos in string for field index
// ------------------------------------------------------
const int TextLine::find_pos(int field_index) const {
	if (field_index == 0) {
		return 0;
	}
	int cnt = 0;
	for (int i = 0; i < _content.length(); ++i) {
		if (_content[i] == _delimiter) {
			++cnt;
		}
		if (cnt == field_index) {
			return i + 1;
		}
	}
	return -1;
}

// ------------------------------------------------------
// get int
// ------------------------------------------------------
const int TextLine::get_int(int index) const {
	int idx = find_pos(index);
	if (idx != -1) {
		int v;
		int npos = find_pos(index + 1);
		if (npos != -1) {
			std::string str = _content.substr(idx, npos - idx - 1);
			sscanf(str.c_str(), "%d", &v);
		}
		else {
			std::string str = _content.substr(idx);
			sscanf(str.c_str(), "%d", &v);
		}
		return v;
	}
	return -1;
}

// ------------------------------------------------------
// get string
// ------------------------------------------------------
int TextLine::get_string(int index, char* dest) const {
	int idx = find_pos(index);
	if (idx != -1) {
		int npos = find_pos(index + 1);
		if (npos != -1) {
			std::string str = _content.substr(idx, npos - idx - 1);
			strcpy(dest, str.c_str());
			return str.length();
		}
		else {
			std::string str = _content.substr(idx);
			strcpy(dest, str.c_str());
			return str.length();
		}
	}
	return 0;
}

// ------------------------------------------------------
// get char
// ------------------------------------------------------
const char TextLine::get_char(int index) const {
	int idx = find_pos(index);
	if (idx != -1) {
		return _content[idx];
	}
	return '-';
}

// ------------------------------------------------------
// get bool
// ------------------------------------------------------
const bool TextLine::get_bool(int index) const {
	int idx = find_pos(index);
	if (idx != -1) {
		char c = _content[idx];
		if (c == 'N' || c == 'n') {
			return false;
		}
		return true;
	}
	return false;
}

