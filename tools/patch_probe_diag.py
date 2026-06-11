from pathlib import Path
import re

p = Path(__file__).resolve().parents[1] / "T4C Server" / "PLAYERS.CPP"
t = p.read_text(encoding="latin-1")

# single ProbeAuthDatabase call after connect only
t = re.sub(
    r"\tProbeAuthDatabase\(\);\n\n    TFormat format;",
    "\tProbeAuthDatabase();\n\n    TFormat format;",
    t,
    count=1,
)
t = re.sub(
    r"\n\tProbeAuthDatabase\(\);\n\}\n\nvoid Players::DestroyODBC",
    "\n}\n\nvoid Players::DestroyODBC",
    t,
    count=1,
)

new_probe = r'''
void Players::ProbeAuthDatabase( void )
{
	fprintf( stderr,
	    "[AUTH] ODBCUsers DSN='%s' user='%s' auth_table='%s' account_col='%s' pwd_col='%s'\n",
	    (LPCTSTR)theApp.csDBDns,
	    (LPCTSTR)theApp.csDBUser,
	    (LPCTSTR)theApp.sAuth.csODBC_Table,
	    (LPCTSTR)theApp.sAuth.csODBC_Account,
	    (LPCTSTR)theApp.sAuth.csODBC_Pwd );

	auto runSelect = []( const char *label, const char *sql ) {
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
	};

	runSelect( "DATABASE()", "SELECT DATABASE()" );
	runSelect( "COUNT(*)", "SELECT COUNT(*) FROM T4CUsers" );
	runSelect( "direct test", "SELECT Password FROM T4CUsers WHERE Account='test'" );

	char szProbePwd[ 64 ];
	const int probe = QueryAccountPassword( "test", szProbePwd, sizeof( szProbePwd ), NULL );
	fprintf( stderr,
	    "[AUTH] QueryAccountPassword('test') => %d (0=OK, 1=462, -1=459) pwd='%s'\n",
	    probe, szProbePwd );
}

'''

t = re.sub(
    r"void Players::ProbeAuthDatabase\( void \)\s*\{[^}]*QueryAccountPassword[^}]*\}\n",
    new_probe,
    t,
    count=1,
    flags=re.DOTALL,
)

# CloseCursor after DELETE if missing
if "DELETE FROM Flags WHERE FlagID='14'" in t and "DELETE FROM Flags" in t:
    block = t.split("DELETE FROM Flags WHERE FlagID='14'")[1].split("ProbeAuthDatabase")[0]
    if "CloseCursor" not in block.split("Flush all online")[0]:
        t = t.replace(
            "\t\t)\n\t);\n\n\n\n    // Flush all online user records.",
            "\t\t)\n\t);\n\tODBCUsers.CloseCursor();\n\n    // Flush all online user records.",
            1,
        )
    block2 = t.split("Flush all online user records")[1].split("}")[0]
    if block2.count("CloseCursor") < 2:
        t = t.replace(
            "(LPCTSTR)theApp.csMachineName \n        ) \n    );\n}",
            "(LPCTSTR)theApp.csMachineName \n        ) \n    );\n\tODBCUsers.CloseCursor();\n}",
            1,
        )

p.write_text(t, encoding="latin-1")
print("probe diag ok")
