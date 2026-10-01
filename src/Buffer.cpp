#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "Buffer.hpp"

namespace sjtu {

Buffer::Buffer(const std::filesystem::path& path) : path_(path) {
    if (path.empty()){
        lines_.emplace_back();
	return;
    }

    std::ifstream file(path);
    if (!file){
        if (std::filesystem::exists(path)) {
            throw std::runtime_error("can't open file: " + path.string());
        }
        lines_.emplace_back();
        return;
    }

    std::string line;
    while (std::getline(file, line)){
	if (!line.empty() && line.back() == '\r'){
            line.pop_back();
	}
	lines_.push_back(line);
    }
    if (lines_.empty()){
        lines_.emplace_back();
    }
	
    //从path指向的文件构造Buffer,你需要打开文件并且把文件内容填充进Buffer,并正确初始化一些状态.
    //注意path可能为空的边界情况
}

Buffer::Buffer(std::vector<std::string> lines, std::filesystem::path path) {
    lines_ = std::move(lines);
    path_ = std::move(path);

    if (lines_.empty()){
	lines_.emplace_back();
    }
}

std::size_t Buffer::GetLineCount() const {
    //返回文件行数
    return lines_.size();
}

const std::string& Buffer::GetLineAt(std::size_t row) const {
    //返回第row行的内容
    return lines_.at(row);
}


std::string Buffer::GetDisplayName() const {
    //返回文件名,若是新文件,返回"[No Name]"
    return path_.empty() ? "[No Name]" : path_.filename().string();
}

bool Buffer::IsModified() const {
    //返回文件和上次保存比起来是否被修改过
    return modified_;    
}

void Buffer::InsertCharacter(std::size_t row, std::size_t column, char value) {
    //在第row行第col列插入一个value, 注意越界检查
    auto& line = lines_.at(row);
    if (column > line.size()){
	throw std::out_of_range("column is outside the line");
    }
    
    line.insert(column, 1, value);
    modified_ = true;
}

void Buffer::EraseCharacter(std::size_t row, std::size_t column) {
   //在第row行第col列删除一个value
    auto& line = lines_.at(row);
    if (column >= line.size()){
	throw std::out_of_range("column is outside the line");
    }

    line.erase(column, 1);
    modified_ = true;
}

void Buffer::SplitLine(std::size_t row, std::size_t column) {
    //在第row行第col列分割,即在此处敲了回车键
    auto& line = lines_.at(row);
    if (column > line.size()){
	throw std::out_of_range("column is outside the line");
    }

    std::string remainder = line.substr(column);
    line.erase(column);
    lines_.insert(lines_.begin() + row + 1, remainder);
    modified_ = true;
}

void Buffer::JoinLine(std::size_t row) {
   //把第row + 1行合并进第row行
    if (row + 1 >= lines_.size()){
	throw std::out_of_range("there is no following line to join");
    }

    lines_[row] += lines_[row + 1];
    lines_.erase(lines_.begin() + row + 1);
    modified_ = true;
}

void Buffer::Save() {
   //把文件内容保存, 直接调用WriteTo方法
    if (path_.empty()) {
	throw std::runtime_error("no file name");
    }

    WriteTo(path_);
    modified_ = false;
}

void Buffer::SaveAs(const std::filesystem::path& path) {
    if (path.empty()) {
	throw std::runtime_error("no file name");
    }

    WriteTo(path);
    path_ = path;
    modified_ = false;
}


void Buffer::WriteTo(const std::filesystem::path& path) const {
   //实际将缓冲区中的内容写入path指向的文件中
    std::ofstream file(path);
    if (!file){
	throw std::runtime_error("can't write file: " + path.string());
    }

    for (const auto& line : lines_){
	file << line << '\n';
    }

    if (!file){
	throw std::runtime_error("can't write file:" + path.string());
    }

}

} // namespace sjtu
