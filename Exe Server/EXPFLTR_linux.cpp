/* Linux stubs for exception filter (Windows SEH / dbghelp). See EXPFLTR.CPP on Win32. */
#include "stdafx.h"
#include "EXPFLTR.H"
#include <cstring>

std::mutex CExpFltr::m_crit;
TCHAR CExpFltr::m_report_file[_MAX_PATH];
TCHAR CExpFltr::m_app_title[MAX_SZ_APP_TITLE];
WORD CExpFltr::m_num_modules = 0;
DWORD CExpFltr::m_ptr_module_addr[MAX_MODULES];
BYTE CExpFltr::m_GP_count = 0;
BYTE CExpFltr::m_max_GP = 10;
VoidFunction CExpFltr::m_stop = nullptr;
CExpFltr CExpFltr::m_Instance;

CExpFltr::CExpFltr(void) = default;
CExpFltr::~CExpFltr(void) = default;

void CExpFltr::InitFilter(void *stop, BYTE max_GP) {
	(void)stop;
	(void)max_GP;
}

LONG CExpFltr::Filter(LPEXCEPTION_POINTERS lp) {
	(void)lp;
	return 0;
}

void CExpFltr::log_exception(CONTEXT *ctxtRec, LPTSTR errorMsg, void *excpAddr) {
	(void)ctxtRec;
	(void)errorMsg;
	(void)excpAddr;
}

void CExpFltr::log_mem_map(HANDLE hFile, DWORD **stackStart, DWORD **stackEnd, DWORD *nStacks) {
	(void)hFile;
	(void)stackStart;
	(void)stackEnd;
	(void)nStacks;
}

BOOL CExpFltr::get_region_info(void *memStart, RGNINFO *rgnInfo) {
	(void)memStart;
	(void)rgnInfo;
	return FALSE;
}

DWORD CExpFltr::fixVirtualQuery(LPVOID lpvAddr, PMEMORY_BASIC_INFORMATION pmem_bi, DWORD cbLen) {
	(void)lpvAddr;
	(void)pmem_bi;
	(void)cbLen;
	return 0;
}

void CExpFltr::walkMemRegion(RGNINFO *rgnInfo) { (void)rgnInfo; }

void CExpFltr::region_map_line(RGNINFO *rgnInfo, LPTSTR buf, WORD buf_size) {
	(void)rgnInfo;
	(void)buf;
	(void)buf_size;
}

void CExpFltr::add_module(LPTSTR buf, short buf_len, LPTSTR module_name) {
	(void)buf;
	(void)buf_len;
	(void)module_name;
}

void CExpFltr::log_stack(HANDLE hFile, DWORD *esp, DWORD *esb, DWORD **stackStart, DWORD **stackEnd,
	DWORD nStacks, void *excpAddr, LPCONTEXT lpContext) {
	(void)hFile;
	(void)esp;
	(void)esb;
	(void)stackStart;
	(void)stackEnd;
	(void)nStacks;
	(void)excpAddr;
	(void)lpContext;
}

void CExpFltr::get_func_names(DWORD pFunc, TCHAR funcNames[][64], DWORD funcAddr[]) {
	(void)pFunc;
	(void)funcNames;
	(void)funcAddr;
}

BOOL CExpFltr::loadExportLists(DWORD module_addr, DWORD **functions, WORD **ordinals, LPTSTR **names,
	DWORD *nFunc, DWORD *nNames) {
	(void)module_addr;
	(void)functions;
	(void)ordinals;
	(void)names;
	(void)nFunc;
	(void)nNames;
	return FALSE;
}

LPTSTR CExpFltr::memType2Text(DWORD memType) {
	(void)memType;
	return const_cast<LPTSTR>(_T("Unknown"));
}

void CExpFltr::SetReportFile(LPTSTR file_name) {
	std::lock_guard<std::mutex> lock(m_crit);
	if (file_name && std::strlen(file_name) < _MAX_PATH) {
		std::strncpy(m_report_file, file_name, _MAX_PATH - 1);
		m_report_file[_MAX_PATH - 1] = 0;
	}
}

void CExpFltr::SetAppTitle(LPTSTR app_title) {
	std::lock_guard<std::mutex> lock(m_crit);
	if (app_title && std::strlen(app_title) < MAX_SZ_APP_TITLE) {
		std::strncpy(m_app_title, app_title, MAX_SZ_APP_TITLE - 1);
		m_app_title[MAX_SZ_APP_TITLE - 1] = 0;
	}
}

EXPORT void ExpFltr_start(void) {}

EXPORT void ExpFltr_end(void) {}
