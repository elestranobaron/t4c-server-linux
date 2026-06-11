-- T4C Server (Linux port) — bootstrap MINIMAL pour la chaîne auth (register / ODBC).
-- Ce n'est PAS une base « monde » complète (PlayingCharacters riche, WDA, logs SQL, etc.).
--
-- Meilleure pratique production : restaurer un dump / .mdb d'origine du pack serveur T4C.
-- Ce script sert au dev local quand tu n'as plus de tables du tout.
--
-- Prérequis :
--   - MariaDB/MySQL
--   - unixODBC + driver MariaDB
--   - DSN nommé comme dans T4CServer.ini : "T4C Server Authentication"
--   - Utilisateur ODBC = section [characters] : DB_USER / DB_PWD (voir build/T4CServer.ini)
--
-- Colonnes auth par défaut (main.cpp) : table T4CUsers, champs Account + Password.

-- Nom de base : aligner avec le DSN ODBC (ex. Database=t4c_server dans /etc/odbc.ini).
CREATE DATABASE IF NOT EXISTS t4c_server
  CHARACTER SET latin1
  COLLATE latin1_swedish_ci;

CREATE USER IF NOT EXISTS 't4cuser'@'%' IDENTIFIED BY 'T4Cpass2026!';
CREATE USER IF NOT EXISTS 't4cuser'@'localhost' IDENTIFIED BY 'T4Cpass2026!';
GRANT ALL PRIVILEGES ON t4c_server.* TO 't4cuser'@'%';
GRANT ALL PRIVILEGES ON t4c_server.* TO 't4cuser'@'localhost';
FLUSH PRIVILEGES;

USE t4c_server;

-- Comptes (ODBC_TABLE / ODBC_NAME_FLD / ODBC_PWD_FLD)
CREATE TABLE IF NOT EXISTS T4CUsers (
  Account  VARCHAR(64)  NOT NULL,
  Password VARCHAR(128) NOT NULL,
  PRIMARY KEY (Account)
) ENGINE=InnoDB;

-- Sessions en ligne (une ligne par compte connecté ; INSERT échoue si déjà présent)
CREATE TABLE IF NOT EXISTS OnlineUsers (
  MachineName VARCHAR(128) NOT NULL DEFAULT '',
  AccountName VARCHAR(64)  NOT NULL,
  PlayerName  VARCHAR(64)  NOT NULL DEFAULT '<logging on>',
  IPaddr      VARCHAR(32)  NOT NULL DEFAULT '',
  PRIMARY KEY (AccountName),
  KEY idx_onlineusers_ip (IPaddr)
) ENGINE=InnoDB;

-- Liste persos (peut être vide ; requise par Players::LoadAccount)
-- PlayerName VARCHAR(64) : la suppression indirecte (Character::DeleteCharacter) renomme en
--   $YYYYMMDDHHMMSS-RRR$<nom> (~20 octets de préfixe + nom) ; VARCHAR(20) fait échouer opcode 15 code 3.
CREATE TABLE IF NOT EXISTS PlayingCharacters (
  UserID        INT UNSIGNED NOT NULL AUTO_INCREMENT,
  PlayerName    VARCHAR(64)  NOT NULL,
  AccountName   VARCHAR(64)  NOT NULL,
  Appearance    SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  CurrentLevel  INT UNSIGNED NOT NULL DEFAULT 1,
  PRIMARY KEY (UserID),
  UNIQUE KEY uk_playername (PlayerName),
  KEY idx_accountname (AccountName)
) ENGINE=InnoDB;

-- Flags compte (peut être vide)
CREATE TABLE IF NOT EXISTS UserFlags (
  AccountName     VARCHAR(64) NOT NULL,
  FlagBitPosition SMALLINT UNSIGNED NOT NULL,
  ExpireDate      INT NOT NULL DEFAULT 0,
  KEY idx_userflags_account (AccountName)
) ENGINE=InnoDB;

-- Compte de test (le serveur passe les logins en minuscules)
INSERT INTO T4CUsers (Account, Password)
VALUES ('test', 'test')
ON DUPLICATE KEY UPDATE Password = VALUES(Password);

-- Nettoyer une session fantôme avant de retester
-- DELETE FROM OnlineUsers WHERE AccountName = 'test';
