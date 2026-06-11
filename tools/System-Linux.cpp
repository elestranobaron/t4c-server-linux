#include "System.h"

#include <cstring>
#include <filesystem>
#include <limits.h>
#include <unistd.h>

namespace fs = std::filesystem;

std::string System::GetMachineName() {
	char buf[256];
	if (gethostname(buf, sizeof(buf)) == 0) {
		return buf;
	}
	return "Unnamed Machine";
}

std::string System::GetProgramPath() {
	char self[PATH_MAX];
	const ssize_t n = readlink("/proc/self/exe", self, sizeof(self) - 1);
	if (n <= 0) {
		return "./";
	}
	self[n] = '\0';
	fs::path p(self);
	std::string dir = p.parent_path().string();
	if (!dir.empty() && dir.back() != '/') {
		dir += '/';
	}
	return dir;
}

std::string System::MakePath(const char *path) {
	if (!path) {
		return GetProgramPath();
	}
	try {
		fs::path base = fs::path(GetProgramPath()) / path;
		fs::path abs = fs::absolute(base);
		std::string s = abs.string();
		if (!s.empty() && s.back() != '/') {
			s += '/';
		}
		return s;
	} catch (...) {
		return GetProgramPath();
	}
}
