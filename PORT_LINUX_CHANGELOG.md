# Port Linux T4Serv3 (1.72) — Changelog

> Pas de commits git. Mise à jour à chaque session.

## Statut global

| Étape | Statut |
|-------|--------|
| Infra build (CMake, build.sh, docker, tools) | ✅ |
| Couche portabilité (Win32Compat, etc.) | ✅ |
| Réseau **NMPacketManager** 1.72 (port `#ifdef`) | ✅ |
| ~~CommCenter~~ (abandonné — T4Serv2 legacy) | ❌ retiré du build |
| PacketManager Linux shim | ❌ retiré |
| linux_npc_region rename headers | ✅ générés (410 symboles) |
| StdAfx.h Linux | ✅ |
| main.cpp guards Linux | ✅ |
| Build Linux **T4CServer** (~124 Mo) | ✅ |
| Runtime serveur (WDA 1.72, .lng, .ini) | ✅ |
| SQL officiel `t4c_server` + `t4c_users` | ✅ |
| ODBC 3 DSN (auth + jeu + web) | ✅ (`~/.odbc.ini`) |
| WDA LP64 loader (spells, objects, creatures) | ✅ porté depuis Final_Step |
| Boot serveur → port UDP **11677** | ✅ (CWDAUtils skip Linux) |
| Test E2E auth headless | ⬜ à relancer (boot ~3–5 min) |
| Test E2E client GUI | ⬜ bloqué par validation E2E auth |

---

## 2026-06-09 05:05:00 — ROADMAP SERVEUR : reste a faire pour terminer le portage (audit, AUCUNE implementation)

Volet serveur de la roadmap globale (volet client : `FinalStEp2/CHANGELOG.md`, meme date). Etat factuel apres stabilisation auth/save/deplacement joueur. But : recenser ce qui manque cote serveur et **comment** le faire. Refs en `Exe Server/...` sauf indication.

### 1. Thread IA des PNJ — DESACTIVE (cause racine « les PNJ ne bougent pas »)

Placement statique des PNJ au boot : **OK** (`NPCstructure::OnServerInitialisation()` `NPCstructure.cpp:133-176` → `WorldMap::create_world_unit(U_NPC,...)`). Mais le **thread IA** n'est jamais demarre sur Linux :

```5426:5432:/mnt/debian/home/tom/Public/FinalStEp0/T4Serv3/Exe Server/main.cpp
         NPCMain::GetInstance();
#ifndef _WIN32
         /* Final_Step : pas de NPCMain::Create() — WDAInitNPC a déjà chargé les PNJ */
#else
         NPCMain::GetInstance().Create();
#endif
```

`NPCMain::Create()` (`NPC Thread.cpp:1158-1161`) demarre `NPCThreadFunc` (`NPC Thread.cpp:180-315`) : wander (L766+), fighting (L464+), `MoveUnit(...)` + broadcast (L656-661), dep/repop via `SubmitNearUnit`. Desactive → PNJ statiques, pas de combat, pas de dialogue spontane.

**Comment faire :** (a) retirer le `#ifndef _WIN32` (ou driver de tick equivalent) ; (b) **re-auditer chaque verrou** pris par `NPCThreadFunc`/`SubmitNearUnit` vis-a-vis du thread UDP et de `PlayerMaintenance` — c'est ici que la threadification avait casse l'enchainement reco. Prerequis deja en place : `CRITICAL_SECTION`→`recursive_mutex`, gardes NULL `GetGodFlags`/`PacketPuppetInfo`. (c) Verifier que `Unit::MoveUnit()` (`Unit.cpp:987+`) broadcast bien les opcodes move (le client sait deja les appliquer). (d) E2E : un PNJ wander doit produire des opcodes move + zero crash.

### 2. Assets WDA manquants (placement complet + quetes WDA + journal)

`runtime/WDA/` ne contient que `DialsoftV2Edit.WDA`, `DialsoftV2NPCs.WDA`. **Manquent** `NPCs.WDA`, `T4C Worlds.WDA`, `T4C Edit.WDA` (cf. `runtime/WDA/README.md`). Sans `T4C Worlds.WDA`, `create_world_unit/GetWorld()` peut echouer. `WDAInitNPC()` (`TFCInit.cpp:1707-1838`) charge ces PNJ d'editeur (interprete `SimpleNPC.cpp`/`Wda/Npc/*`). Certains PNJ introuvables en C++ (Wellan, Herman, Valerius, Kinaata, Aspectra) sont peut-etre **definis en WDA** → invisibles tant que `NPCs.WDA` absent.

**Comment faire :** deployer le pack WDA LP64 complet ; revalider `WDAInitNPC` (try/catch continue silencieusement sur echec, `TFCInit.cpp:1432-1438`).

### 3. Chemins WDA codes en dur (non portables)

`QuestBook.cpp:32` : `strFileP += "WDA\\QuestBook.dat";` (pas de `#ifndef _WIN32`), et `QuestBook.dat` absent du runtime → erreur critique + return (`QuestBook.cpp:37-46`). Verifier aussi `Professions.cpp`, `EventsMaster.cpp` et autres `WDA\\`.

**Comment faire :** separateur portable + livrer `QuestBook.dat`.

### 4. CWDAUtils desactive (LP64)

`TFCInit.cpp:1508-1511` : init `CWDAUtils` **skip sur Linux** (contournement crash LP64). Sur Windows il charge tables item/monstre/sort/summon + skins de quete. Impact : quetes/skins alternatifs indisponibles.

**Comment faire :** porter `CWDAUtils` en LP64 (audit des tailles de structs lues du WDA) ou remplacer par un chargement equivalent.

### 5. Kit de depart vide (config, pas code)

`CreateCharacter()` (`Character.cpp:2977+`) boucle sur `vStartupItems` lue de `T4CServer.ini [software\vircom\the 4th coming server\characters] StartupItem1..N` (`AutoConfigUpdate` `Character.cpp:216-240`). Or `StartupItem1=` **vide** partout (`runtime/T4CServer.ini:9`, build, docker, HallRes). IDs dispo : `__OBJ_TORCH=40015` (`DynObjListing.h:49`), habits 155-156 (`t4c_eng.lng`).

**Comment faire :** renseigner `StartupItem1..N` avec les noms summon officiels (torche + habits). Aucun changement de code.

### 6. Persistance perso — OK, a verifier seulement

`SaveCharacter()` (`Character.cpp:1010-1735`) persiste position (`PlayingCharacters.wlX/Y/World` ~L1591) + inventaire (`PlayerItems` L1067-1274) + flags/boosts/effets. Declencheurs : `AsyncDeletePlayer`→save (`PlayerManager.cpp:1613-1616`), autosave `QueryNextSave` (`PLAYERS.CPP:1106-1117`). ODBC porte (`ODBCMage.cpp:180-184`). Save async (retour FALSE L1735 = batch via `ODBCCharAsyncSave`).

**A faire (verif, pas reecriture) :** confirmer l'aboutissement de la save batch sur deco Linux + DSN MariaDB ; E2E reco doit recharger la **nouvelle** position.

---

## 2026-06-09 04:35:00 — CAUSE RACINE du « perso ne bouge plus » : cle de validation 32 bits comparee en 64 bits (LP64)

**Symptome :** apres le fix du deadlock, les moves etaient rejetes (`[Session] joueur inconnu`) car le joueur `in_game` etait **supprime** juste apres l'entree monde.

**Chaine remontee (avec diagnostics temporaires) :**
1. `DeletePlayer` etait appele sur le joueur **vivant** par le thread de maintenance (`PlayerManager::PlayerMaintenance`, backtrace).
2. Le joueur etait ajoute a `tlPlayersToDelete` via la condition `(IsDeleteFlags() || IsIdle() || dwExitDecompte==0) && !IsLoading()`.
3. Valeurs au moment du delete : `dwExitDecompte=65535` (OK), `IdleTime>round` (OK), mais **`dwKickoutTime (512) < round (513)`** → `IsIdle()` renvoie TRUE.
4. `dwKickoutTime` etait arme par `PutPlayerInGame` sur **cle de validation invalide** : `attendu=0xaa3059fe297e38ff recu=696137983`.
5. **Cause finale :** la cle T4C est un entier **32 bits** sur le fil (le client n'envoie/recoit que 4 octets, `recu=0x297e38ff`). Mais `Players::GetKeyCode()` renvoie un **`long` 64 bits** (8 octets sous Linux LP64) dont les 32 bits hauts etaient parasites (`0xaa3059fe`). La comparaison `GetKeyCode() != lKey` echouait donc **systematiquement** → `dwKickoutTime` arme → joueur kicke ~1-2 s apres l'entree → moves rejetes. Les 32 bits bas (`0x297e38ff`) correspondaient pourtant exactement a la cle envoyee.

**Correctif :** `Players::SetKeyCode`/`GetKeyCode` bornent desormais la cle a 32 bits (`(long)(DWORD)(v & 0xFFFFFFFF)`), pour que generation, stockage, comparaison et envoi soient coherents sur 32 bits (`PLAYERS.CPP`). Validation : le perso avance, et la reconnexion apres entree monde n'arme plus de kick.

## 2026-06-09 04:50:00 — Crash GetNearItems sur reconnexion (Players NULL d'une unite PC orpheline)

**Symptome :** apres le fix de la cle, le deplacement marche ; mais a la **reconnexion** + re-entree monde, `SIGSEGV` dans `Players::GetGodFlags(this=0x0)`.

**Backtrace :** `AsyncRQFUNC_GetNearItems` → `WorldMap::packet_inview_units` → `VerifyStackPacketting` → `VerifyUnitPacketting` → `Unit::QueryInvisible::SendPacketTo` (Unit.cpp:3737) → `ch->GetPlayer()->GetGodFlags()` avec `GetPlayer()==NULL`.

**Cause :** une unite `U_PC` peut subsister en jeu sans `Players` associe (session orpheline en teardown apres logout/reco). `Unit::QueryInvisible::SendPacketTo` derefencait `GetPlayer()` sans garde, alors que `Character::GetGodFlags()` (utilise juste au-dessus) gere deja le cas NULL.

**Correctif :** garde `pl != NULL` avant `pl->GetGodFlags()` dans `Unit::QueryInvisible::SendPacketTo` (`Unit.cpp`), meme schema que les autres derefs proteges. Couvert par l'E2E (le probe envoie `RQ_GetNearItems` a chaque entree monde, scenario `T4C_E2E_WORLD_RECONNECT` + `MOVE_TEST`).

---

## 2026-06-09 03:40:00 — CAUSE RACINE du stall : CRITICAL_SECTION non-recursif (self-deadlock)

**Diagnostic final.** Le backtrace dump du thread `UDPReceivePacketThread` (opcode 60 `GetNearItems`, bloque ~30 s) resolu via `addr2line` donne :

```
GetPlayerResourceFctForSession (PlayerManager.cpp:1771)
 -> PeekPlayerAtEndpoint (PlayerManager.cpp:1846)
  -> MultiReaderSingleWriter::EnterReader() (MultiReaderSingleWriter.h:38)
   -> EnterCriticalSection (Win32Compat.h:299)
    -> __gthread_mutex_lock   [BLOQUE INDEFINIMENT]
```

Le thread de reception (lecteur de la liste joueurs) est bloque parce qu'un **ecrivain detient `m_csWrite` sans jamais le relacher**.

**Cause racine :** sous Win32 un `CRITICAL_SECTION` est **recursif** (re-entrant par le thread proprietaire). Le portage Linux l'emulait avec un `std::mutex` **non-recursif** (`Win32Compat.h`). Or plusieurs chemins prennent le verrou **ecrivain** de la liste des joueurs (`MultiReaderSingleWriter::EnterWriter`) puis **relisent** cette liste pendant l'operation — typiquement le thread de maintenance (`PlayerManager.cpp:1010`) qui, sous `EnterWriter`, appelle `Broadcast::BCObjectRemoved` / `DeletePlayer` qui re-parcourent les joueurs via `EnterReader`. Avec un mutex non-recursif, ce re-`lock()` du **meme** mutex par le **meme** thread = **self-deadlock**. Le verrou ecrivain n'est jamais relache -> tous les lecteurs suivants (thread de reception sur opcode 60, et les paquets de **move**) gelent pour toujours. Declenche au nettoyage d'une **session orpheline** apres entree monde + reconnexion -> "le perso ne peut plus avancer".

**Correctif :** `struct CRITICAL_SECTION { std::recursive_mutex mtx; }` (au lieu de `std::mutex`), restaurant la semantique Win32. Modifie dans `Exe Server/Win32Compat.h` (header compile, vu dans le backtrace), `Exe Server/win32compat.h` et `tools/Win32Compat.h` pour coherence. Aucune autre regression : `CRITICAL_SECTION::mtx` n'est manipule qu'au travers de `Enter/LeaveCriticalSection` (lock/unlock), compatibles avec `recursive_mutex`.

**Validation :** rebuild serveur, puis scenario GUI connexion -> entree monde -> deplacement -> logout -> reconnexion -> entree monde -> deplacement (plus de `[DEADLOCK]`, moves traites).

---

## 2026-06-09 03:20:00 — Diagnostic stall thread de reception (apres fix crash : wedge restant)

**Contexte :** apres le fix du crash opcode 68, le serveur ne meurt plus mais un `[DEADLOCK] Suspected stall in 'NMPacketManager::UDPReceivePacketThread'` apparait apres l'entree monde, puis plus aucun paquet n'est traite (moves ignores, RegisterAccount d'une nouvelle session sans reponse). `SuspendThread` etant un no-op Linux (`Win32Compat.h`), le detecteur ne cause pas le blocage : un **handler synchrone** (dispatch sur le thread de reception) bloque > 30 s sur un paquet. Non reproductible en zone vide localhost (l'e2e n'a pas les PNJ/creatures/joueurs que le client interroge via opcode 68).

**Diagnostic ajoute (Linux) :**
- `PacketManager.cpp` : suit l'opcode en cours de dispatch (`DbgSetReceiveOpcode`) + handler `SIGUSR1` qui imprime la **backtrace** du thread bloque (`backtrace_symbols_fd`).
- `NMPacketManager.cpp` : enregistre le thread de reception au demarrage (`DbgRegisterReceiveThreadForStallDump`).
- `DeadlockDetector.cpp` : quand le stall vise `UDPReceivePacketThread`, appelle `DbgDumpReceiveStall()` → log `[DEADLOCK] receive thread bloque sur opcode=N (...) — backtrace:` suivi de la pile.

→ Permet de localiser en **un seul test** le handler fautif. A retirer une fois la cause corrigee.

---

## 2026-06-09 02:55:00 — CRASH serveur opcode 68 apres reconnexion (le perso n'avance plus)

**Symptome :** apres `connexion → monde → quitter → revenir → monde`, le perso tourne mais n'avance plus. Cote client `[MOVE] -> dir … (udp_open=1)` (envoi OK), cote serveur les datagrammes move 10 o **n'arrivent plus** (seuls quelques 4/18 o passent).

**Cause racine (segfault serveur) :** apres la ré-entrée monde, le client envoie l'opcode **68 `RQ_PuppetInformation`**. Sur le serveur :
```
SIGSEGV  Players::GetGodFlags (this=0x0)   PLAYERS.CPP:380
 ← Character::PacketPuppetInfo            Character.cpp:9837  (pl = 0x0)
 ← TFCMessagesHandler::RQFUNC_PuppetInformation (opcode 68)
```
`Character::PacketPuppetInfo` faisait `pl = GetPlayer(); pl->GetGodFlags()` **sans garde NULL**. `GetPlayer()` renvoie NULL pour une unite non-joueur ou un `Character` dont le lien `Players` n'est pas (re)etabli apres reconnexion → deref NULL → **le serveur meurt**. Les moves suivants partent alors vers un serveur mort (`sendto` reussit sur localhost mais rien n'est traite), d'ou « envoyé mais jamais reçu ».

**Fix :** garde `pl != NULL` avant `GetGodFlags()` dans `Character.cpp` :
- `PacketPuppetInfo` (~l.9837, `GOD_TRUE_INVISIBILITY`)
- chemin combat distance (~l.9156→11344, `GOD_DEVELOPPER`)

Aligne sur le style defensif de la reference 1.68 (`SpellMessageHandler.cpp`: `if(pl != NULL){ … }`). Ne copie pas la ref a la lettre (172 ≠ 168) : seule la garde NULL est reprise.

**Validation :** reproduit puis corrige via la sonde headless `t4c_e2e_auth_probe` (`T4C_E2E_ENTER_WORLD=1 T4C_E2E_WORLD_RECONNECT=1 T4C_E2E_MOVE_TEST=1`) — avant : move non confirme + serveur crashe ; apres : move confirme avant ET apres reconnexion, serveur vivant. Garde ajoutee a `scripts/run_reconnect_sim.sh`.

**Nettoyage logs de diagnostic (anti-spam) :** retire `[UDP] rx datagram`, `[UDP] drop already-received`, `[PKT] interpret entry move`, `[PKT] move/session rx`, `[Move] rx`, `[Move] pas in_game`, `[Move] PickLock timeout` (serveur) et `[MOVE] udp_open`, `[UDP] sendto FAIL` (client).

---

## 2026-06-08 23:50:00 — Chemins logs/EXTBD Linux (anti `logs\…\`)

**Symptome** : a chaque boot, dossiers/fichiers `logs\2026_06_03__05h13_50\` et `EXTBD\000001.bin` dans `build/` (backslash Windows interprete comme nom de fichier).

**Fix** : `main.cpp` — suffixe horodate `…/` sous Linux ; `Character.cpp` — `EXTBD/`. Rebuild `./build.sh`.

---

## 2026-06-04 01:00:00 — Reconnexion : RegisterAccount sans reponse

**Symptome :** client renvoie opcode 14 apres deconnexion ; pas de reponse serveur.

**Cause :** `RQFUNC_RegisterAccount` — si session fantome sur le meme IP/port (`IsPlayerResourceExist`), return silencieux (jamais de « resend auth »).

**Fix :**
- `CPlayerManager::ForceReconnectPurge()` — purge synchrone (meme IP/port UDP)
- `CPlayerManager::ForceReconnectPurgeByAccount()` — purge par nom de compte (client relance)
- RegisterAccount + AccountLogged + ODBC : purge puis auth normale

**Rebuild :** `./build.sh` → redeployer `build/T4CServer` sur la machine 192.168.1.76

---

## 2026-06-03 05:56:30 — Session 10 (port loader WDA LP64)

### Cause racine spells (session 9)

Le parseur Win32 lisait **8 champs de trop** avant les `bool` (`dwMinStr…`, `boSkillExclusion`) → assert `WDAFile::Read(bool&)`.

### Fixes appliqués

| Fichier | Fix |
|---------|-----|
| `WDASpells.cpp` | Séquence LP64 : `dwMinLevel` → `boLineOfSight` (sans stats extra) |
| `WDAObjects.cpp` | Retrait `dwBuyFlagID`, `strSellPrice`, `dblParadePC` ; logs progression |
| `WDACreatures.cpp` | LP64 : 1× `boCanAttack` ; `SkipSection` + table offsets (dev) |
| `WDAFile.h/.cpp` | `Tell()` / `Seek()` |
| `TFCInit.cpp` | `T4C_SKIP_CREATURES` optionnel ; try/catch `WDAInitNPC` ; **skip `CWDAUtils` Linux** (crash LP64 dans `ReadListSpell`) |

### Validation boot

| Test | Résultat |
|------|----------|
| Spells + objects Worlds/Edit | ✅ |
| Hives, area links, clans | ✅ |
| Port **11677** UDP | ✅ (~2 min avec `T4C_SKIP_CREATURES=1` en dev) |

**Note** : `T4C_SKIP_CREATURES=1` n’est **pas** activé par défaut ni dans `run_e2e_test.sh` — raccourci dev uniquement. Boot prod = créatures chargées (~+1–3 min).

### Prochaine action

- [ ] E2E auth sans skip créatures (`./FinalStEp2/scripts/run_e2e_test.sh`, `T4C_E2E_SERVER_WAIT=600`)
- [ ] Optionnel : porter/fix `CWDAUtils` LP64 (quest flags, skins) au lieu du skip Linux

---

## 2026-06-03 05:23:51 — Session 9 (tests E2E + fixes démarrage serveur)

### Tests lancés

Script : **`FinalStEp2/scripts/run_e2e_test.sh`**

| Étape | Résultat |
|-------|----------|
| Preflight ODBC (2 DSN) | ✅ PASS |
| Sonde `t4c_e2e_auth_probe` (14→99→26) | ❌ non exécutée — serveur crash |
| Journal horodaté | `FinalStEp2/logs/e2e/20260603_052351/` |

**Exit serveur** : assert `WDAFile::Read(bool&)` — chargement spells WDA LP64.

### Fixes serveur appliqués (démarrage)

| Fichier | Fix |
|---------|-----|
| `T4CLog.cpp` | `GetLog()` null-safe avant init logs (static NPC) |
| `PLAYERS.CPP` | `CAutoLoad` sans `theApp.csT4CKEY` en static init Linux |
| `main.cpp` | `csT4CKEY` dans ctor ; `AddTrailingBackslash` → `/` sur Linux |
| `T4CLog.cpp` | IOCP log : clés 64-bit (`uintptr_t`) |
| `Win32Compat.h` | `CloseHandle` ne `fclose` pas les handles stub |
| `TFC_MAIN.cpp` | `LoadDLLList` → TRUE (NPCs statiques Linux) |
| `TFCInit.cpp` | chemins WDA `./WDA/` ; skip `DialsoftV2Edit` stub |
| `WDAFile.cpp` | EOF → exception (sans assert) |
| `copy_server_runtime.sh` | fallback WDA LP64 si stub (< 1 Mo Worlds) |

### Assets WDA corrigés

Source LP64 (MD5 docker) :

| Fichier | MD5 |
|---------|-----|
| `T4C Worlds.WDA` | `f972edd2d4b663dfceb2127c41b0e1c0` |
| `T4C Edit.WDA` | `3f1dcf1a6d95066bdc39196f378c39fe` |

Chemin : `T4C_Server_Linux_Final_Step/docker/runtime/WDA/`

### Outil E2E client

- **`FinalStEp2/src/tools/e2e_auth_probe.cpp`** — auth headless sans SDL3 GUI
- Cible CMake : `t4c_e2e_auth_probe`
- **`FinalStEp2/scripts/run_e2e_test.sh`** — orchestration + journaux horodatés

### Prochaine action

- [ ] Déboguer `WDASpells::CreateFrom` / offset tables WDA LP64 (assert bool)
- [ ] Relancer `./scripts/run_e2e_test.sh` jusqu'à PASS auth
- [ ] Test client GUI : `./scripts/run_client.sh 127.0.0.1 11677`

---

## 2026-06-03 04:49:17 — Session 8 (runtime, SQL officiel, ODBC)

### Fait — runtime serveur

- Dossier **`runtime/`** (~43 Mo) depuis archive officielle 1.7 :
  - Source : `T4C_V1R7X/1.7/1.7/T4CServer/T4CServer172/`
  - WDA : pack avec **`ServerMap.bin`** (~37 Mo) ; complément **`NPCs.WDA`** si stub 8 o
  - Langues : `t4c_*.lng`, `t4c_fr.elng`
  - Config : `T4CServer.ini`, `SQL/`, `WebServer/`, `res/` HTML
- Script : **`scripts/copy_server_runtime.sh`** (reproductible, `runtime/SOURCE.txt`)
- Lancement : **`FinalStEp2/scripts/run_server.sh`** (rsync runtime → build, exec `T4CServer`)
- Port UDP **11677** (`RECV_PORT` / `SEND_PORT` dans `runtime/T4CServer.ini`)

### Fait — base de données (structure officielle complète)

Scripts :

| Script | Rôle |
|--------|------|
| `scripts/build_official_sql.sh` | Génère `t4c_server_official.sql` + `t4c_users_official.sql` |
| `scripts/import_official_sql.sh` | Import MariaDB des **deux** bases |
| `scripts/install_odbc_ini.sh` | Installe les 3 DSN |

Sources officielles :

| Fichier archive | Base cible | Contenu |
|-----------------|------------|---------|
| `runtime/SQL/structure_data.sql` | `t4c_server` | ~30 tables jeu (PascalCase Linux) |
| `runtime/SQL/structure_users.sql` | `t4c_users` | table `accounts` (web) |

Adaptations Linux :

- Noms tables **PascalCase** (`PlayingCharacters`, `AuctionGive`, …) — requêtes serveur
- Extras : `T4CUsers`, `NMSGold`, PK `OnlineUsers`, compte **`test` / `test`**
- Import statement-by-statement via `isql` (batch ODBC instable)
- Bootstrap root MariaDB : `MARIADB_ROOT_PASSWORD` défaut **`T4Croot2026!`** (conteneur `t4c-mariadb`)

État DB vérifié :

- **`t4c_server`** : ~30 tables, compte `test/test` dans `T4CUsers`
- **`t4c_users`** : table `accounts`

### Fait — ODBC

Fichier de référence : **`runtime/SQL/odbc.ini`** (3 DSN)

| DSN | Base | Usage serveur |
|-----|------|---------------|
| `T4C Server Authentication` | `t4c_server` | Auth (`ODBC_DSN`, table `T4CUsers`) |
| `T4C Server` | `t4c_server` | Données jeu (`ODBC_DB_SRCNAME` / `USERS_DSN`) |
| `T4C Users Web` | `t4c_users` | Inscription web (`accounts`) |

- Driver : **`MariaDB ODBC Driver`** → `/usr/lib/mariadb/libmaodbc.so`
- Installé dans **`~/.odbc.ini`** — les 3 DSN répondent OK à `isql`
- Pour installation système : `sudo scripts/install_odbc_ini.sh`

### Prérequis E2E restants

```bash
# 1. ODBC système (si serveur lancé hors session utilisateur courante)
sudo /mnt/debian/home/tom/Public/FinalStEp0/T4Serv3/scripts/install_odbc_ini.sh

# 2. Serveur
/mnt/debian/home/tom/Public/FinalStEp2/scripts/run_server.sh

# 3. Client (rebuild si binaire antérieur au fix port 11677)
/mnt/debian/home/tom/Public/FinalStEp2/scripts/build_client.sh
/mnt/debian/home/tom/Public/FinalStEp2/scripts/run_client.sh 127.0.0.1 11677
```

---

## 2026-06-03 03:17 — Session 7 (build vert + corrections compile)

### Fait

- **Build vert** : `build/T4CServer` (~124 Mo), `build.sh` + `FinalStEp2/scripts/build_server.sh`
- Corrections compile Exe Server :
  - Retrait `inline` sur définitions dans headers (`Unit.h`, `Character.h`, `WorldMap.h`, `PLAYERS.H`)
  - `Format.h` inclus depuis `NPCmacroScriptLng.h`
  - `TFCPacket.cpp` : surcharge `Get(std::uint32_t*)`
  - `TFCException.cpp`, `MonsterStatDestroy_*`, `ProfessionTrainerNPC`
  - Chemins WDA Linux dans `TFCInit.cpp`
- Port réseau aligné client/serveur : **`PORT_SERVER 11677`** (`UDP/NMPacketManager.h`)

### Chemins

| Rôle | Chemin |
|------|--------|
| Dépôt serveur | `/mnt/debian/home/tom/Public/FinalStEp0/T4Serv3` |
| Binaire | `T4Serv3/build/T4CServer` |
| Runtime | `T4Serv3/runtime/` |
| Scripts client | `FinalStEp2/scripts/run_server.sh`, `build_server.sh` |

---

## 2026-06-01 17:00 — Session 2 (implémentation)

### Fait
- Copié infra T4Serv2 → T4Serv3 : `build.sh`, `CMakeLists.txt`, `docker/`, `tools/`, `Crypto/`
- Copié couche portabilité dans `Exe Server/` : Win32Compat, CommCenter, EXPFLTR_linux, etc.
- `PacketManager.h` bifurqué (`T4C_LINUX_BUILD` → `PacketManager_linux_impl.h` + `PacketManager_linux.cpp`)
- `StdAfx.h` réécrit avec bloc `#ifndef _WIN32`
- `main.cpp` : guards CrashRpt, WebServer, NMPacketManager, service Linux
- Stubs `StdAfx.h` dans les 8 dossiers `Dll Npcs*`
- Généré `linux_npc_region/*_rename.h` (8 régions)
- Restauré `Crypto/crypt.h` + `xorkey.h` depuis git (cassés sur Linux par alias Windows)
- CMake configure OK (SDL3, OpenSSL, ODBC)
- **Build WSL démarré** — erreurs corrigées au fil de l'eau :
  - SharedStructures.h, Players.h (WINSOCK)
  - GenRef.h (macro MSVC `##classname`)
  - StandardTypes.h (`__int64`, LPWORD, strcpy_s, CPtrArray)
  - Format.h (vsprintf_s → vsprintf T4Serv2)
  - Unit.h SendPrivateMessage const ref
  - random.h dice const ref
  - NPCmacroScriptLng.h FROM_NPC (`.##X` → `.X`)

### En cours
- `make -j4` WSL (lent sur /mnt/c, ~1600 fichiers)

### Reste à faire
- [ ] Fin cycles compile-fix (link errors restants)
- [ ] Merger patches Linux fichiers cœur restants depuis T4Serv2
- [ ] Link errors (symboles manquants)

---

## 2026-06-01 20:00 — Session 3 (Exe Server compile-fix)

### Fait
- `Win32Compat.h` : `timeGetTime()`, `CRITICAL_SECTION` + API associées
- Guards `#ifdef _WIN32` sur `mmsystem.h` / `process.h` (Arena*, Character, Unit, WorldMap, Events…)
- `T4CLog.h` : types étendus (`LOG_ARENA`, `LOG_AH`, …), `NB_LOG_TYPES`, macros `_LOG_*`
- `Portability.h` : `CPtrArray::RemoveAt`, `SetAtGrow`
- `TFC Server.h` : stub `CStringArray`
- `TFCPacket.cpp` : `operator<< (const CString&)` + `GetDebugPacketString` Linux
- Suppression casts `(CString &)` invalides dans `Exe Server/*.cpp`
- Outils : `tools/compile_one.sh`, `compile_all_exe.sh`, `compile_key.sh`
- **Vérifié OK** : Arena1Master, AsyncFuncQueue, main, Character, TFC_MAIN, AutoConfig, CommCenter, EXPFLTR_linux

### En cours
- **Build déplacé sur FS WSL natif** : `~/Server_FinalStep/T4Serv3` (`\\wsl.localhost\Debian\home\toto\Server_FinalStep\T4Serv3`)
- Script : `tools/setup_server_finalstep.sh` (rsync + cmake + make)

---

## 2026-06-01 22:10 — Session 4 (WSL natif)

### Fait
- Copie projet → `/home/toto/Server_FinalStep/T4Serv3` (sans `build/`, sans `.git`)
- Miniconda3 supprimé (`C:\Users\tom\miniconda3`), profil PowerShell vide
- Build relancé depuis FS Linux (cmake en cours)

### Chemins
| Côté | Chemin |
|------|--------|
| WSL | `~/Server_FinalStep/T4Serv3` |
| Windows/Cursor | `\\wsl.localhost\Debian\home\toto\Server_FinalStep\T4Serv3` |
| Log build | `~/Server_FinalStep/build.log` |

### Notes
- Windows (`*.vcproj`, `.sln`) non touché
- Build : `cd T4Serv3/build && cmake .. && make -j4`

---

## 2026-06-01 23:10 — Session 5 (normalisation casse Linux)

### Problème
Sur NTFS (Windows), des paires comme `xorkey.h` / `Xorkey.h` ou `crypt.h` / `Crypt.h` sont **fusionnées** en un seul fichier. Sur Linux (case-sensitive), les `#include` qui référencent l’autre casse échouent.

### Solution
- Script **`tools/normalize_linux_case.py`** :
  1. **Crypto** : fichier **minuscule** = contenu réel ; **MixedCase.h** = alias `#include "minuscule.h"`
  2. **Reste du projet** : scan de tous les `#include "..."` → **symlinks** vers le nom réel sur disque
- Remplace l’ancien `tools/linux_case_symlinks.sh` (liste manuelle incomplète)
- Intégré dans `tools/continue_finalstep.sh` (avant cmake/make)

### Usage
```bash
cd ~/Server_FinalStep/T4Serv3
python3 tools/normalize_linux_case.py .
rm -rf build && mkdir build && cd build && cmake .. && make -j4
```

### Exemple Crypto (comme T4Serv2)
| Fichier | Rôle |
|---------|------|
| `xorkey.h` | clés XOR (contenu) |
| `Xorkey.h` | alias → `#include "xorkey.h"` |
| `crypt.h` | API TFCCrypt (contenu) |
| `Crypt.h` | alias → `#include "crypt.h"` |

---

## 2026-06-02 02:45 — Session 6 (pivot réseau fidèle 1.72)

### Décision
**Abandon de CommCenter / `PacketManager_linux.cpp`** (héritage T4Serv2, protocole wire différent — TFCCrypt vs NMP+zlib).  
Objectif : **port fidèle T4Serv3 1.72** — client 1.72 ↔ serveur Linux 1.72, même stack que Windows.

T4Serv2 reste modèle pour la **méthode** (`#ifdef _WIN32`, CMake, tools, normalize casse) — **pas** pour substituer des sous-systèmes.

### Principe portabilité
```
Windows : NMPacketManager + UDPClient (WSA overlapped) + PacketManager.cpp
Linux   : même fichiers, blocs #else natifs (pthread, recvfrom/sendto, IOCP→queue Win32Compat)
Pas de shim : pas de remplacement par une autre couche réseau
```

### CMake (déjà appliqué)
- **In** : `UDP/*.cpp`, `PacketManager.cpp`
- **Out** : `CommCenter.cpp`, `PacketManager_linux.cpp`, `WebServer/*`
- **Link** : `zlib` (`find_package(ZLIB)` ou `-lz`)

### Fichiers modifiés / en cours
| Fichier | Changement |
|---------|------------|
| `CMakeLists.txt` | pivot build réseau |
| `PacketManager.h` | header unifié 1.72 (WebServer `#ifdef _WIN32` only) |
| `UDP/Socket.h`, `Socket.cpp` | `#ifdef` WSA vs fd Linux |
| `UDP/UDPClient.cpp` | `#ifdef` sockets Linux natifs |
| `UDP/NMPacketManager.cpp` | includes, `Reverse16`, IOCP clés 64-bit (`uintptr_t`) |
| `PacketManager.cpp` | `Reverse()` portable (fini `__asm`) |
| `main.cpp` | `m_bCanStartComm = TRUE` aussi sur Linux |
| `Character.cpp` | `#include "MD5/MD5Checksum.h"` (slash forward) |

### Build précédent (avant pivot)
- **~66 %** — NPCs + Gameops OK (normalisation casse validée)
- Échec Exe Server : `MD5\`, `SendPacket` dual-socket (résolu par pivot NMP), `ASSERT`

### Reste (session 6 — obsolète, voir sessions 7–8)
- [x] Finir port `UDPClient.cpp` / `NMPacketManager.cpp`
- [x] Compile-fix Exe Server → link
- [x] Runtime + SQL + ODBC
- [x] Test E2E client 1.72 ↔ serveur Linux (11677)

---

## 2026-06-03 19:37 — Session 11 (auth E2E 14→99→26 PASS)

### Cause racine auth (ODBC)
- **`SQL_CUR_USE_ODBC`** (curseur legacy Windows) incompatible avec MariaDB/unixODBC : `SQLFetch` échoue sans `SQLBindCol` préalable alors que le code utilise `SQLGetData` après fetch.
- **Fix** : `ODBCMage::Connect()` utilise **`SQL_CUR_USE_DRIVER`** sur Linux.

### Auth login / protocole
- **`FetchAuthPassword`** : lecture mot de passe via connexion **`ODBCUsers`** (plus de 2ᵉ connexion `ODBCAuthRead` défaillante).
- **`TFCServer.ini`** : `Version=1720` dans `[generalconfig]` ; défaut Linux **`dwVersion=1720`** dans `TFC_MAIN.cpp` (au lieu de `0x000E`).
- **`TFCPacket::Get(long)`** serveur : borne lecture **4 octets** (wire int32) sur LP64.

### Client (FinalStEp2)
- **`TFCPacket::Get(long)`** : même fix wire 4 octets (LP64 lisait 8 octets → `TFCPacketException` sur réponse opcode 14).
- **`T4CLoginSessionHandlePacket`** : catch `TFCPacketException` (log + arrêt propre).

### Validation
```bash
T4C_E2E_SKIP_CREATURES=1 ./scripts/run_e2e_test.sh
# PASS auth E2E (test@127.0.0.1:11677) — 14 → 99 → 26
```
Boot probe : `[BOOT] ODBCUsers auth probe test => 'test'`

---

## 2026-06-03 22:15 — Session 12 (reconnexion auth — ghost OnlineUsers)

### Symptôme
Deuxième login même compte après quit client :
```
[AUTH] reply ok=1 loggedOn=1 …
[AUTH] reject: AccountLogged('test')
```

### Cause
`AccountLogged()` fait `INSERT INTO OnlineUsers` ; si le client quitte sans `Logoff()` (fermeture fenêtre en sélection perso), la ligne reste → INSERT échoue → « déjà connecté ».

### Fix (`PLAYERS.CPP`)
Sur échec INSERT : `DELETE FROM OnlineUsers WHERE AccountName=…` puis **retry INSERT** une fois (fantôme stale).

### Note
`[DEADLOCK] Suspected stall in 'Player Maintenance Thread'` = garde-fou Linux (pas de exit 106) — bruit connu, non bloquant auth.
