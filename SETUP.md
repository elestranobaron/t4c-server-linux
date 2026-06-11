# SETUP — Serveur T4C 1.72 Linux (T4Serv3)

Méthode **une seule commande** pour réintégrer les fichiers manquants après un clone.

> Les données serveur (WDA, elng… ≈ 66 Mo, propriétaires Vircom) ne sont **pas**
> dans git. Elles sont réintégrées depuis l'archive officielle T4C_V1R7X que le
> testeur doit posséder. Les dumps SQL de structure restent dans `runtime/SQL/`.

## Ce que git contient / ne contient pas

| Dans git                                      | PAS dans git (réintégré par le setup) |
|------------------------------------------------|----------------------------------------|
| Sources (`Exe Server/`, `Dll Npcs*/`, …)       | `runtime/WDA/`, elng, `T4CServer.ini` (secrets DB) |
| `runtime/SQL/*.sql` (structure base)           | `build/` (binaire + données de run)     |
| `scripts/`, `docker/`, `CMakeLists.txt`        | `Logs/`, `EXTBD/`, `svr.id`            |

## Prérequis (Debian/Ubuntu/Arch)

- `cmake`, `g++` (C++17)
- `unixODBC` (+ `isql`), **MariaDB ODBC Driver**
- MariaDB : local **ou** conteneur Docker (`docker/`, conteneur `t4c-mariadb`)
- L'archive officielle **T4C_V1R7X** (contient `T4CServer/T4CServer172/WDA/…`)

## Méthode en 1 coup

```bash
git clone <repo-serveur> FinalStEp0
cd FinalStEp0/T4Serv3

# Chemin vers l'archive T4C 1.7 du testeur (dossier contenant T4CServer/) :
export T4C_SERVER_ASSET_SRC="/chemin/vers/T4C_V1R7X/1.7/1.7"

./scripts/setup_from_clone.sh
```

Le script fait, dans l'ordre :

1. **Vérif outils** (cmake, g++, isql/unixODBC)
2. **Réintégration runtime** (WDA, elng, ini → `runtime/`) via `scripts/copy_server_runtime.sh`
3. **Base de données** : import structure `t4c_server` + `t4c_users` (`scripts/import_official_sql.sh`)
4. **DSN ODBC** : installation `/etc/odbc.ini` ou `~/.odbc.ini` (`scripts/install_odbc_ini.sh`)
5. **Build** (`./build.sh` — ne supprime jamais `build/`)

Lancement :

```bash
cd build && ODBCINI=~/.odbc.ini ./T4CServer
# Attendre : [BOOT] SERVER_READY  (port UDP 11677)
```

## Vérifier sans rien copier

```bash
./scripts/setup_from_clone.sh --check
```

## Secrets

`runtime/T4CServer.ini` contient les identifiants DB (`DB_USER`/`DB_PWD`) —
il est **ignoré par git**. Il est fourni par l'archive officielle puis adapté ;
les valeurs par défaut des scripts (`t4cuser` / `T4Cpass2026!`) doivent être
changées pour tout déploiement non local.

## Test E2E avec le client

Voir `FinalStEp2/SETUP.md` — le script `run_e2e_test.sh` du client démarre ce
serveur, attend `SERVER_READY` et déroule auth + entrée monde + dialogue PNJ.
