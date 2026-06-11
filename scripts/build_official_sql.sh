#!/usr/bin/env bash
# Génère runtime/SQL/t4c_server_official.sql depuis structure_data.sql (archive 1.72).
# Génère runtime/SQL/t4c_users_official.sql depuis structure_users.sql (web).
# Adaptations Linux : base t4c_server, noms de tables PascalCase (requêtes serveur).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${ROOT}/runtime/SQL/structure_data.sql"
OUT="${ROOT}/runtime/SQL/t4c_server_official.sql"
USERS_SRC="${ROOT}/runtime/SQL/structure_users.sql"
USERS_OUT="${ROOT}/runtime/SQL/t4c_users_official.sql"

if [[ ! -f "${SRC}" ]]; then
  echo "Manquant: ${SRC}" >&2
  exit 1
fi

transform_table_names() {
  sed \
    -e 's/`auctiongive`/`AuctionGive`/g' \
    -e 's/`auctionsold`/`AuctionSold`/g' \
    -e 's/`boosts`/`Boosts`/g' \
    -e 's/`chestguild`/`ChestGuild`/g' \
    -e 's/`chestitems`/`ChestItems`/g' \
    -e 's/`effects`/`Effects`/g' \
    -e 's/`flags`/`Flags`/g' \
    -e 's/`gmmsg`/`GMMsg`/g' \
    -e 's/`guildlogs`/`GuildLogs`/g' \
    -e 's/`guildmaster`/`GuildMaster`/g' \
    -e 's/`guildusers`/`GuildUsers`/g' \
    -e 's/`logdeath`/`LogDeath`/g' \
    -e 's/`logdeath2`/`LogDeath2`/g' \
    -e 's/`loggameop`/`LogGameOp`/g' \
    -e 's/`logitems`/`LogItems`/g' \
    -e 's/`lognpcs`/`LogNPCs`/g' \
    -e 's/`logpage`/`LogPage`/g' \
    -e 's/`logpc`/`LogPC`/g' \
    -e 's/`logshouts`/`LogShouts`/g' \
    -e 's/`logtext`/`LogText`/g' \
    -e 's/`logworld`/`LogWorld`/g' \
    -e 's/`onlineusers`/`OnlineUsers`/g' \
    -e 's/`playeritems`/`PlayerItems`/g' \
    -e 's/`playerprofession`/`PlayerProfession`/g' \
    -e 's/`playerskills`/`PlayerSkills`/g' \
    -e 's/`playerspells`/`PlayerSpells`/g' \
    -e 's/`playingcharacters`/`PlayingCharacters`/g' \
    -e 's/`userflags`/`UserFlags`/g' \
    -e "s/\`PlayerName\` varchar(50)/\`PlayerName\` varchar(64)/" \
    -e 's/`type`/`Type`/g' \
    -e 's/`Isarene`/`IsArene`/g'
}

TABLES=(
  AuctionGive AuctionSold Boosts ChestGuild ChestItems Effects Flags GMMsg
  GuildLogs GuildMaster GuildUsers LogDeath LogDeath2 LogGameOp LogItems LogNPCs
  LogPage LogPC LogShouts LogText LogWorld OnlineUsers PlayerItems PlayerProfession
  PlayerSkills PlayerSpells PlayingCharacters UserFlags T4CUsers NMSGold
  flags _odbc_test
)

{
  echo "-- Généré par scripts/build_official_sql.sh — schéma officiel T4C 1.72 pour t4c_server"
  echo "USE t4c_server;"
  echo ""
  echo "-- Nettoyage (t4cuser n'a pas DROP DATABASE)"
  for t in "${TABLES[@]}"; do
    echo "DROP TABLE IF EXISTS \`${t}\`;"
  done
  echo ""
  transform_table_names < "${SRC}" \
    | grep -vE '^SET SQL_MODE|^SET AUTOCOMMIT|^START TRANSACTION|^SET time_zone|^COMMIT;|^/\*!40101|^--' \
    | sed 's/;COMMIT;/;/g' \
    | sed '/^[[:space:]]*$/d'
  echo ""
  grep -vE '^--' "${ROOT}/runtime/SQL/t4c_server_extras.sql"
} > "${OUT}"

echo "[build_official_sql] OK → ${OUT} ($(wc -l < "${OUT}") lignes)"

if [[ -f "${USERS_SRC}" ]]; then
  {
    echo "-- Généré par scripts/build_official_sql.sh — schéma officiel T4C 1.72 pour t4c_users"
    echo "USE t4c_users;"
    echo ""
    echo "DROP TABLE IF EXISTS \`accounts\`;"
    echo ""
    grep -vE '^SET SQL_MODE|^SET AUTOCOMMIT|^START TRANSACTION|^SET time_zone|^COMMIT;|^/\*!40101|^--' \
      "${USERS_SRC}" \
      | sed 's/;COMMIT;/;/g' \
      | sed '/^[[:space:]]*$/d' \
      | awk '
        /^CREATE TABLE `accounts`/ { in_create=1 }
        in_create && /^\)/ {
          print $0
          print "  PRIMARY KEY (`id`),"
          print "  UNIQUE KEY `username` (`username`)"
          in_create=0
          next
        }
        /^ALTER TABLE `accounts`/ { skip=1; next }
        skip && /AUTO_INCREMENT/ { skip=0; next }
        skip { next }
        { print }
      '
  } > "${USERS_OUT}"
  echo "[build_official_sql] OK → ${USERS_OUT} ($(wc -l < "${USERS_OUT}") lignes)"
fi
