-- phpMyAdmin SQL Dump
-- version 4.7.0
-- https://www.phpmyadmin.net/
--
-- Hôte : 127.0.0.1
-- Généré le :  sam. 24 juin 2017 à 21:06
-- Version du serveur :  10.1.22-MariaDB
-- Version de PHP :  7.1.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Base de données :  `t4c_data`
--

-- --------------------------------------------------------

--
-- Structure de la table `auctiongive`
--

CREATE TABLE `auctiongive` (
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

-- --------------------------------------------------------

--
-- Structure de la table `auctionsold`
--

CREATE TABLE `auctionsold` (
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

-- --------------------------------------------------------

--
-- Structure de la table `boosts`
--

CREATE TABLE `boosts` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `BaseOwnerID` int(11) DEFAULT NULL,
  `BoostID` int(11) DEFAULT NULL,
  `Stat` int(11) DEFAULT NULL,
  `Boost` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `chestguild`
--

CREATE TABLE `chestguild` (
  `N°` int(11) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `ObjID` int(11) DEFAULT NULL,
  `ObjType` varchar(50) DEFAULT NULL,
  `Qty` int(11) DEFAULT NULL,
  `Madeby` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `chestitems`
--

CREATE TABLE `chestitems` (
  `ID` bigint(20) NOT NULL,
  `ObjID` int(11) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `ObjType` varchar(50) DEFAULT NULL,
  `Qty` int(11) DEFAULT NULL,
  `MadeBy` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `effects`
--

CREATE TABLE `effects` (
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

-- --------------------------------------------------------

--
-- Structure de la table `flags`
--

CREATE TABLE `flags` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `BaseOwnerID` int(11) DEFAULT NULL,
  `FlagID` int(11) DEFAULT NULL,
  `FlagValue` int(11) DEFAULT NULL,
  `DynamicFlag` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `gmmsg`
--

CREATE TABLE `gmmsg` (
  `ID` int(11) NOT NULL,
  `Status` int(11) DEFAULT NULL,
  `CreateTime` int(11) DEFAULT NULL,
  `CreateTimeT` varchar(50) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `PlayerName` varchar(50) DEFAULT NULL,
  `Message` varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `guildlogs`
--

CREATE TABLE `guildlogs` (
  `LogID` int(11) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `TimeInfo` varchar(50) DEFAULT NULL,
  `Logs` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `guildmaster`
--

CREATE TABLE `guildmaster` (
  `ID` bigint(20) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `Notes` varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `guildusers`
--

CREATE TABLE `guildusers` (
  `ID` bigint(20) NOT NULL,
  `GuildName` varchar(50) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `GuildTitle` int(11) DEFAULT NULL,
  `GuildPermission` int(11) DEFAULT NULL,
  `PlayerLastName` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logdeath`
--

CREATE TABLE `logdeath` (
  `ID` bigint(20) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` text CHARACTER SET latin1
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logdeath2`
--

CREATE TABLE `logdeath2` (
  `ID` bigint(20) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Victime` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Assassin` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `type` int(8) DEFAULT NULL,
  `Isarene` int(8) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `loggameop`
--

CREATE TABLE `loggameop` (
  `ID` bigint(20) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Loginfo` text CHARACTER SET latin1
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logitems`
--

CREATE TABLE `logitems` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `lognpcs`
--

CREATE TABLE `lognpcs` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Loginfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logpage`
--

CREATE TABLE `logpage` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logpc`
--

CREATE TABLE `logpc` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logshouts`
--

CREATE TABLE `logshouts` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logtext`
--

CREATE TABLE `logtext` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `Level` varchar(50) CHARACTER SET latin1 DEFAULT NULL,
  `LogInfo` varchar(4096) CHARACTER SET latin1 DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `logworld`
--

CREATE TABLE `logworld` (
  `ID` int(11) NOT NULL,
  `TimeStamp` varchar(50) DEFAULT NULL,
  `Level` varchar(50) DEFAULT NULL,
  `LogInfo` varchar(4096) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `onlineusers`
--

CREATE TABLE `onlineusers` (
  `MachineName` varchar(20) DEFAULT NULL,
  `AccountName` varchar(50) DEFAULT NULL,
  `PlayerName` varchar(50) DEFAULT NULL,
  `IPaddr` varchar(16) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `playeritems`
--

CREATE TABLE `playeritems` (
  `ID` bigint(20) NOT NULL,
  `ObjID` int(11) DEFAULT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `EquipPos` int(11) DEFAULT NULL,
  `ObjType` varchar(50) DEFAULT NULL,
  `Qty` int(11) DEFAULT NULL,
  `MadeBy` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `playerprofession`
--

CREATE TABLE `playerprofession` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `FormuleID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `playerskills`
--

CREATE TABLE `playerskills` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `SkillID` int(11) DEFAULT NULL,
  `SkillPnts` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `playerspells`
--

CREATE TABLE `playerspells` (
  `ID` bigint(20) NOT NULL,
  `OwnerID` int(11) DEFAULT NULL,
  `SpellID` int(11) DEFAULT NULL,
  `SpellPnts` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure de la table `playingcharacters`
--

CREATE TABLE `playingcharacters` (
  `UserID` int(11) NOT NULL,
  `PlayerName` varchar(50) DEFAULT NULL,
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

-- --------------------------------------------------------

--
-- Structure de la table `userflags`
--

CREATE TABLE `userflags` (
  `ID` bigint(20) NOT NULL,
  `AccountName` varchar(50) DEFAULT NULL,
  `FlagBitPosition` int(11) DEFAULT NULL,
  `FlagExtraValue` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Index pour les tables déchargées
--

--
-- Index pour la table `auctiongive`
--
ALTER TABLE `auctiongive`
  ADD PRIMARY KEY (`IndexID`);

--
-- Index pour la table `auctionsold`
--
ALTER TABLE `auctionsold`
  ADD PRIMARY KEY (`IndexID`);

--
-- Index pour la table `boosts`
--
ALTER TABLE `boosts`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `BaseOwnerID` (`BaseOwnerID`);

--
-- Index pour la table `chestguild`
--
ALTER TABLE `chestguild`
  ADD PRIMARY KEY (`N°`),
  ADD KEY `GuildName` (`GuildName`);

--
-- Index pour la table `chestitems`
--
ALTER TABLE `chestitems`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);

--
-- Index pour la table `effects`
--
ALTER TABLE `effects`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `BaseOwnerID` (`BaseOwnerID`);

--
-- Index pour la table `flags`
--
ALTER TABLE `flags`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Owner` (`OwnerID`,`BaseOwnerID`);

--
-- Index pour la table `gmmsg`
--
ALTER TABLE `gmmsg`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `guildlogs`
--
ALTER TABLE `guildlogs`
  ADD PRIMARY KEY (`LogID`),
  ADD KEY `GuildName` (`GuildName`);

--
-- Index pour la table `guildmaster`
--
ALTER TABLE `guildmaster`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `guildusers`
--
ALTER TABLE `guildusers`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logdeath`
--
ALTER TABLE `logdeath`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logdeath2`
--
ALTER TABLE `logdeath2`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `loggameop`
--
ALTER TABLE `loggameop`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logitems`
--
ALTER TABLE `logitems`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `lognpcs`
--
ALTER TABLE `lognpcs`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logpage`
--
ALTER TABLE `logpage`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logpc`
--
ALTER TABLE `logpc`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logshouts`
--
ALTER TABLE `logshouts`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logtext`
--
ALTER TABLE `logtext`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `logworld`
--
ALTER TABLE `logworld`
  ADD PRIMARY KEY (`ID`);

--
-- Index pour la table `onlineusers`
--
ALTER TABLE `onlineusers`
  ADD UNIQUE KEY `AccountName` (`AccountName`);

--
-- Index pour la table `playeritems`
--
ALTER TABLE `playeritems`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);

--
-- Index pour la table `playerprofession`
--
ALTER TABLE `playerprofession`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);

--
-- Index pour la table `playerskills`
--
ALTER TABLE `playerskills`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);

--
-- Index pour la table `playerspells`
--
ALTER TABLE `playerspells`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OwnerID` (`OwnerID`);

--
-- Index pour la table `playingcharacters`
--
ALTER TABLE `playingcharacters`
  ADD PRIMARY KEY (`UserID`),
  ADD UNIQUE KEY `PlayerName` (`PlayerName`),
  ADD KEY `AccountName` (`AccountName`);

--
-- Index pour la table `userflags`
--
ALTER TABLE `userflags`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `AccountName` (`AccountName`);

--
-- AUTO_INCREMENT pour les tables déchargées
--

--
-- AUTO_INCREMENT pour la table `auctiongive`
--
ALTER TABLE `auctiongive`
  MODIFY `IndexID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `auctionsold`
--
ALTER TABLE `auctionsold`
  MODIFY `IndexID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `boosts`
--
ALTER TABLE `boosts`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `chestguild`
--
ALTER TABLE `chestguild`
  MODIFY `N°` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `chestitems`
--
ALTER TABLE `chestitems`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `effects`
--
ALTER TABLE `effects`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `flags`
--
ALTER TABLE `flags`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `gmmsg`
--
ALTER TABLE `gmmsg`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `guildlogs`
--
ALTER TABLE `guildlogs`
  MODIFY `LogID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `guildmaster`
--
ALTER TABLE `guildmaster`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `guildusers`
--
ALTER TABLE `guildusers`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `logdeath`
--
ALTER TABLE `logdeath`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `logdeath2`
--
ALTER TABLE `logdeath2`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `loggameop`
--
ALTER TABLE `loggameop`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `lognpcs`
--
ALTER TABLE `lognpcs`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `logpc`
--
ALTER TABLE `logpc`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `logshouts`
--
ALTER TABLE `logshouts`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `logtext`
--
ALTER TABLE `logtext`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `logworld`
--
ALTER TABLE `logworld`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `playeritems`
--
ALTER TABLE `playeritems`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `playerprofession`
--
ALTER TABLE `playerprofession`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `playerskills`
--
ALTER TABLE `playerskills`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `playerspells`
--
ALTER TABLE `playerspells`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `playingcharacters`
--
ALTER TABLE `playingcharacters`
  MODIFY `UserID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT pour la table `userflags`
--
ALTER TABLE `userflags`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
