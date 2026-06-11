from pathlib import Path

p = Path(__file__).resolve().parents[1] / "T4C Server" / "PLAYERS.CPP"
t = p.read_text(encoding="latin-1")

probe_fn = """
void Players::ProbeAuthDatabase( void )
{
	fprintf( stderr,
	    "[AUTH] ODBCUsers DSN='%s' user='%s' auth_table='%s' account_col='%s' pwd_col='%s'\\n",
	    (LPCTSTR)theApp.csDBDns,
	    (LPCTSTR)theApp.csDBUser,
	    (LPCTSTR)theApp.sAuth.csODBC_Table,
	    (LPCTSTR)theApp.sAuth.csODBC_Account,
	    (LPCTSTR)theApp.sAuth.csODBC_Pwd );
	char szProbePwd[ 64 ];
	const int probe = QueryAccountPassword( "test", szProbePwd, sizeof( szProbePwd ), NULL );
	fprintf( stderr,
	    "[AUTH] probe compte 'test' => %d (0=OK, 1=absent/462, -1=erreur SQL/459)\\n",
	    probe );
}

"""

if "void Players::ProbeAuthDatabase" not in t:
    t = t.replace(
        "void Players::DestroyODBC( void )",
        probe_fn + "\nvoid Players::DestroyODBC( void )",
        1,
    )

if "ProbeAuthDatabase();" not in t:
    t = t.replace(
        "(LPCTSTR)theApp.csMachineName \n        ) \n    );\n}",
        "(LPCTSTR)theApp.csMachineName \n        ) \n    );\n\tProbeAuthDatabase();\n}",
        1,
    )

if 'fprintf( stderr, "[QueryAccountPassword] SendRequest ECHEC' not in t:
    t = t.replace(
        "\t\treturn -1;\n\t}\n\tif( !ODBCUsers.Fetch() )",
        '\t\tfprintf( stderr, "[QueryAccountPassword] SendRequest ECHEC: %s\\n", (LPCTSTR)csQuery );\n\t\treturn -1;\n\t}\n\tif( !ODBCUsers.Fetch() )',
        1,
    )

if 'fprintf( stderr, "[QueryAccountPassword] aucune ligne (462)' not in t:
    t = t.replace(
        "\t\treturn 1;\n\t}\n\tODBCUsers.GetString( 1, szPassword, pwdBufLen );",
        '\t\tfprintf( stderr, "[QueryAccountPassword] aucune ligne (462): %s\\n", (LPCTSTR)csQuery );\n\t\treturn 1;\n\t}\n\tODBCUsers.GetString( 1, szPassword, pwdBufLen );',
        1,
    )

p.write_text(t, encoding="latin-1")
print("ok")
