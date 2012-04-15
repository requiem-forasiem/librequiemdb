BEGIN;

UPDATE _format SET version="14.6";

ALTER TABLE Requiem_Alertident CHANGE _index _index INTEGER NOT NULL;
ALTER TABLE Requiem_Source CHANGE _index _index SMALLINT NOT NULL;
ALTER TABLE Requiem_Target CHANGE _index _index SMALLINT NOT NULL;
ALTER TABLE Requiem_File CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_FileAccess CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_FileAccess_Permission CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_Linkage CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_Inode CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_Checksum CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_Node CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_Address CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_User CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_UserId CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_Process CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_ProcessArg CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_ProcessEnv CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_Service CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_WebService CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_WebServiceArg CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;
ALTER TABLE Requiem_SnmpService CHANGE _parent0_index _parent0_index SMALLINT NOT NULL;


ALTER TABLE Requiem_Checksum CHANGE algorithm algorithm ENUM("MD4", "MD5", "SHA1", "SHA2-256", "SHA2-384", "SHA2-512", "CRC-32", "Haval", "Tiger", "Gost") NOT NULL;

COMMIT;
