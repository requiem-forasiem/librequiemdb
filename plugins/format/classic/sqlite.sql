
CREATE TABLE _format (
 name TEXT NOT NULL,
 version TEXT NOT NULL
);
INSERT INTO _format (name, version) VALUES('classic', '14.7');


CREATE TABLE Requiem_Alert (
 _ident INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
 messageid TEXT NULL
) ;

CREATE INDEX requiem_alert_messageid ON Requiem_Alert (messageid);



CREATE TABLE Requiem_Alertident (
 _message_ident INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 alertident TEXT NOT NULL,
 analyzerid TEXT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _index)
) ;




CREATE TABLE Requiem_ToolAlert (
 _message_ident INTEGER NOT NULL PRIMARY KEY,
 name TEXT NOT NULL,
 command TEXT NULL
) ;




CREATE TABLE Requiem_CorrelationAlert (
 _message_ident INTEGER NOT NULL PRIMARY KEY,
 name TEXT NOT NULL
) ;




CREATE TABLE Requiem_OverflowAlert (
 _message_ident INTEGER NOT NULL PRIMARY KEY,
 program TEXT NOT NULL,
 size INTEGER NULL,
 buffer BLOB NULL
) ;




CREATE TABLE Requiem_Heartbeat (
 _ident INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
 messageid TEXT NULL,
 heartbeat_interval INTEGER NULL
) ;




CREATE TABLE Requiem_Analyzer (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _index INTEGER NOT NULL,
 analyzerid TEXT NULL,
 name TEXT NULL,
 manufacturer TEXT NULL,
 model TEXT NULL,
 version TEXT NULL,
 class TEXT NULL,
 ostype TEXT NULL,
 osversion TEXT NULL,
 PRIMARY KEY (_parent_type,_message_ident,_index)
) ;

CREATE INDEX requiem_analyzer_analyzerid ON Requiem_Analyzer (_parent_type,_index,analyzerid);
CREATE INDEX requiem_analyzer_index_model ON Requiem_Analyzer (_parent_type,_index,model);




CREATE TABLE Requiem_Classification (
 _message_ident INTEGER NOT NULL PRIMARY KEY,
 ident TEXT NULL,
 text TEXT NOT NULL
) ;

CREATE INDEX requiem_classification_index_text ON Requiem_Classification (text);




CREATE TABLE Requiem_Reference (
 _message_ident INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 origin TEXT NOT NULL,
 name TEXT NOT NULL,
 url TEXT NOT NULL,
 meaning TEXT NULL,
 PRIMARY KEY (_message_ident, _index)
) ;

CREATE INDEX requiem_reference_index_name ON Requiem_Reference (name);




CREATE TABLE Requiem_Source (
 _message_ident INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 ident TEXT NULL,
 spoofed TEXT NOT NULL,
 interface TEXT NULL,
 PRIMARY KEY (_message_ident, _index)
) ;




CREATE TABLE Requiem_Target (
 _message_ident INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 ident TEXT NULL,
 decoy TEXT NOT NULL,
 interface TEXT NULL,
 PRIMARY KEY (_message_ident, _index)
) ;




CREATE TABLE Requiem_File (
 _message_ident INTEGER NOT NULL,
 _parent0_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 ident TEXT NULL,
 path TEXT NOT NULL,
 name TEXT NOT NULL,
 category TEXT NULL,
 create_time DATETIME NULL,
 create_time_gmtoff INTEGER NULL,
 modify_time DATETIME NULL,
 modify_time_gmtoff INTEGER NULL,
 access_time DATETIME NULL,
 access_time_gmtoff INTEGER NULL,
 data_size INTEGER NULL,
 disk_size INTEGER NULL,
 fstype TEXT NULL,
 file_type TEXT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _index)
) ;




CREATE TABLE Requiem_FileAccess (
 _message_ident INTEGER NOT NULL,
 _parent0_index INTEGER NOT NULL,
 _parent1_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) ;




CREATE TABLE Requiem_FileAccess_Permission (
 _message_ident INTEGER NOT NULL,
 _parent0_index INTEGER NOT NULL,
 _parent1_index INTEGER NOT NULL,
 _parent2_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 permission TEXT NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _parent2_index, _index)
) ;




CREATE TABLE Requiem_Linkage (
 _message_ident INTEGER NOT NULL,
 _parent0_index INTEGER NOT NULL,
 _parent1_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 category TEXT NOT NULL,
 name TEXT NOT NULL,
 path TEXT NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) ;




CREATE TABLE Requiem_Inode (
 _message_ident INTEGER NOT NULL,
 _parent0_index INTEGER NOT NULL,
 _parent1_index INTEGER NOT NULL,
 change_time DATETIME NULL,
 change_time_gmtoff INTEGER NULL, 
 number INTEGER NULL,
 major_device INTEGER NULL,
 minor_device INTEGER NULL,
 c_major_device INTEGER NULL,
 c_minor_device INTEGER NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index)
) ;




CREATE TABLE Requiem_Checksum (
 _message_ident INTEGER NOT NULL,
 _parent0_index INTEGER NOT NULL,
 _parent1_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 algorithm TEXT NOT NULL,
 value TEXT NOT NULL,
 checksum_key TEXT NULL, 
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) ;



CREATE TABLE Requiem_Impact (
 _message_ident INTEGER NOT NULL PRIMARY KEY,
 description TEXT NULL,
 severity TEXT NULL,
 completion TEXT NULL,
 type TEXT NOT NULL
) ;

CREATE INDEX requiem_impact_index_severity ON Requiem_Impact (severity);
CREATE INDEX requiem_impact_index_completion ON Requiem_Impact (completion);
CREATE INDEX requiem_impact_index_type ON Requiem_Impact (type);




CREATE TABLE Requiem_Action (
 _message_ident INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 description TEXT NULL,
 category TEXT NOT NULL,
 PRIMARY KEY (_message_ident, _index)
) ;




CREATE TABLE Requiem_Confidence (
 _message_ident INTEGER NOT NULL PRIMARY KEY,
 confidence FLOAT NULL,
 rating TEXT NOT NULL
) ;




CREATE TABLE Requiem_Assessment (
 _message_ident INTEGER NOT NULL PRIMARY KEY
) ;




CREATE TABLE Requiem_AdditionalData (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL,
 _index INTEGER NOT NULL,
 type TEXT NOT NULL,
 meaning TEXT NULL,
 data BLOB NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _index)
) ;




CREATE TABLE Requiem_CreateTime (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 time DATETIME NOT NULL,
 usec INTEGER NOT NULL,
 gmtoff INTEGER NOT NULL,
 PRIMARY KEY (_parent_type,_message_ident)
) ;

CREATE INDEX requiem_createtime_index ON Requiem_CreateTime (_parent_type,time);



CREATE TABLE Requiem_DetectTime (
 _message_ident INTEGER NOT NULL PRIMARY KEY,
 time DATETIME NOT NULL,
 usec INTEGER NOT NULL,
 gmtoff INTEGER NOT NULL
) ;

CREATE INDEX requiem_detecttime_index ON Requiem_DetectTime (time);



CREATE TABLE Requiem_AnalyzerTime (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 time DATETIME NOT NULL,
 usec INTEGER NOT NULL,
 gmtoff INTEGER NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident)
) ;

CREATE INDEX requiem_analyzertime_index ON Requiem_AnalyzerTime (_parent_type,time);




CREATE TABLE Requiem_Node (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 ident TEXT NULL,
 category TEXT NULL,
 location TEXT NULL,
 name TEXT NULL,
 PRIMARY KEY(_parent_type, _message_ident, _parent0_index)
) ;

CREATE INDEX requiem_node_index_location ON Requiem_Node (_parent_type,_parent0_index,location);
CREATE INDEX requiem_node_index_name ON Requiem_Node (_parent_type,_parent0_index,name);




CREATE TABLE Requiem_Address (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 ident TEXT NULL,
 category TEXT NOT NULL,
 vlan_name TEXT NULL,
 vlan_num INTEGER NULL,
 address TEXT NOT NULL,
 netmask TEXT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;

CREATE INDEX requiem_address_index_address ON Requiem_Address (_parent_type,_parent0_index,_index,address);




CREATE TABLE Requiem_User (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 ident TEXT NULL,
 category TEXT NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;




CREATE TABLE Requiem_UserId (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 _parent1_index INTEGER NOT NULL,
 _parent2_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 ident TEXT NULL,
 type TEXT NOT NULL,
 name TEXT NULL,
 tty TEXT NULL,
 number INTEGER NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _parent1_index, _parent2_index, _index) 
) ;




CREATE TABLE Requiem_Process (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 ident TEXT NULL,
 name TEXT NOT NULL,
 pid INTEGER NULL,
 path TEXT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;




CREATE TABLE Requiem_ProcessArg (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL DEFAULT 'A', 
 _parent0_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 arg TEXT NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;




CREATE TABLE Requiem_ProcessEnv (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 env TEXT NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;




CREATE TABLE Requiem_Service (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 ident TEXT NULL,
 ip_version INTEGER NULL,
 name TEXT NULL,
 port INTEGER NULL,
 iana_protocol_number INTEGER NULL,
 iana_protocol_name TEXT NULL,
 portlist VARCHAR  NULL,
 protocol TEXT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;

CREATE INDEX requiem_service_index_protocol_port ON Requiem_Service (_parent_type,_parent0_index,protocol,port);
CREATE INDEX requiem_service_index_protocol_name ON Requiem_Service (_parent_type,_parent0_index,protocol,name);




CREATE TABLE Requiem_WebService (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 url TEXT NOT NULL,
 cgi TEXT NULL,
 http_method TEXT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;




CREATE TABLE Requiem_WebServiceArg (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 _index INTEGER NOT NULL,
 arg TEXT NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;




CREATE TABLE Requiem_SnmpService (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL, 
 _parent0_index INTEGER NOT NULL,
 snmp_oid TEXT NULL, 
 message_processing_model INTEGER NULL,
 security_model INTEGER NULL,
 security_name TEXT NULL,
 security_level INTEGER NULL,
 context_name TEXT NULL,
 context_engine_id TEXT NULL,
 command TEXT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;
