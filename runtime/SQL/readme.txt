SQL script to create SQL structure for T4C Server 1.72.
All tables should use MYISAM Engine, except the logs ones and account table, which can be either MYISAM OR INNODB. I suggest using UTF8 character set for portability, but it's up to you and your environment, pay attention to your website, it could send data in Latin-1 if you don't ask it to use UTF8, on a windows server.
After some tests, it appears that INNODB on other tables produce SQL errors on the server, i don't know why. In case of not saving, turn table in Myisam engine.
Maybe it's the ODBC connector, or just T4C Server who can't handle it properly.

For ODBC connector, use whatever you need with your database manager. For Mysql use MySQL 32 bit connector, for MariaDB you can either use MariaDB connector or MySQL, but prefer the one according to your database in case of trouble.
On the ODBC connector option, MAKE SURE that you tick the AUTO RECONNECT feature, especially if your database is not local (like two separate VPS). Other option shouldn't be needed.
Don't forget to force UTF8 conversion (or whatever you set up on your database) if needed. You'll rapidly see the problem otherwise, because accent characters wont work well (for anything out of english users)!