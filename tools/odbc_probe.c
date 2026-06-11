#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>

static void log_err(SQLSMALLINT handleType, SQLHANDLE handle, const char *tag)
{
    SQLCHAR state[16], msg[512];
    SQLINTEGER native;
    SQLSMALLINT len;
    for (SQLRETURN r = SQLGetDiagRec(handleType, handle, 1, state, &native, msg, sizeof(msg), &len);
         r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO;
         r = SQLGetDiagRec(handleType, handle, 2, state, &native, msg, sizeof(msg), &len))
        fprintf(stderr, "[%s] %s native=%ld %s\n", tag, state, (long)native, msg);
}

static int probe(const char *dsn, const char *user, const char *pwd, int use_odbc_cursors)
{
    SQLHENV henv = SQL_NULL_HENV;
    SQLHDBC hdbc = SQL_NULL_HDBC;
    SQLHSTMT hstmt = SQL_NULL_HSTMT;
    SQLRETURN rc;

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    if (use_odbc_cursors)
        SQLSetConnectAttr(hdbc, SQL_ATTR_ODBC_CURSORS, (SQLPOINTER)SQL_CUR_USE_ODBC, 0);

    rc = SQLConnect(hdbc, (SQLCHAR *)dsn, SQL_NTS,
                    user ? (SQLCHAR *)user : NULL, user ? SQL_NTS : 0,
                    pwd ? (SQLCHAR *)pwd : NULL, pwd ? SQL_NTS : 0);
    fprintf(stderr, "Connect(%s, %s) rc=%d cursors=%d\n", dsn, user ? user : "(dsn)", (int)rc, use_odbc_cursors);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        log_err(SQL_HANDLE_DBC, hdbc, "connect");

    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    const char *queries[] = {
        "SELECT DATABASE()",
        "SELECT COUNT(*) FROM T4CUsers",
        "SELECT Password FROM T4CUsers WHERE Account='test'",
        NULL
    };

    for (int i = 0; queries[i]; ++i) {
        rc = SQLExecDirect(hstmt, (SQLCHAR *)queries[i], SQL_NTS);
        fprintf(stderr, "ExecDirect rc=%d sql=%s\n", (int)rc, queries[i]);
        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
            log_err(SQL_HANDLE_STMT, hstmt, "exec");
            continue;
        }
        rc = SQLFetch(hstmt);
        fprintf(stderr, "Fetch rc=%d\n", (int)rc);
        if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
            SQLCHAR val[256];
            SQLLEN ind;
            SQLGetData(hstmt, 1, SQL_C_CHAR, val, sizeof(val), &ind);
            fprintf(stderr, "  col1='%s'\n", val);
        } else {
            log_err(SQL_HANDLE_STMT, hstmt, "fetch");
        }
        SQLCloseCursor(hstmt);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return 0;
}

int main(void)
{
    const char *dsn = "T4C Server";
    const char *user = "t4cuser";
    const char *pwd = "T4Cpass2026!";
    probe(dsn, user, pwd, 1);
    probe(dsn, NULL, NULL, 1);
    probe(dsn, user, pwd, 0);
    return 0;
}
