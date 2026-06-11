#include "stdafx.h"
#include "RegKeyHandler.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <map>
#include <mutex>
#include <filesystem>
#include <string>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifndef _WIN32

namespace {

std::map<std::string, std::string> g_iniValues;
bool g_iniLoadedOk = false;
std::recursive_mutex g_iniMutex;

void TrimInPlace(std::string &s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

std::string NormalizeIniKey(std::string s) {
    TrimInPlace(s);
    std::string out;
    out.reserve(s.size());
    bool prevSlash = false;
    for (unsigned char uc : s) {
        char c = static_cast<char>(std::tolower(uc));
        if (c == '/') {
            c = '\\';
        }
        if (c == '\\') {
            if (!prevSlash) {
                out += '\\';
                prevSlash = true;
            }
        } else {
            prevSlash = false;
            out += c;
        }
    }
    return out;
}

/** Parse INI. Caller must hold g_iniMutex. Remplace la map d'un coup (swap) pour les lecteurs concurrents. */
bool ParseIniFile(const std::filesystem::path &iniPath) {
    std::map<std::string, std::string> fresh;
    std::ifstream in(iniPath);
    if (!in) {
        g_iniLoadedOk = false;
        return false;
    }

    std::string currentSectionNorm;
    std::string line;

    while (std::getline(in, line)) {
        TrimInPlace(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (line.size() >= 2 && line.front() == '[') {
            std::size_t close = line.find(']');
            if (close != std::string::npos) {
                std::string sec = line.substr(1, close - 1);
                currentSectionNorm = NormalizeIniKey(sec);
            }
            continue;
        }

        std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string rawKey = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        TrimInPlace(rawKey);
        TrimInPlace(val);

        std::string fullKey;
        if (currentSectionNorm.empty()) {
            fullKey = NormalizeIniKey(rawKey);
        } else {
            std::string nk = NormalizeIniKey(rawKey);
            fullKey = currentSectionNorm + "\\" + nk;
            fullKey = NormalizeIniKey(fullKey);
        }

        if (!fullKey.empty()) {
            fresh[fullKey] = val;
        }
    }

    g_iniValues.swap(fresh);
    g_iniLoadedOk = true;
    return true;
}

std::filesystem::path IniFilePath() {
    TCHAR buf[4096];
    DWORD n = GetModuleFileName(nullptr, buf, static_cast<DWORD>(sizeof(buf) / sizeof(TCHAR)));
    if (n == 0) {
        return std::filesystem::path("T4CServer.ini");
    }
    std::filesystem::path exe(static_cast<const char *>(buf));
    return exe.parent_path() / "T4CServer.ini";
}

void EnsureIniLoadedUnlocked() {
    if (!g_iniLoadedOk || g_iniValues.empty()) {
        ParseIniFile(IniFilePath());
    }
}

std::string LookupKey(const std::string &openedSubKeyNorm, LPCTSTR item) {
    std::string key = openedSubKeyNorm;
    if (item && item[0]) {
        if (!key.empty()) {
            key += "\\";
        }
        key += item;
    }
    return NormalizeIniKey(key);
}

const std::string *FindIniValueUnlocked(const std::string &openedSubKeyNorm, LPCTSTR item) {
    std::string lk = LookupKey(openedSubKeyNorm, item);
    auto it = g_iniValues.find(lk);
    if (it != g_iniValues.end()) {
        return &it->second;
    }
    return nullptr;
}

void StoreIniValueUnlocked(const std::string &openedSubKeyNorm, LPCTSTR item, const std::string &value) {
    std::string lk = LookupKey(openedSubKeyNorm, item);
    if (!lk.empty()) {
        g_iniValues[lk] = value;
    }
}

} // namespace

void RegKeyHandler::EnsureIniLoaded() {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    EnsureIniLoadedUnlocked();
}

bool RegKeyHandler::ReloadIniFromDisk() {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    return ParseIniFile(IniFilePath());
}

//////////////////////////////////////////////////////////////////////
RegKeyHandler::RegKeyHandler() {
    returnstr[0] = '\0';
}

RegKeyHandler::~RegKeyHandler() {}

BOOL RegKeyHandler::Create(HKEY /*main_key*/, LPCTSTR sub_key) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    EnsureIniLoadedUnlocked();
    if (!g_iniLoadedOk) {
        return FALSE;
    }
    m_iniSubKey = sub_key ? sub_key : "";
    m_iniSubKey = NormalizeIniKey(m_iniSubKey);
    return TRUE;
}

BOOL RegKeyHandler::Open(HKEY /*main_key*/, LPCTSTR sub_key) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    EnsureIniLoadedUnlocked();
    if (!g_iniLoadedOk) {
        return FALSE;
    }
    m_iniSubKey = sub_key ? sub_key : "";
    m_iniSubKey = NormalizeIniKey(m_iniSubKey);
    return TRUE;
}

void RegKeyHandler::WriteProfileString(LPCTSTR item, LPCTSTR value) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    StoreIniValueUnlocked(m_iniSubKey, item, value ? value : "");
}

void RegKeyHandler::WriteProfileInt(LPCTSTR item, DWORD value) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    StoreIniValueUnlocked(m_iniSubKey, item, std::to_string(value));
}

LPCTSTR RegKeyHandler::GetProfileString(LPCTSTR item, LPCTSTR default_arg) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    const std::string *found = FindIniValueUnlocked(m_iniSubKey, item);
    if (found != nullptr) {
        std::strncpy(returnstr, found->c_str(), sizeof(returnstr) - 1);
        returnstr[sizeof(returnstr) - 1] = '\0';
        return returnstr;
    }
    std::strncpy(returnstr, default_arg ? default_arg : "", sizeof(returnstr) - 1);
    returnstr[sizeof(returnstr) - 1] = '\0';
    return returnstr;
}

DWORD RegKeyHandler::GetProfileInt(LPCTSTR item, DWORD default_arg) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    const std::string *found = FindIniValueUnlocked(m_iniSubKey, item);
    if (found == nullptr || found->empty()) {
        return default_arg;
    }
    char *end = nullptr;
    unsigned long v = std::strtoul(found->c_str(), &end, 0);
    if (end == found->c_str()) {
        return default_arg;
    }
    return static_cast<DWORD>(v);
}

void RegKeyHandler::Close(void) {
    m_iniSubKey.clear();
}

BOOL RegKeyHandler::DeleteValue(LPCTSTR lpszItem) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    std::string lk = LookupKey(m_iniSubKey, lpszItem);
    return g_iniValues.erase(lk) != 0 ? TRUE : FALSE;
}

BOOL RegKeyHandler::DeleteKey(LPCTSTR subkey) {
    std::lock_guard<std::recursive_mutex> lock(g_iniMutex);
    std::string base = m_iniSubKey;
    if (subkey && subkey[0]) {
        if (!base.empty()) {
            base += "\\";
        }
        base += subkey;
    }
    base = NormalizeIniKey(base);
    if (base.empty()) {
        return FALSE;
    }
    if (base.back() != '\\') {
        base += "\\";
    }
    std::size_t erased = 0;
    for (auto it = g_iniValues.begin(); it != g_iniValues.end();) {
        const std::string &k = it->first;
        if (k.size() >= base.size() && std::equal(base.begin(), base.end(), k.begin())) {
            it = g_iniValues.erase(it);
            ++erased;
        } else {
            ++it;
        }
    }
    return erased != 0 ? TRUE : FALSE;
}

#else /* _WIN32 */

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RegKeyHandler::RegKeyHandler() {
    keyhandle = NULL;
}

RegKeyHandler::~RegKeyHandler() {
    if (keyhandle != NULL)
        RegCloseKey(keyhandle);
}

////////////////////////////////////////////////////////////////////////////
BOOL RegKeyHandler::Create(HKEY main_key, LPCTSTR sub_key) {
    DWORD exists;
    DWORD err;
    err = RegCreateKeyEx(main_key, sub_key, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                         &keyhandle, &exists);
    mainkey = main_key;
    subkey = sub_key;

    if (err != ERROR_SUCCESS)
        return FALSE;
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL RegKeyHandler::Open(HKEY main_key, LPCTSTR sub_key) {
    DWORD err;

    if (keyhandle != NULL) {
        RegCloseKey(keyhandle);
    }

    err = RegOpenKeyEx(main_key, sub_key, 0, KEY_ALL_ACCESS, &keyhandle);

#ifdef _DEBUG
    if (err != ERROR_SUCCESS) {
        TRACE("\r\nError 0x%x(%u) opening registry key %s.", err, err, sub_key);
    }
#endif

    mainkey = main_key;
    subkey = sub_key;

    if (err != ERROR_SUCCESS)
        return FALSE;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void RegKeyHandler::WriteProfileString(LPCTSTR item, LPCTSTR value) {
    RegSetValueEx(keyhandle, item, 0, REG_SZ, (CONST BYTE *)value, lstrlen(value));
};

////////////////////////////////////////////////////////////////////////////
void RegKeyHandler::WriteProfileInt(LPCTSTR item, DWORD value) {
    RegSetValueEx(keyhandle, item, 0, REG_DWORD, (unsigned char *)&value, sizeof(DWORD));
}

////////////////////////////////////////////////////////////////////////////
LPCTSTR RegKeyHandler::GetProfileString(LPCTSTR item, LPCTSTR default_arg) {
    DWORD count = 1020;
    DWORD type;

    returnstr[0] = 0;
    if (RegQueryValueEx(keyhandle, item, 0, &type, (LPBYTE)returnstr, &count) != ERROR_SUCCESS) {
        lstrcpy(returnstr, default_arg);
        return returnstr;
    }
    return returnstr;
}
////////////////////////////////////////////////////////////////////////////////
DWORD RegKeyHandler::GetProfileInt(LPCTSTR item, DWORD default_arg) {
    DWORD count;
    DWORD type;
    unsigned char *value;

    if (RegQueryValueEx(keyhandle, item, NULL, &type, NULL, &count) != ERROR_SUCCESS)
        return default_arg;

    value = new unsigned char[count];

    if (RegQueryValueEx(keyhandle, item, NULL, &type, value, &count) != ERROR_SUCCESS) {
        delete[] value;
        return default_arg;
    }

    if (type != REG_DWORD) {
        delete[] value;
        return default_arg;
    }

    DWORD return_var = 0;

    for (signed int i = count - 1; i >= 0; i--) {
        return_var *= 256;
        return_var += value[i];
    }
    delete[] value;
    return return_var;
}
//////////////////////////////////////////////////////////////////////////////////////////
void RegKeyHandler::Close(void) {
    if (keyhandle != NULL)
        RegCloseKey(keyhandle);
    keyhandle = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL RegKeyHandler::DeleteValue(LPCTSTR lpszItem) {
    if (RegDeleteValue(keyhandle, lpszItem) == ERROR_SUCCESS) {
        return TRUE;
    }

    return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////
BOOL RegKeyHandler::DeleteKey(LPCTSTR subkey) {
    if (RegDeleteKey(keyhandle, subkey) == ERROR_SUCCESS) {
        return TRUE;
    }

    return FALSE;
}

#endif /* _WIN32 */
