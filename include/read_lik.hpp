#pragma once

#include <map>
#include <set>

typedef std::map<std::string, std::set<std::string>> lik_facs;

lik_facs read_lik(std::string lik_file_path);