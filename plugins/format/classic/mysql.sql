DROP TABLE IF EXISTS _format;

CREATE TABLE _format (
 name VARCHAR(255) NOT NULL,
 version VARCHAR(255) NOT NULL
);
INSERT INTO _format (name, version) VALUES('classic', '14.7');

DROP TABLE IF EXISTS Requiem_Alert;

CREATE TABLE Requiem_Alert (
 _ident BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
 messageid VARCHAR(255) NULL
) TYPE=InnoDB;

CREATE INDEX requiem_alert_messageid ON Requiem_Alert (messageid);


DROP TABLE IF EXISTS Requiem_Alertident;

CREATE TABLE Requiem_Alertident (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _index INTEGER NOT NULL,
 _parent_type ENUM('T','C') NOT NULL, # T=ToolAlert C=CorrelationAlert
 alertident VARCHAR(255) NOT NULL,
 analyzerid VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_ToolAlert;

CREATE TABLE Requiem_ToolAlert (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY,
 name VARCHAR(255) NOT NULL,
 command VARCHAR(255) NULL
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_CorrelationAlert;

CREATE TABLE Requiem_CorrelationAlert (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY,
 name VARCHAR(255) NOT NULL
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_OverflowAlert;

CREATE TABLE Requiem_OverflowAlert (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY,
 program VARCHAR(255) NOT NULL,
 size INTEGER UNSIGNED NULL,
 buffer BLOB NULL
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Heartbeat;

CREATE TABLE Requiem_Heartbeat (
 _ident BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
 messageid VARCHAR(255) NULL,
 heartbeat_interval INTEGER NULL
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Analyzer;

CREATE TABLE Requiem_Analyzer (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H') NOT NULL, # A=Alert H=Hearbeat
 _index TINYINT NOT NULL,
 analyzerid VARCHAR(255) NULL,
 name VARCHAR(255) NULL,
 manufacturer VARCHAR(255) NULL,
 model VARCHAR(255) NULL,
 version VARCHAR(255) NULL,
 class VARCHAR(255) NULL,
 ostype VARCHAR(255) NULL,
 osversion VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type,_message_ident,_index)
) TYPE=InnoDB;

CREATE INDEX requiem_analyzer_analyzerid ON Requiem_Analyzer (_parent_type,_index,analyzerid);
CREATE INDEX requiem_analyzer_index_model ON Requiem_Analyzer (_parent_type,_index,model);



DROP TABLE IF EXISTS Requiem_Classification;

CREATE TABLE Requiem_Classification (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY,
 ident VARCHAR(255) NULL,
 text VARCHAR(255) NOT NULL
) TYPE=InnoDB;

CREATE INDEX requiem_classification_index_text ON Requiem_Classification (text(40));



DROP TABLE IF EXISTS Requiem_Reference;

CREATE TABLE Requiem_Reference (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _index TINYINT NOT NULL,
 origin ENUM("unknown","vendor-specific","user-specific","bugtraqid","cve","osvdb") NOT NULL,
 name VARCHAR(255) NOT NULL,
 url VARCHAR(255) NOT NULL,
 meaning VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _index)
) TYPE=InnoDB;

CREATE INDEX requiem_reference_index_name ON Requiem_Reference (name(40));



DROP TABLE IF EXISTS Requiem_Source;

CREATE TABLE Requiem_Source (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _index SMALLINT NOT NULL,
 ident VARCHAR(255) NULL,
 spoofed ENUM("unknown","yes","no") NOT NULL,
 interface VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Target;

CREATE TABLE Requiem_Target (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _index SMALLINT NOT NULL,
 ident VARCHAR(255) NULL,
 decoy ENUM("unknown","yes","no") NOT NULL,
 interface VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_File;

CREATE TABLE Requiem_File (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent0_index SMALLINT NOT NULL,
 _index TINYINT NOT NULL,
 ident VARCHAR(255) NULL,
 path VARCHAR(255) NOT NULL,
 name VARCHAR(255) NOT NULL,
 category ENUM("current", "original") NULL,
 create_time DATETIME NULL,
 create_time_gmtoff INTEGER NULL,
 modify_time DATETIME NULL,
 modify_time_gmtoff INTEGER NULL,
 access_time DATETIME NULL,
 access_time_gmtoff INTEGER NULL,
 data_size INT UNSIGNED NULL,
 disk_size INT UNSIGNED NULL,
 fstype ENUM("ufs", "efs", "nfs", "afs", "ntfs", "fat16", "fat32", "pcfs", "joliet", "iso9660") NULL,
 file_type VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_FileAccess;

CREATE TABLE Requiem_FileAccess (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent0_index SMALLINT NOT NULL,
 _parent1_index TINYINT NOT NULL,
 _index TINYINT NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_FileAccess_Permission;

CREATE TABLE Requiem_FileAccess_Permission (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent0_index SMALLINT NOT NULL,
 _parent1_index TINYINT NOT NULL,
 _parent2_index TINYINT NOT NULL,
 _index TINYINT NOT NULL,
 permission VARCHAR(255) NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _parent2_index, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Linkage;

CREATE TABLE Requiem_Linkage (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent0_index SMALLINT NOT NULL,
 _parent1_index TINYINT NOT NULL,
 _index TINYINT NOT NULL,
 category ENUM("hard-link","mount-point","reparse-point","shortcut","stream","symbolic-link") NOT NULL,
 name VARCHAR(255) NOT NULL,
 path VARCHAR(255) NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Inode;

CREATE TABLE Requiem_Inode (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent0_index SMALLINT NOT NULL,
 _parent1_index TINYINT NOT NULL,
 change_time DATETIME NULL,
 change_time_gmtoff INTEGER NULL, 
 number INT UNSIGNED NULL,
 major_device INT UNSIGNED NULL,
 minor_device INT UNSIGNED NULL,
 c_major_device INT UNSIGNED NULL,
 c_minor_device INT UNSIGNED NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Checksum;

CREATE TABLE Requiem_Checksum (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent0_index SMALLINT NOT NULL,
 _parent1_index TINYINT NOT NULL,
 _index TINYINT NOT NULL,
 algorithm ENUM("MD4", "MD5", "SHA1", "SHA2-256", "SHA2-384", "SHA2-512", "CRC-32", "Haval", "Tiger", "Gost") NOT NULL,
 value VARCHAR(255) NOT NULL,
 checksum_key VARCHAR(255) NULL, # key is a reserved word
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) TYPE=InnoDB;


DROP TABLE IF EXISTS Requiem_Impact;

CREATE TABLE Requiem_Impact (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY,
 description TEXT NULL,
 severity ENUM("info", "low","medium","high") NULL,
 completion ENUM("failed", "succeeded") NULL,
 type ENUM("admin", "dos", "file", "recon", "user", "other") NOT NULL
) TYPE=InnoDB;

CREATE INDEX requiem_impact_index_severity ON Requiem_Impact (severity);
CREATE INDEX requiem_impact_index_completion ON Requiem_Impact (completion);
CREATE INDEX requiem_impact_index_type ON Requiem_Impact (type);



DROP TABLE IF EXISTS Requiem_Action;

CREATE TABLE Requiem_Action (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _index TINYINT NOT NULL,
 description VARCHAR(255) NULL,
 category ENUM("block-installed", "notification-sent", "taken-offline", "other") NOT NULL,
 PRIMARY KEY (_message_ident, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Confidence;

CREATE TABLE Requiem_Confidence (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY,
 confidence FLOAT NULL,
 rating ENUM("low", "medium", "high", "numeric") NOT NULL
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Assessment;

CREATE TABLE Requiem_Assessment (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_AdditionalData;

CREATE TABLE Requiem_AdditionalData (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A', 'H') NOT NULL,
 _index TINYINT NOT NULL,
 type ENUM("boolean","byte","character","date-time","integer","ntpstamp","portlist","real","string","byte-string","xml") NOT NULL,
 meaning VARCHAR(255) NULL,
 data BLOB NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_CreateTime;

CREATE TABLE Requiem_CreateTime (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H') NOT NULL, # A=Alert H=Hearbeat
 time DATETIME NOT NULL,
 usec INTEGER UNSIGNED NOT NULL,
 gmtoff INTEGER NOT NULL,
 PRIMARY KEY (_parent_type,_message_ident)
) TYPE=InnoDB;

CREATE INDEX requiem_createtime_index ON Requiem_CreateTime (_parent_type,time);


DROP TABLE IF EXISTS Requiem_DetectTime;

CREATE TABLE Requiem_DetectTime (
 _message_ident BIGINT UNSIGNED NOT NULL PRIMARY KEY,
 time DATETIME NOT NULL,
 usec INTEGER UNSIGNED NOT NULL,
 gmtoff INTEGER NOT NULL
) TYPE=InnoDB;

CREATE INDEX requiem_detecttime_index ON Requiem_DetectTime (time);


DROP TABLE IF EXISTS Requiem_AnalyzerTime;

CREATE TABLE Requiem_AnalyzerTime (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H') NOT NULL, # A=Alert H=Hearbeat
 time DATETIME NOT NULL,
 usec INTEGER UNSIGNED NOT NULL,
 gmtoff INTEGER NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident)
) TYPE=InnoDB;

CREATE INDEX requiem_analyzertime_index ON Requiem_AnalyzerTime (_parent_type,time);



DROP TABLE IF EXISTS Requiem_Node;

CREATE TABLE Requiem_Node (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H','S','T') NOT NULL, # A=Analyzer T=Target S=Source H=Heartbeat
 _parent0_index SMALLINT NOT NULL,
 ident VARCHAR(255) NULL,
 category ENUM("unknown","ads","afs","coda","dfs","dns","hosts","kerberos","nds","nis","nisplus","nt","wfw") NULL,
 location VARCHAR(255) NULL,
 name VARCHAR(255) NULL,
 PRIMARY KEY(_parent_type, _message_ident, _parent0_index)
) TYPE=InnoDB;

CREATE INDEX requiem_node_index_location ON Requiem_Node (_parent_type,_parent0_index,location(20));
CREATE INDEX requiem_node_index_name ON Requiem_Node (_parent_type,_parent0_index,name(20));



DROP TABLE IF EXISTS Requiem_Address;

CREATE TABLE Requiem_Address (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H','S','T') NOT NULL, # A=Analyser T=Target S=Source H=Heartbeat
 _parent0_index SMALLINT NOT NULL,
 _index TINYINT NOT NULL,
 ident VARCHAR(255) NULL,
 category ENUM("unknown","atm","e-mail","lotus-notes","mac","sna","vm","ipv4-addr","ipv4-addr-hex","ipv4-net","ipv4-net-mask","ipv6-addr","ipv6-addr-hex","ipv6-net","ipv6-net-mask") NOT NULL,
 vlan_name VARCHAR(255) NULL,
 vlan_num INTEGER UNSIGNED NULL,
 address VARCHAR(255) NOT NULL,
 netmask VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) TYPE=InnoDB;

CREATE INDEX requiem_address_index_address ON Requiem_Address (_parent_type,_parent0_index,_index,address(10));



DROP TABLE IF EXISTS Requiem_User;

CREATE TABLE Requiem_User (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('S','T') NOT NULL, # T=Target S=Source
 _parent0_index SMALLINT NOT NULL,
 ident VARCHAR(255) NULL,
 category ENUM("unknown","application","os-device") NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_UserId;

CREATE TABLE Requiem_UserId (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('S','T', 'F') NOT NULL, # T=Target User S=Source User F=File Access 
 _parent0_index SMALLINT NOT NULL,
 _parent1_index TINYINT NOT NULL,
 _parent2_index TINYINT NOT NULL,
 _index TINYINT NOT NULL,
 ident VARCHAR(255) NULL,
 type ENUM("current-user","original-user","target-user","user-privs","current-group","group-privs","other-privs") NOT NULL,
 name VARCHAR(255) NULL,
 tty VARCHAR(255) NULL,
 number INTEGER UNSIGNED NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _parent1_index, _parent2_index, _index) # _parent_index1 and _parent2_index will always be zero if parent_type = 'F'
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Process;

CREATE TABLE Requiem_Process (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H','S','T') NOT NULL, # A=Analyzer T=Target S=Source H=Heartbeat
 _parent0_index SMALLINT NOT NULL,
 ident VARCHAR(255) NULL,
 name VARCHAR(255) NOT NULL,
 pid INTEGER UNSIGNED NULL,
 path VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_ProcessArg;

CREATE TABLE Requiem_ProcessArg (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H','S','T') NOT NULL DEFAULT 'A', # A=Analyser T=Target S=Source
 _parent0_index SMALLINT NOT NULL,
 _index TINYINT NOT NULL,
 arg VARCHAR(255) NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_ProcessEnv;

CREATE TABLE Requiem_ProcessEnv (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('A','H','S','T') NOT NULL, # A=Analyser T=Target S=Source
 _parent0_index SMALLINT NOT NULL,
 _index TINYINT NOT NULL,
 env VARCHAR(255) NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_Service;

CREATE TABLE Requiem_Service (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('S','T') NOT NULL, # T=Target S=Source
 _parent0_index SMALLINT NOT NULL,
 ident VARCHAR(255) NULL,
 ip_version TINYINT UNSIGNED NULL,
 name VARCHAR(255) NULL,
 port SMALLINT UNSIGNED NULL,
 iana_protocol_number TINYINT UNSIGNED NULL,
 iana_protocol_name VARCHAR(255) NULL,
 portlist VARCHAR (255) NULL,
 protocol VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) TYPE=InnoDB;

CREATE INDEX requiem_service_index_protocol_port ON Requiem_Service (_parent_type,_parent0_index,protocol(10),port);
CREATE INDEX requiem_service_index_protocol_name ON Requiem_Service (_parent_type,_parent0_index,protocol(10),name(10));



DROP TABLE IF EXISTS Requiem_WebService;

CREATE TABLE Requiem_WebService (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('S','T') NOT NULL, # T=Target S=Source
 _parent0_index SMALLINT NOT NULL,
 url VARCHAR(255) NOT NULL,
 cgi VARCHAR(255) NULL,
 http_method VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_WebServiceArg;

CREATE TABLE Requiem_WebServiceArg (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('S','T') NOT NULL, # T=Target S=Source
 _parent0_index SMALLINT NOT NULL,
 _index TINYINT NOT NULL,
 arg VARCHAR(255) NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) TYPE=InnoDB;



DROP TABLE IF EXISTS Requiem_SnmpService;

CREATE TABLE Requiem_SnmpService (
 _message_ident BIGINT UNSIGNED NOT NULL,
 _parent_type ENUM('S','T') NOT NULL, # T=Target S=Source
 _parent0_index SMALLINT NOT NULL,
 snmp_oid VARCHAR(255) NULL, # oid is a reserved word in PostgreSQL 
 message_processing_model INTEGER UNSIGNED NULL,
 security_model INTEGER UNSIGNED NULL,
 security_name VARCHAR(255) NULL,
 security_level INTEGER UNSIGNED NULL,
 context_name VARCHAR(255) NULL,
 context_engine_id VARCHAR(255) NULL,
 command VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) TYPE=InnoDB;
