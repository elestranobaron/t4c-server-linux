from pathlib import Path

p = Path(__file__).resolve().parents[1] / "T4C Server" / "PLAYERS.CPP"
lines = p.read_text(encoding="latin-1").splitlines(keepends=True)

if any("QueryAccountPassword" in ln for ln in lines):
    print("QueryAccountPassword already present")
    raise SystemExit(0)

end = next(i for i, ln in enumerate(lines) if "int Players::IPLogged" in ln)
del_start = end
for i in range(end - 1, 0, -1):
    if lines[i].strip() == "}":
        del_start = i + 1
        break
del_end = del_start

new_fn = """
//////////////////////////////////////////////////////////////////////////////////////////
int Players::QueryAccountPassword
//////////////////////////////////////////////////////////////////////////////////////////
(
 LPCSTR szAccountQuoted,
 char *szPassword,
 int pwdBufLen,
 DWORD *pdwCredits
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	if( !szAccountQuoted || !szPassword || pwdBufLen < 1 ){
		return -1;
	}
	szPassword[ 0 ] = 0;

	CString csQuery;
	if( !theApp.sAuth.csODBC_Credits.IsEmpty() ){
		csQuery.Format(
		    "SELECT %s, %s FROM %s WHERE %s='%s'",
		    (LPCTSTR)theApp.sAuth.csODBC_Pwd,
		    (LPCTSTR)theApp.sAuth.csODBC_Credits,
		    (LPCTSTR)theApp.sAuth.csODBC_Table,
		    (LPCTSTR)theApp.sAuth.csODBC_Account,
		    szAccountQuoted );
	} else {
		csQuery.Format(
		    "SELECT %s FROM %s WHERE %s='%s'",
		    (LPCTSTR)theApp.sAuth.csODBC_Pwd,
		    (LPCTSTR)theApp.sAuth.csODBC_Table,
		    (LPCTSTR)theApp.sAuth.csODBC_Account,
		    szAccountQuoted );
	}
	if( !theApp.sAuth.csODBC_Where.IsEmpty() ){
		csQuery += " AND (";
		csQuery += theApp.sAuth.csODBC_Where;
		csQuery += ")";
	}

	ODBCUsers.Lock();
	if( !ODBCUsers.SendRequest( (LPCTSTR)csQuery ) ){
		ODBCUsers.CloseCursor();
		ODBCUsers.Unlock();
		_LOG_PC
		    LOG_DEBUG_LVL1,
		    "[QueryAccountPassword] SendRequest ECHEC: %s",
		    (LPCTSTR)csQuery
		LOG_
		return -1;
	}
	if( !ODBCUsers.Fetch() ){
		ODBCUsers.CloseCursor();
		ODBCUsers.Unlock();
		return 1;
	}
	ODBCUsers.GetString( 1, szPassword, pwdBufLen );
	if( pdwCredits != NULL && !theApp.sAuth.csODBC_Credits.IsEmpty() ){
		ODBCUsers.GetDWORD( 2, pdwCredits );
	}
	ODBCUsers.CloseCursor();
	ODBCUsers.Unlock();
	return 0;
}

"""

new_lines = lines[:del_start] + [new_fn] + lines[del_end:]
p.write_text("".join(new_lines), encoding="latin-1")
print(f"inserted QueryAccountPassword before line {del_start + 1}")
