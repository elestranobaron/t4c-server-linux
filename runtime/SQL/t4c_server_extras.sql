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
