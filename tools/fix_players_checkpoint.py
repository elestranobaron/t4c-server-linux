from pathlib import Path

p = Path(__file__).resolve().parents[1] / "T4C Server" / "PLAYERS.CPP"
t = p.read_text(encoding="latin-1")

start_marker = "/* removed dead AccountLogged INSERT branch */"
end_marker = "/////////////////////////////////////////\n// How many connections from this IP?"

if start_marker not in t:
    raise SystemExit("start marker not found")

new_funcs = """
int Players::QueryAccountPassword
(
 LPCSTR szAccountQuoted,
 char *szPassword,
 int pwdBufLen,
 DWORD *pdwCredits
)
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
	ODBCUsers.CloseCursor();
	if( !ODBCUsers.SendRequest( (LPCTSTR)csQuery ) ){
		ODBCUsers.CloseCursor();
		ODBCUsers.Unlock();
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

static void AuthProbeSelect( const char *label, const char *sql )
{
	ODBCUsers.Lock();
	ODBCUsers.CloseCursor();
	const bool sent = ODBCUsers.SendRequest( sql ) ? true : false;
	const bool gotRow = sent && ODBCUsers.Fetch();
	char buf[ 128 ];
	buf[ 0 ] = 0;
	if( gotRow ){
		ODBCUsers.GetString( 1, buf, sizeof( buf ) );
	}
	ODBCUsers.CloseCursor();
	ODBCUsers.Unlock();
	fprintf( stderr, "[AUTH] %-28s send=%d fetch=%d val='%s' | %s\n",
	    label, sent ? 1 : 0, gotRow ? 1 : 0, buf, sql );
}

void Players::ProbeAuthDatabase( void )
{
	fprintf( stderr,
	    "[AUTH] ODBCUsers DSN='%s' user='%s' auth_table='%s' account_col='%s' pwd_col='%s'\n",
	    (LPCTSTR)theApp.csDBDns,
	    (LPCTSTR)theApp.csDBUser,
	    (LPCTSTR)theApp.sAuth.csODBC_Table,
	    (LPCTSTR)theApp.sAuth.csODBC_Account,
	    (LPCTSTR)theApp.sAuth.csODBC_Pwd );

	AuthProbeSelect( "DATABASE()", "SELECT DATABASE()" );
	AuthProbeSelect( "COUNT(*)", "SELECT COUNT(*) FROM T4CUsers" );
	AuthProbeSelect( "direct test", "SELECT Password FROM T4CUsers WHERE Account='test'" );

	char szProbePwd[ 64 ];
	const int probe = QueryAccountPassword( "test", szProbePwd, sizeof( szProbePwd ), NULL );
	fprintf( stderr,
	    "[AUTH] QueryAccountPassword('test') => %d (0=OK, 1=462, -1=459) pwd='%s'\n",
	    probe, szProbePwd );
}

"""

i0 = t.index(start_marker)
i1 = t.index(end_marker)
t = t[:i0] + new_funcs + t[i1:]
p.write_text(t, encoding="latin-1")
print("fixed")
