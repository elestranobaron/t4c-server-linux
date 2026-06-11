# Docker — T4C Server Linux

## Approche

Le serveur tourne dans un répertoire « home » identique à `build/` : `T4CServer`, `T4CServer.ini`, `*.elng`, `WDA/`.

| Rôle | Chemin |
|------|--------|
| Assets (hors repo public) | `docker/runtime/` ou volume `T4C_RUNTIME_ASSETS` |
| Exécution conteneur | `/srv/t4c` (rempli par `entrypoint.sh` depuis `/opt/t4c-assets`) |

## Démarrage

```bash
chmod +x docker/sync-runtime-from-build.sh docker/entrypoint.sh
./docker/sync-runtime-from-build.sh
cp .env.example .env
docker compose up --build
```

Compte test : `test` / `test`. Boot WDA : 1–3 min.

Alternative sans sync :

```bash
T4C_RUNTIME_ASSETS=./build docker compose up --build
```

## Fichiers

- `docker/Dockerfile` — build multi-stage (Arch)
- `docker-compose.yml` — MariaDB + serveur (UDP 11677)
- `docker/entrypoint.sh` — staging + ODBC
- `docker/runtime/` — WDA + elng (à remplir localement)

Les WDA et `.elng` ne doivent pas être poussés sur un repo public.
