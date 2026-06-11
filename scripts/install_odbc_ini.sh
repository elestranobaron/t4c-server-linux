#!/usr/bin/env bash
# Installe les 3 DSN T4C dans /etc/odbc.ini (sudo) ou ~/.odbc.ini (sans sudo).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${ROOT}/runtime/SQL/odbc.ini"
TARGET="${1:-/etc/odbc.ini}"

if [[ ! -f "${SRC}" ]]; then
  "${ROOT}/scripts/import_official_sql.sh" --odbc-only 2>/dev/null || true
fi

if [[ "${TARGET}" == "/etc/odbc.ini" ]] && [[ ! -w "${TARGET}" ]]; then
  echo "[odbc] installation systeme (sudo requis)…"
  sudo cp "${SRC}" "${TARGET}"
  sudo chmod 644 "${TARGET}"
else
  cp "${SRC}" "${TARGET}"
  chmod 644 "${TARGET}"
fi

echo "[odbc] installe → ${TARGET}"
echo "[odbc] verification DSN:"
for dsn in "T4C Server Authentication" "T4C Server" "T4C Users Web"; do
  if echo "SELECT 1;" | isql -v "${dsn}" t4cuser T4Cpass2026! -b >/dev/null 2>&1; then
    echo "  OK  [${dsn}]"
  else
    echo "  FAIL [${dsn}]" >&2
  fi
done
