-- Généré par scripts/build_official_sql.sh — schéma officiel T4C 1.72 pour t4c_server
USE t4c_server;

-- Nettoyage (t4cuser n'a pas DROP DATABASE)
DROP TABLE IF EXISTS `AuctionGive`;
DROP TABLE IF EXISTS `AuctionSold`;
DROP TABLE IF EXISTS `Boosts`;
DROP TABLE IF EXISTS `ChestGuild`;
DROP TABLE IF EXISTS `ChestItems`;
DROP TABLE IF EXISTS `Effects`;
DROP TABLE IF EXISTS `Flags`;
DROP TABLE IF EXISTS `GMMsg`;
DROP TABLE IF EXISTS `GuildLogs`;
DROP TABLE IF EXISTS `GuildMaster`;
DROP TABLE IF EXISTS `GuildUsers`;
DROP TABLE IF EXISTS `LogDeath`;
DROP TABLE IF EXISTS `LogDeath2`;
DROP TABLE IF EXISTS `LogGameOp`;
DROP TABLE IF EXISTS `LogItems`;
DROP TABLE IF EXISTS `LogNPCs`;
DROP TABLE IF EXISTS `LogPage`;
DROP TABLE IF EXISTS `LogPC`;
DROP TABLE IF EXISTS `LogShouts`;
DROP TABLE IF EXISTS `LogText`;
DROP TABLE IF EXISTS `LogWorld`;
DROP TABLE IF EXISTS `OnlineUsers`;
DROP TABLE IF EXISTS `PlayerItems`;
DROP TABLE IF EXISTS `PlayerProfession`;
DROP TABLE IF EXISTS `PlayerSkills`;
DROP TABLE IF EXISTS `PlayerSpells`;
DROP TABLE IF EXISTS `PlayingCharacters`;
DROP TABLE IF EXISTS `UserFlags`;
DROP TABLE IF EXISTS `T4CUsers`;
DROP TABLE IF EXISTS `NMSGold`;
DROP TABLE IF EXISTS `flags`;
DROP TABLE IF EXISTS `_odbc_test`;

CREATE TABLE `AuctionGive` (
  `IndexID` int(11) NOT NULL,
  `AuctionStatus` int(11) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `ObjType` varchar(50) NOT NULL,
  `ObjName` varchar(255) NOT NULL,
  `Qty` int(11) DEFAULT NULL,
  `MadeBy` varchar(50) DEFAULT NULL,
  `Gold` int(11) DEFAULT NULL,
  `Charge` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `AuctionSold` (
  `IndexID` int(11) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `VendeurName` varchar(50) DEFAULT NULL,
  `ObjType` varchar(50) NOT NULL,
  `ObjName` varchar(255) NOT NULL,
  `Qty` int(11) DEFAULT NULL,
  `EquipPos` int(11) DEFAULT NULL,
  `MadeBy` varchar(50) NOT NULL,
  `BuyItNow` int(11) DEFAULT NULL,
  `MinimumBid` int(11) DEFAULT NULL,
  `TimeEnter` int(11) DEFAULT NULL,
  `TimeMax` int(11) DEFAULT NULL,
  `CurrentBid` int(11) DEFAULT NULL,
  `BidOwnerID` int(11) DEFAULT NULL,
  `Charge` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `Boosts` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `BaseOwnerID` int(11) DEFAULT NULL,
  `BoostID` int(11) DEFAULT NULL,
  `Stat` int(11) DEFAULT NULL,
  `Boost` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `ChestGuild` (
  `N°` int(11) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `ObjID` int(11) DEFAULT NULL,
  `ObjType` varchar(50) DEFAULT NULL,
  `Qty` int(11) DEFAULT NULL,
  `Madeby` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `ChestItems` (
  `ID` bigint(20) NOT NULL,
  `ObjID` int(11) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `ObjType` varchar(50) DEFAULT NULL,
  `Qty` int(11) DEFAULT NULL,
  `MadeBy` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `Effects` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `BaseOwnerID` int(11) DEFAULT NULL,
  `EffectID` int(11) DEFAULT NULL,
  `EffectType` int(11) DEFAULT NULL,
  `Timer` int(11) DEFAULT NULL,
  `EffectData` varchar(50) DEFAULT NULL,
  `TotalDuration` int(11) DEFAULT NULL,
  `BindedSpellID` int(11) DEFAULT NULL,
  `BindedFlagID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `Flags` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `BaseOwnerID` int(11) DEFAULT NULL,
  `FlagID` int(11) DEFAULT NULL,
  `FlagValue` int(11) DEFAULT NULL,
  `DynamicFlag` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `GMMsg` (
  `ID` int(11) NOT NULL,
  `Status` int(11) DEFAULT NULL,
  `CreateTime` int(11) DEFAULT NULL,
  `CreateTimeT` varchar(50) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `PlayerName` varchar(64) DEFAULT NULL,
  `Message` varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `GuildLogs` (
  `LogID` int(11) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `TimeInfo` varchar(50) DEFAULT NULL,
  `Logs` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `GuildMaster` (
  `ID` bigint(20) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `Notes` varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `GuildUsers` (
  `ID` bigint(20) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `GuildTitle` int(11) DEFAULT NULL,
  `GuildPermission` int(11) DEFAULT NULL,
  `PlayerLastName` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `LogDeath` (
  `ID` bigint(20) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` text CHARACTER SET latin1
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogDeath2` (
  `ID` bigint(20) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Victime` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Assassin` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Type` int(8) DEFAULT NULL,
  `IsArene` int(8) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogGameOp` (
  `ID` bigint(20) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Loginfo` text CHARACTER SET latin1
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogItems` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogNPCs` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Loginfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogPage` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogPC` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogShouts` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogText` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `LogWorld` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) DEFAULT NULL,
  `Level` varchar(50) DEFAULT NULL,
  `LogInfo` varchar(4096) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE `OnlineUsers` (
  `MachineName` varchar(20) DEFAULT NULL,
  `AccountName` varchar(50) DEFAULT NULL,
  `PlayerName` varchar(64) DEFAULT NULL,
  `IPaddr` varchar(16) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `PlayerItems` (
  `ID` bigint(20) NOT NULL,
  `ObjID` int(11) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `EquipPos` int(11) DEFAULT NULL,
  `ObjType` varchar(50) DEFAULT NULL,
  `Qty` int(11) DEFAULT NULL,
  `MadeBy` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `PlayerProfession` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `FormuleID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `PlayerSkills` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `SkillID` int(11) DEFAULT NULL,
  `SkillPnts` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `PlayerSpells` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `SpellID` int(11) DEFAULT NULL,
  `SpellPnts` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `PlayingCharacters` (
  `UserID` int(11) NOT NULL,
  `PlayerName` varchar(64) DEFAULT NULL,
  `AccountName` varchar(50) DEFAULT NULL,
  `wlX` int(11) DEFAULT NULL,
  `wlY` int(11) DEFAULT NULL,
  `wlWorld` int(11) DEFAULT NULL,
  `nClass` int(11) DEFAULT NULL,
  `CurrentHP` int(11) DEFAULT NULL,
  `MaxHP` int(11) DEFAULT NULL,
  `CurrentMana` int(11) DEFAULT NULL,
  `MaxMana` int(11) DEFAULT NULL,
  `Strength` int(11) DEFAULT NULL,
  `Endurance` int(11) DEFAULT NULL,
  `Agility` int(11) DEFAULT NULL,
  `Intelligence` int(11) DEFAULT NULL,
  `WillPower` int(11) DEFAULT NULL,
  `Wisdom` int(11) DEFAULT NULL,
  `Luck` int(11) DEFAULT NULL,
  `CurrentLevel` int(11) DEFAULT NULL,
  `AttackSkill` int(11) DEFAULT NULL,
  `DodgeSkill` int(11) DEFAULT NULL,
  `Gold` int(11) DEFAULT NULL,
  `Appearance` int(11) DEFAULT NULL,
  `Corpse` int(11) DEFAULT NULL,
  `XP` double DEFAULT NULL,
  `StatPnts` int(11) DEFAULT NULL,
  `SkillPnts` int(11) DEFAULT NULL,
  `Karma` int(11) DEFAULT NULL,
  `Gender` int(11) DEFAULT NULL,
  `ListingTitle` varchar(255) DEFAULT NULL,
  `ListingMisc` varchar(255) DEFAULT NULL,
  `MoveExhaust` int(11) DEFAULT NULL,
  `MentalExhaust` int(11) DEFAULT NULL,
  `AttackExhaust` int(11) DEFAULT NULL,
  `IsDead` int(11) DEFAULT NULL,
  `Crime` int(11) DEFAULT NULL,
  `Honor` int(11) DEFAULT NULL,
  `CompanionName` varchar(50) DEFAULT NULL,
  `CompanionID` int(11) DEFAULT NULL,
  `RPXpPoint` int(11) DEFAULT '0',
  `RPXp` int(11) DEFAULT '0',
  `CreatedAt` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `UpdatedAt` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `UserFlags` (
  `ID` bigint(20) NOT NULL,
  `AccountName` varchar(50) DEFAULT NULL,
  `FlagBitPosition` int(11) DEFAULT NULL,
  `FlagExtraValue` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
ALTER TABLE `AuctionGive`
  ADD PRIMARY KEY (`IndexID`);
ALTER TABLE `AuctionSold`
  ADD PRIMARY KEY (`IndexID`);
ALTER TABLE `Boosts`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `BaseOwnerID` (`BaseOwnerID`);
ALTER TABLE `ChestGuild`
  ADD PRIMARY KEY (`N°`),
  ADD KEY `GuildName` (`GuildName`);
ALTER TABLE `ChestItems`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);
ALTER TABLE `Effects`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `BaseOwnerID` (`BaseOwnerID`);
ALTER TABLE `Flags`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Owner` (`OwnerID`,`BaseOwnerID`);
ALTER TABLE `GMMsg`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `GuildLogs`
  ADD PRIMARY KEY (`LogID`),
  ADD KEY `GuildName` (`GuildName`);
ALTER TABLE `GuildMaster`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `GuildUsers`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogDeath`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogDeath2`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogGameOp`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogItems`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogNPCs`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogPage`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogPC`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogShouts`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogText`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `LogWorld`
  ADD PRIMARY KEY (`ID`);
ALTER TABLE `OnlineUsers`
  ADD UNIQUE KEY `AccountName` (`AccountName`);
ALTER TABLE `PlayerItems`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);
ALTER TABLE `PlayerProfession`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);
ALTER TABLE `PlayerSkills`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);
ALTER TABLE `PlayerSpells`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);
ALTER TABLE `PlayingCharacters`
  ADD PRIMARY KEY (`UserID`),
  ADD UNIQUE KEY `PlayerName` (`PlayerName`),
  ADD KEY `AccountName` (`AccountName`);
ALTER TABLE `UserFlags`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `AccountName` (`AccountName`);
ALTER TABLE `AuctionGive`
  MODIFY `IndexID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `AuctionSold`
  MODIFY `IndexID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `Boosts`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `ChestGuild`
  MODIFY `N°` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `ChestItems`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `Effects`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `Flags`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `GMMsg`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `GuildLogs`
  MODIFY `LogID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `GuildMaster`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `GuildUsers`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogDeath`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogDeath2`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogGameOp`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogNPCs`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogPC`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogShouts`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogText`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `LogWorld`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `PlayerItems`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `PlayerProfession`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `PlayerSkills`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `PlayerSpells`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
ALTER TABLE `PlayingCharacters`
  MODIFY `UserID` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `UserFlags`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;

CREATE TABLE `T4CUsers` (
  `Account`  VARCHAR(64)  NOT NULL,
  `Password` VARCHAR(128) NOT NULL,
  PRIMARY KEY (`Account`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `NMSGold` (
  `nmsAccount`     VARCHAR(64)  NOT NULL,
  `nbrEcu`         INT UNSIGNED NOT NULL DEFAULT 0,
  `numTransaction` VARCHAR(64)  NOT NULL,
  `status`         INT NOT NULL DEFAULT 0,
  KEY `idx_nmsgold_account` (`nmsAccount`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

ALTER TABLE `OnlineUsers` ADD PRIMARY KEY (`AccountName`);

REPLACE INTO `T4CUsers` (`Account`, `Password`) VALUES ('test', 'test');
