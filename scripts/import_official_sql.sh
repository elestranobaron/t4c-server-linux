#!/usr/bin/env bash
# Importe la structure SQL officielle T4C 1.72 : t4c_server + t4c_users.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SQL_FILE="${ROOT}/runtime/SQL/t4c_server_official.sql"
USERS_SQL="${ROOT}/runtime/SQL/t4c_users_official.sql"
DSN="${T4C_ODBC_DSN:-T4C Server Authentication}"
DB_USER="${T4C_DB_USER:-t4cuser}"
DB_PASS="${T4C_DB_PASSWORD:-T4Cpass2026!}"
DB_NAME="${T4C_DB_NAME:-t4c_server}"
USERS_DB="${T4C_USERS_DB:-t4c_users}"
MARIADB_CONTAINER="${MARIADB_CONTAINER:-t4c-mariadb}"
MARIADB_ROOT_PASSWORD="${MARIADB_ROOT_PASSWORD:-T4Croot2026!}"
ODBC_ONLY=0

for arg in "$@"; do
  [[ "${arg}" == "--odbc-only" ]] && ODBC_ONLY=1
done

write_odbc_ini() {
  local f="${ROOT}/runtime/SQL/odbc.ini"
  cat > "${f}" <<EOF
# DSN officiels T4C 1.72 — copier vers /etc/odbc.ini (sudo) ou ~/.odbc.ini
[T4C Server Authentication]
Driver   = MariaDB ODBC Driver
Description = T4C auth (T4CUsers)
Server   = 127.0.0.1
Port     = 3306
Database = ${DB_NAME}
User     = ${DB_USER}
Password = ${DB_PASS}

[T4C Server]
Driver   = MariaDB ODBC Driver
Description = T4C game data (persos, monde, guildes…)
Server   = 127.0.0.1
Port     = 3306
Database = ${DB_NAME}
User     = ${DB_USER}
Password = ${DB_PASS}

[T4C Users Web]
Driver   = MariaDB ODBC Driver
Description = T4C web registration (accounts)
Server   = 127.0.0.1
Port     = 3306
Database = ${USERS_DB}
User     = ${DB_USER}
Password = ${DB_PASS}
EOF
  cp "${f}" "${ROOT}/runtime/SQL/odbc.ini.snippet"
  echo "[import] ODBC → ${f}"
}

root_exec() {
  local stmt="$1"
  printf '%s\n' "${stmt}" | isql -v -k \
    "Driver={MariaDB ODBC Driver};Server=127.0.0.1;Port=3306;User=root;Password=${MARIADB_ROOT_PASSWORD}" \
    -b 2>&1
}

bootstrap_databases() {
  echo "[import] bootstrap bases ${DB_NAME} + ${USERS_DB}"
  if docker ps --format '{{.Names}}' 2>/dev/null | grep -qx "${MARIADB_CONTAINER}"; then
    docker exec -i "${MARIADB_CONTAINER}" mariadb -uroot -p"${MARIADB_ROOT_PASSWORD}" <<EOF
CREATE DATABASE IF NOT EXISTS ${DB_NAME} CHARACTER SET latin1 COLLATE latin1_swedish_ci;
CREATE DATABASE IF NOT EXISTS ${USERS_DB} CHARACTER SET utf8 COLLATE utf8_general_ci;
CREATE USER IF NOT EXISTS '${DB_USER}'@'%' IDENTIFIED BY '${DB_PASS}';
GRANT ALL PRIVILEGES ON ${DB_NAME}.* TO '${DB_USER}'@'%';
GRANT ALL PRIVILEGES ON ${USERS_DB}.* TO '${DB_USER}'@'%';
FLUSH PRIVILEGES;
EOF
    return
  fi
  root_exec "CREATE DATABASE IF NOT EXISTS ${DB_NAME} CHARACTER SET latin1 COLLATE latin1_swedish_ci;" || true
  root_exec "CREATE DATABASE IF NOT EXISTS ${USERS_DB} CHARACTER SET utf8 COLLATE utf8_general_ci;" || true
  root_exec "GRANT ALL PRIVILEGES ON ${DB_NAME}.* TO '${DB_USER}'@'%';" || true
  root_exec "GRANT ALL PRIVILEGES ON ${USERS_DB}.* TO '${DB_USER}'@'%';" || true
  root_exec "FLUSH PRIVILEGES;" || true
}

import_schema_file() {
  local file="$1"
  local conn="$2"
  echo "[import] ${file}"
  mapfile -t _stmts < <(awk 'BEGIN{RS=";"} {
    gsub(/\r/, "", $0)
    gsub(/\n/, " ", $0)
    gsub(/^[ \t]+|[ \t]+$/, "", $0)
    if (length($0) == 0) next
    if ($0 ~ /^USE /) next
    print $0 ";"
  }' "${file}")

  run_stmt() {
    local stmt="$1"
    [[ -z "${stmt// }" ]] && return 0
    local out
    if [[ -n "${conn}" ]]; then
      out=$(printf '%s\n' "${stmt}" | isql -v -k "${conn}" -b 2>&1) || true
    else
      out=$(printf '%s\n' "${stmt}" | isql -v "${DSN}" "${DB_USER}" "${DB_PASS}" -b 2>&1) || true
    fi
    if grep -qi 'ERROR' <<<"${out}"; then
      if grep -qiE 'already exists|Duplicate key name|Multiple primary key' <<<"${out}"; then
        return 0
      fi
      echo "[import] ERREUR: ${stmt:0:140}..." >&2
      grep -i error <<<"${out}" >&2 || true
      return 1
    fi
    return 0
  }

  local stmt err=0
  for stmt in "${_stmts[@]}"; do
    [[ "${stmt}" == DROP\ TABLE* ]] || continue
    run_stmt "${stmt}" || err=1
  done
  for stmt in "${_stmts[@]}"; do
    [[ "${stmt}" == DROP\ TABLE* ]] && continue
    run_stmt "${stmt}" || err=1
  done
  return "${err}"
}

if [[ "${ODBC_ONLY}" -eq 1 ]]; then
  write_odbc_ini
  exit 0
fi

"${ROOT}/scripts/build_official_sql.sh"
bootstrap_databases
import_schema_file "${SQL_FILE}" "" || exit 1

USERS_CONN="Driver={MariaDB ODBC Driver};Server=127.0.0.1;Port=3306;Database=${USERS_DB};User=${DB_USER};Password=${DB_PASS}"
if [[ -f "${USERS_SQL}" ]]; then
  import_schema_file "${USERS_SQL}" "${USERS_CONN}" || exit 1
fi

write_odbc_ini

echo "[import] t4c_server:"
echo "SHOW TABLES;" | isql -v "${DSN}" "${DB_USER}" "${DB_PASS}" -b 2>/dev/null | grep -E '^\|' | head -35 || true

echo "[import] t4c_users:"
echo "SHOW TABLES;" | isql -v -k "${USERS_CONN}" -b 2>/dev/null | grep -E '^\|' || true

echo "[import] OK — structure officielle complete"
echo "[import] ODBC: sudo ${ROOT}/scripts/install_odbc_ini.sh  (ou cp runtime/SQL/odbc.ini ~/.odbc.ini)"
echo "[import] compte jeu: test / test"
