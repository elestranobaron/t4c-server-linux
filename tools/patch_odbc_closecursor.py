from pathlib import Path

p = Path(__file__).resolve().parents[1] / "T4C Server" / "PLAYERS.CPP"
t = p.read_text(encoding="latin-1")

if "DELETE FROM Flags" in t and "DELETE FROM Flags WHERE FlagID='14'" in t:
    t = t.replace(
        "\t\t)\n\t);\n\n\n\n    // Flush all online user records.",
        "\t\t)\n\t);\n\tODBCUsers.CloseCursor();\n\n    // Flush all online user records.",
        1,
    )
    t = t.replace(
        "(LPCTSTR)theApp.csMachineName \n        ) \n    );\n\tProbeAuthDatabase();",
        "(LPCTSTR)theApp.csMachineName \n        ) \n    );\n\tODBCUsers.CloseCursor();\n\tProbeAuthDatabase();",
        1,
    )

needle = "\tODBCUsers.Lock();\n\tif( !ODBCUsers.SendRequest( (LPCTSTR)csQuery ) )"
if needle in t and "\tODBCUsers.CloseCursor();\n\tif( !ODBCUsers.SendRequest" not in t:
    t = t.replace(
        needle,
        "\tODBCUsers.Lock();\n\tODBCUsers.CloseCursor();\n\tif( !ODBCUsers.SendRequest( (LPCTSTR)csQuery ) )",
        1,
    )

p.write_text(t, encoding="latin-1")
print("patched CloseCursor")
