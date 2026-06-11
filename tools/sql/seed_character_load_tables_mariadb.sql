-- Tables vides requises par Character::LoadCharacter (apres seed_test_player_mariadb.sql).
-- Sans elles, certaines requetes ODBC echouent ; le serveur doit quand meme repondre opcode 13.

USE t4c_server;

CREATE TABLE IF NOT EXISTS Flags (
  OwnerID       INT UNSIGNED NOT NULL,
  BaseOwnerID   INT UNSIGNED NOT NULL DEFAULT 0,
  FlagID        INT UNSIGNED NOT NULL,
  FlagValue     INT NOT NULL DEFAULT 0,
  DynamicFlag   TINYINT UNSIGNED NOT NULL DEFAULT 1,
  KEY idx_flags_owner (OwnerID, BaseOwnerID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS PlayerItems (
  ObjID     INT UNSIGNED NOT NULL,
  OwnerID   INT UNSIGNED NOT NULL,
  EquipPos  TINYINT UNSIGNED NOT NULL DEFAULT 0,
  ObjType   VARCHAR(50) NOT NULL DEFAULT '',
  Qty       INT UNSIGNED NOT NULL DEFAULT 1,
  KEY idx_playeritems_owner (OwnerID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS ChestItems (
  ObjID     INT UNSIGNED NOT NULL,
  OwnerID   INT UNSIGNED NOT NULL,
  ObjType   VARCHAR(50) NOT NULL DEFAULT '',
  Qty       INT UNSIGNED NOT NULL DEFAULT 1,
  KEY idx_chestitems_owner (OwnerID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS PlayerSkills (
  OwnerID    INT UNSIGNED NOT NULL,
  SkillID    INT UNSIGNED NOT NULL,
  SkillPnts  INT UNSIGNED NOT NULL DEFAULT 0,
  KEY idx_playerskills_owner (OwnerID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS PlayerSpells (
  OwnerID   INT UNSIGNED NOT NULL,
  SpellID   INT UNSIGNED NOT NULL,
  KEY idx_playerspells_owner (OwnerID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS Effects (
  OwnerID         INT UNSIGNED NOT NULL,
  BaseOwnerID     INT UNSIGNED NOT NULL DEFAULT 0,
  EffectID        INT UNSIGNED NOT NULL,
  EffectType      SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  Timer           INT UNSIGNED NOT NULL DEFAULT 0,
  EffectData      VARCHAR(256) NOT NULL DEFAULT '',
  TotalDuration   INT NOT NULL DEFAULT 0,
  BindedSpellID   INT UNSIGNED NOT NULL DEFAULT 0,
  BindedFlagID    INT UNSIGNED NOT NULL DEFAULT 0,
  KEY idx_effects_owner (OwnerID, BaseOwnerID)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS Boosts (
  OwnerID       INT UNSIGNED NOT NULL,
  BaseOwnerID   INT UNSIGNED NOT NULL DEFAULT 0,
  BoostID       INT UNSIGNED NOT NULL,
  Stat          SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  Boost         VARCHAR(256) NOT NULL DEFAULT '',
  KEY idx_boosts_owner (OwnerID, BaseOwnerID)
) ENGINE=InnoDB;
