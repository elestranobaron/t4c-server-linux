#!/usr/bin/env bash
# Réintègre les fichiers manquants après un clone git et prépare le serveur.
# Usage :
#   export T4C_SERVER_ASSET_SRC="/chemin/vers/T4C_V1R7X/1.7/1.7"
#   ./scripts/setup_from_clone.sh            # setup complet (runtime + DB + ODBC + build)
#   ./scripts/setup_from_clone.sh --check    # diagnostic seul, ne modifie RIEN
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CHECK_ONLY=0
[[ "${1:-}" == "--check" ]] && CHECK_ONLY=1

ok()   { printf '  [OK]      %s\n' "$*"; }
miss() { printf '  [MANQUE]  %s\n' "$*"; MISSING=1; }
MISSING=0

# Fichiers critiques attendus dans runtime/ (échantillon représentatif).
CRITICAL_RUNTIME=(
  "WDA/T4C Worlds.WDA"
  "WDA/NPCs.WDA"
  "T4CServer.ini"
  "t4c_eng.lng"
  "SQL/t4c_server_official.sql"
  "SQL/t4c_users_official.sql"
)

echo "=== [1/5] Outils ==="
for tool in cmake g++ isql; do
  if command -v "${tool}" >/dev/null 2>&1; then ok "${tool}"; else miss "${tool} (cmake/g++/unixODBC)"; fi
done

echo "=== [2/5] Runtime serveur (WDA, elng, ini) ==="
if [[ ${CHECK_ONLY} -eq 0 ]]; then
  if [[ ! -f "${ROOT}/runtime/WDA/T4C Worlds.WDA" ]]; then
    if [[ -z "${T4C_SERVER_ASSET_SRC:-}" ]]; then
      echo "  runtime/WDA absent et T4C_SERVER_ASSET_SRC non défini." >&2
      echo "  export T4C_SERVER_ASSET_SRC=\"/chemin/vers/T4C_V1R7X/1.7/1.7\"" >&2
      exit 1
    fi
    "${ROOT}/scripts/copy_server_runtime.sh"
  else
    echo "  runtime/ déjà présent — copie sautée (relancer copy_server_runtime.sh pour rafraîchir)."
  fi
fi

echo "=== [3/5] Fichiers critiques ==="
for f in "${CRITICAL_RUNTIME[@]}"; do
  if [[ -e "${ROOT}/runtime/${f}" ]]; then ok "runtime/${f}"; else miss "runtime/${f}"; fi
done

if [[ ${CHECK_ONLY} -eq 1 ]]; then
  echo "=== DSN ODBC (diagnostic) ==="
  for dsn in "T4C Server Authentication" "T4C Server"; do
    if echo "SELECT 1;" | isql -v "${dsn}" "${T4C_DB_USER:-t4cuser}" "${T4C_DB_PASSWORD:-T4Cpass2026!}" -b >/dev/null 2>&1; then
      ok "DSN [${dsn}]"
    else
      miss "DSN [${dsn}] (scripts/install_odbc_ini.sh + import_official_sql.sh)"
    fi
  done
  echo ""
  if [[ ${MISSING} -eq 0 ]]; then
    echo "Diagnostic : tout est présent. Lancer ./scripts/setup_from_clone.sh (sans --check)."
  else
    echo "Diagnostic : éléments manquants ci-dessus."
  fi
  exit "${MISSING}"
fi

if [[ ${MISSING} -ne 0 ]]; then
  echo ""
  echo "Des fichiers critiques manquent (liste ci-dessus) — vérifier T4C_SERVER_ASSET_SRC." >&2
  exit 1
fi

echo "=== [4/5] Base de données + ODBC ==="
"${ROOT}/scripts/import_official_sql.sh"
# /etc/odbc.ini si sudo possible, sinon ~/.odbc.ini
if [[ -w /etc/odbc.ini ]] || sudo -n true 2>/dev/null; then
  "${ROOT}/scripts/install_odbc_ini.sh"
else
  "${ROOT}/scripts/install_odbc_ini.sh" "${HOME}/.odbc.ini"
fi

echo "=== [5/5] Build serveur ==="
sed -i 's/\r$//' "${ROOT}/build.sh" 2>/dev/null || true
chmod +x "${ROOT}/build.sh"
"${ROOT}/build.sh"

echo ""
echo "Terminé. Lancer le serveur :"
echo "  cd ${ROOT}/build && ODBCINI=\${HOME}/.odbc.ini ./T4CServer"
echo "  (attendre « [BOOT] SERVER_READY » — port UDP 11677)"
