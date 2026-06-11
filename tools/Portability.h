#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef _WIN32
#include <windows.h>
#include <afxstr.h>
typedef CString String;
#else
#include "StandardTypes.h"
#include <string>
#include <algorithm> // for std::remove
#include <mutex>
#include <cstdio>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <strings.h>
#include <ctime>
#include <cstdlib>

#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef _snprintf
#define _snprintf std::snprintf
#endif
#define _atoi64(s) (std::strtoll((s), nullptr, 10))
#ifndef __cdecl
#define __cdecl
#endif
#ifndef CDECL
#define CDECL
#endif

/* MSVC provides itoa; POSIX code uses snprintf. */
inline char *itoa(int value, char *buffer, int radix) {
    if (radix != 10) {
        buffer[0] = '\0';
        return buffer;
    }
    std::snprintf(buffer, 16, "%d", value);
    return buffer;
}
inline char *ltoa(long value, char *buffer, int radix) {
    if (radix != 10) {
        buffer[0] = '\0';
        return buffer;
    }
    std::snprintf(buffer, 32, "%ld", value);
    return buffer;
}
/* MSVC _strtime: writes time into caller buffer (≥26 chars). */
inline char *_strtime(char *buffer) {
    if (!buffer) {
        return buffer;
    }
    std::time_t t = std::time(nullptr);
    std::tm *loc = std::localtime(&t);
    if (!loc) {
        buffer[0] = '\0';
        return buffer;
    }
    std::strftime(buffer, 32, "%H:%M:%S", loc);
    return buffer;
}
class String : public std::string {
public:
    using std::string::string;
    using std::string::operator=;
    String(const std::string &s) : std::string(s) {}
    String(std::string &&s) noexcept : std::string(std::move(s)) {}
    void Remove(char ch) {
        erase(std::remove(begin(), end(), ch), end());
    }
    bool IsEmpty() const {
        return empty();
    }
    void MakeUpper() {
        std::transform(begin(), end(), begin(), ::toupper);
    }
    operator const char*() const {
        return c_str();
    }
    char GetAt(int nIndex) const {
        if (nIndex < 0 || static_cast<size_t>(nIndex) >= size()) {
            return '\0';
        }
        return (*this)[static_cast<size_t>(nIndex)];
    }
    void Append(const char *s) {
        if (s) {
            append(s);
        }
    }
    void Append(char ch) {
        push_back(ch);
    }
    void SetAt(int nIndex, char ch) {
        if (nIndex >= 0 && static_cast<size_t>(nIndex) < size()) {
            (*this)[static_cast<size_t>(nIndex)] = ch;
        }
    }
    void Insert(int nIndex, const char *s) {
        if (!s) {
            return;
        }
        const size_t pos = nIndex < 0 ? 0u : static_cast<size_t>(nIndex);
        insert(pos, s);
    }
    // MFC CString compatibility (substrings / keyword search in NPC scripts)
    int Find(const char *s) const {
        if (s == nullptr) {
            return -1;
        }
        const size_t pos = find(s);
        return pos == npos ? -1 : static_cast<int>(pos);
    }
    int Find(const String &s) const {
        return Find(s.c_str());
    }
    int Find(const char *s, int start) const {
        if (!s || start < 0) {
            return -1;
        }
        const size_t pos = find(s, static_cast<size_t>(start));
        return pos == npos ? -1 : static_cast<int>(pos);
    }
    int ReverseFind(char ch) const {
        for (int idx = static_cast<int>(size()) - 1; idx >= 0; --idx) {
            if ((*this)[static_cast<size_t>(idx)] == ch) {
                return idx;
            }
        }
        return -1;
    }
    String Left(int n) const {
        if (n <= 0) {
            return String();
        }
        return String(substr(0u, static_cast<size_t>(n)));
    }
    String Right(int n) const {
        if (n <= 0 || empty()) {
            return String();
        }
        const size_t len = size();
        const size_t take = static_cast<size_t>(n) > len ? len : static_cast<size_t>(n);
        return String(substr(len - take));
    }
    int Format(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        char buf[4096];
        const int n = std::vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        if (n > 0 && n < static_cast<int>(sizeof(buf))) {
            assign(buf);
        }
        return n;
    }
    void TrimLeft() {
        size_t i = 0;
        while (i < size() && std::isspace(static_cast<unsigned char>((*this)[static_cast<size_t>(i)]))) {
            ++i;
        }
        if (i > 0) {
            erase(0, i);
        }
    }
    void TrimRight() {
        while (!empty() &&
               std::isspace(static_cast<unsigned char>((*this)[size() - 1u]))) {
            pop_back();
        }
    }
    String SpanIncluding(const char *charSet) const {
        String out;
        if (!charSet) {
            return out;
        }
        for (size_t i = 0; i < size(); ++i) {
            const char c = (*this)[i];
            if (::strchr(charSet, c)) {
                out += c;
            }
        }
        return out;
    }
    size_t GetLength() const {
        return length();
    }
    void Empty() {
        clear();
    }
    char *GetBuffer(int minLen) {
        if (minLen < 0) {
            minLen = 0;
        }
        if (minLen > (int)size()) {
            resize(static_cast<size_t>(minLen));
        }
        return size() ? &(*this)[0] : nullptr;
    }
    void ReleaseBuffer(int newLen = -1) {
        if (newLen >= 0) {
            resize(static_cast<size_t>(newLen));
        }
    }
    void MakeLower() {
        std::transform(begin(), end(), begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }
    String Mid(size_t first, size_t count) const {
        if (first >= size()) {
            return String();
        }
        return String(static_cast<const std::string &>(*this).substr(first, count));
    }
    String Mid(size_t first) const {
        if (first >= size()) {
            return String();
        }
        return String(static_cast<const std::string &>(*this).substr(first));
    }
    int CompareNoCase(const String &o) const {
        return strcasecmp(c_str(), o.c_str());
    }
    int Replace(const char *pszOld, const char *pszNew) {
        if (!pszOld || !pszNew) {
            return 0;
        }
        int n = 0;
        const size_t oldLen = std::strlen(pszOld);
        const size_t newLen = std::strlen(pszNew);
        size_t pos = 0;
        while ((pos = find(pszOld, pos)) != npos) {
            replace(pos, oldLen, pszNew);
            pos += newLen;
            ++n;
        }
        return n;
    }
};

typedef String CString;

inline int AfxMessageBox(const CString &msg) {
    std::fputs(msg.c_str(), stderr);
    std::fputc('\n', stderr);
    fflush(stderr);
    return 0;
}

#ifndef strcpy_s
#define strcpy_s(dst, sz, src) \
    do { \
        if ((dst) && (sz) > 0) { \
            std::strncpy((dst), (src), (size_t)(sz) - 1); \
            (dst)[(size_t)(sz) - 1] = '\0'; \
        } \
    } while (0)
#endif

#include <vector>
class CPtrArray {
public:
    void Add(void *p) { m_items.push_back(p); }
    void *GetAt(int i) const { return (i >= 0 && (size_t)i < m_items.size()) ? m_items[(size_t)i] : nullptr; }
    int GetSize() const { return (int)m_items.size(); }
    void RemoveAll() { m_items.clear(); }
    void RemoveAt(int i, int count = 1) {
        if (i < 0 || count <= 0) {
            return;
        }
        const size_t idx = static_cast<size_t>(i);
        if (idx >= m_items.size()) {
            return;
        }
        const size_t end = idx + static_cast<size_t>(count);
        if (end >= m_items.size()) {
            m_items.erase(m_items.begin() + static_cast<std::ptrdiff_t>(idx), m_items.end());
        } else {
            m_items.erase(m_items.begin() + static_cast<std::ptrdiff_t>(idx),
                          m_items.begin() + static_cast<std::ptrdiff_t>(end));
        }
    }
    void SetAtGrow(int i, void *p) {
        if (i < 0) {
            return;
        }
        const size_t need = static_cast<size_t>(i) + 1u;
        if (need > m_items.size()) {
            m_items.resize(need, nullptr);
        }
        m_items[static_cast<size_t>(i)] = p;
    }
private:
    std::vector<void *> m_items;
};


// Dummy RGB for colors, can be adjusted if needed
#define RGB(r, g, b) ((unsigned int)(((unsigned char)(r) | ((unsigned short)((unsigned char)(g)) << 8)) | (((unsigned int)(unsigned char)(b)) << 16)))

#endif

#include <cstring> // for strcpy, strlen

#endif // PORTABILITY_H
