#!/usr/bin/env bash
# Copie les assets runtime serveur depuis l'archive T4C 1.7 officielle.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUNTIME="${ROOT}/runtime"
SRC="${T4C_SERVER_ASSET_SRC:-/mnt/hard/underground/Nouveau dossier (2)/T4C_V1R7X/1.7/1.7}"
DEPLOY="${SRC}/T4CServer/T4CServer172"

if [[ ! -d "${DEPLOY}/WDA" ]]; then
  echo "Source introuvable: ${DEPLOY}/WDA" >&2
  exit 1
fi

mkdir -p "${RUNTIME}/WDA" "${RUNTIME}/logs" "${RUNTIME}/update" "${RUNTIME}/SQL" "${RUNTIME}/WebServer"

echo "[copy] WDA depuis ${DEPLOY}/WDA"
cp -a "${DEPLOY}/WDA/." "${RUNTIME}/WDA/"
rm -f "${RUNTIME}/WDA/Rar.exe" 2>/dev/null || true

# L'archive 1.7 livre parfois des WDA « lock » (8 octets). Complément si stub.
WDA_FALLBACK="${T4C_WDA_FALLBACK:-/mnt/debian/home/tom/Public/T4C_Server_Linux_Final_Step/docker/runtime/WDA}"
if [[ -d "${WDA_FALLBACK}" ]]; then
  declare -A WDA_MIN=(
    ["NPCs.WDA"]=1024
    ["T4C Worlds.WDA"]=1048576
    ["T4C Edit.WDA"]=65536
  )
  for wda in NPCs.WDA "T4C Worlds.WDA" "T4C Edit.WDA"; do
    dst="${RUNTIME}/WDA/${wda}"
    min="${WDA_MIN[$wda]:-64}"
    cur=$(stat -c%s "${dst}" 2>/dev/null || echo 0)
    if [[ ! -f "${dst}" ]] || [[ "${cur}" -lt "${min}" ]]; then
      src="${WDA_FALLBACK}/${wda}"
      if [[ -f "${src}" ]] && [[ $(stat -c%s "${src}") -ge "${min}" ]]; then
        echo "[copy] complément WDA (${cur}o → LP64): ${wda} ← ${WDA_FALLBACK}"
        cp -a "${src}" "${dst}"
      fi
    fi
  done
fi

echo "[copy] langues serveur (.lng)"
for f in t4c_eng.lng t4c_frn.lng t4c_ger.lng; do
  cp -a "${DEPLOY}/${f}" "${RUNTIME}/${f}"
done
cp -a "${RUNTIME}/t4c_frn.lng" "${RUNTIME}/t4c_fr.elng"

echo "[copy] update / identité"
cp -a "${DEPLOY}/update/motd.txt" "${RUNTIME}/update/" 2>/dev/null || true
cp -a "${DEPLOY}/svr.id" "${RUNTIME}/" 2>/dev/null || true

echo "[copy] WebServer + SQL"
cp -a "${DEPLOY}/WebServer/." "${RUNTIME}/WebServer/" 2>/dev/null || true
cp -a "${SRC}/T4CServer/SQL Structure/." "${RUNTIME}/SQL/" 2>/dev/null || true

echo "[copy] pages HTML admin (res/)"
if [[ -d "${ROOT}/Exe Server/res" ]]; then
  mkdir -p "${RUNTIME}/res"
  cp -a "${ROOT}/Exe Server/res/." "${RUNTIME}/res/"
fi

if [[ ! -f "${RUNTIME}/T4CServer.ini" ]]; then
  echo "[copy] T4CServer.ini (template docker, port 11677)"
  cp -a "${ROOT}/docker/runtime/T4CServer.ini" "${RUNTIME}/T4CServer.ini"
fi

cat > "${RUNTIME}/SOURCE.txt" <<EOF
Assets runtime serveur T4C
==========================

Source copiée depuis :
  ${SRC}

Déploiement de référence :
  ${DEPLOY}

Regénérer :
  ${ROOT}/scripts/copy_server_runtime.sh

Variables :
  T4C_SERVER_ASSET_SRC  — racine archive 1.7 (défaut ci-dessus)
EOF

echo "[copy] OK → ${RUNTIME}"
du -sh "${RUNTIME}" "${RUNTIME}/WDA"
