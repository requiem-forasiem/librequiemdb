-- you can safely ignore error if the following action fails:
ALTER TABLE Requiem_Checksum DROP CONSTRAINT requiem_checksum_algorithm_check;

BEGIN;
UPDATE _format SET version='14.6';

ALTER TABLE Requiem_Alertident ALTER COLUMN _index TYPE INT4;
ALTER TABLE Requiem_Service ALTER COLUMN _parent0_index TYPE INT2;
ALTER TABLE Requiem_Service ALTER COLUMN ip_version TYPE INT2;
ALTER TABLE Requiem_Service ALTER COLUMN iana_protocol_number TYPE INT2;
ALTER TABLE Requiem_Service ALTER COLUMN port TYPE INT4;


ALTER TABLE Requiem_Checksum ADD CHECK ( algorithm IN ('MD4', 'MD5', 'SHA1', 'SHA2-256', 'SHA2-384', 'SHA2-512', 'CRC-32', 'Haval', 'Tiger', 'Gost'));

ALTER TABLE Requiem_Alertident ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_ToolAlert ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_CorrelationAlert ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_OverflowAlert ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Analyzer ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Classification ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Reference ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Source ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Target ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_File ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_FileAccess ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_FileAccess_Permission ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Linkage ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Inode ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Checksum ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Impact ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Action ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Confidence ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Assessment ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_AdditionalData ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_CreateTime ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_DetectTime ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_AnalyzerTime ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Node ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Address ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_User ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_UserId ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Process ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_ProcessArg ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_ProcessEnv ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_Service ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_WebService ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_WebServiceArg ALTER COLUMN _message_ident TYPE INT8;
ALTER TABLE Requiem_SnmpService ALTER COLUMN _message_ident TYPE INT8;

COMMIT;
