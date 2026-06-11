#!/bin/sh
# Stage strategic assets into the server "home" (same layout as local build/).
set -eu

APP_DIR="${T4C_APP_DIR:-/srv/t4c}"
ASSETS_DIR="${T4C_ASSETS_DIR:-/opt/t4c-assets}"
T4C_DB_HOST="${T4C_DB_HOST:-mariadb}"
T4C_DB_PORT="${T4C_DB_PORT:-3306}"
T4C_DB_NAME="${T4C_DB_NAME:-t4c_server}"
T4C_DB_USER="${T4C_DB_USER:-t4cuser}"
T4C_DB_PASSWORD="${T4C_DB_PASSWORD:-T4Cpass2026!}"
export T4C_DB_HOST T4C_DB_PORT T4C_DB_NAME T4C_DB_USER T4C_DB_PASSWORD

log() { printf '[t4c-entrypoint] %s\n' "$*"; }

mkdir -p "$APP_DIR/logs" "$APP_DIR/WDA"

if [ -d "$ASSETS_DIR" ] && [ -n "$(ls -A "$ASSETS_DIR" 2>/dev/null || true)" ]; then
    log "Staging assets from ${ASSETS_DIR} -> ${APP_DIR}"
    cp -a "$ASSETS_DIR/." "$APP_DIR/"
else
    log "WARN: no assets in ${ASSETS_DIR}; relying on image copy only"
fi

mkdir -p /etc
if [ -f /opt/t4c-odbc/odbcinst.ini ]; then
    cp /opt/t4c-odbc/odbcinst.ini /etc/odbcinst.ini
fi
if [ -f /opt/t4c-odbc/odbc.ini.template ]; then
    log "Generating /etc/odbc.ini (host=${T4C_DB_HOST})"
    envsubst < /opt/t4c-odbc/odbc.ini.template > /etc/odbc.ini
fi
export ODBCINI=/etc/odbc.ini
export ODBCSYSINI=/etc

missing=0
for f in \
    "$APP_DIR/T4CServer.ini" \
    "$APP_DIR/WDA/T4C Worlds.WDA" \
    "$APP_DIR/WDA/T4C Edit.WDA" \
    "$APP_DIR/WDA/NPCs.WDA"
do
    if [ ! -f "$f" ]; then
        log "MISSING required asset: $f"
        missing=1
    fi
done

if ! ls "$APP_DIR"/*.elng >/dev/null 2>&1; then
    log "MISSING required asset: *.elng in ${APP_DIR} (see LangDB1 in T4CServer.ini)"
    missing=1
fi

if [ ! -x "$APP_DIR/T4CServer" ]; then
    log "MISSING executable: ${APP_DIR}/T4CServer"
    missing=1
fi

if [ "$missing" -ne 0 ]; then
    log "Fill docker/runtime/ (./docker/sync-runtime-from-build.sh) or mount T4C_RUNTIME_ASSETS"
    exit 1
fi

if [ "${T4C_VERIFY_WDA_MD5:-1}" = "1" ]; then
    worlds_md5=$(md5sum "$APP_DIR/WDA/T4C Worlds.WDA" | awk '{print $1}')
    edit_md5=$(md5sum "$APP_DIR/WDA/T4C Edit.WDA" | awk '{print $1}')
    if [ "$worlds_md5" != "f972edd2d4b663dfceb2127c41b0e1c0" ] || \
       [ "$edit_md5" != "3f1dcf1a6d95066bdc39196f378c39fe" ]; then
        log "WARN: WDA MD5 mismatch (expected LP64 pair from CHANGELOG)"
        log "      Worlds.WDA=${worlds_md5}"
        log "      Edit.WDA=${edit_md5}"
        log "      Set T4C_VERIFY_WDA_MD5=0 to skip this check"
    fi
fi

cd "$APP_DIR"
log "Starting T4CServer (cwd=${APP_DIR}) — WDA boot may take several minutes"
exec ./T4CServer
