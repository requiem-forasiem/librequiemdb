DROP TABLE _format;

CREATE TABLE _format (
 name VARCHAR(255) NOT NULL,
 version VARCHAR(255) NOT NULL
);
INSERT INTO _format (name, version) VALUES('classic', '14.7');

DROP TABLE Requiem_Alert;

CREATE TABLE Requiem_Alert (
 _ident BIGSERIAL PRIMARY KEY,
 messageid VARCHAR(255) NULL
) ;

CREATE INDEX requiem_alert_messageid ON Requiem_Alert (messageid);


DROP TABLE Requiem_Alertident;

CREATE TABLE Requiem_Alertident (
 _message_ident INT8 NOT NULL,
 _index INT4 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('T','C')) NOT NULL, 
 alertident VARCHAR(255) NOT NULL,
 analyzerid VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _index)
) ;



DROP TABLE Requiem_ToolAlert;

CREATE TABLE Requiem_ToolAlert (
 _message_ident INT8 NOT NULL PRIMARY KEY,
 name VARCHAR(255) NOT NULL,
 command VARCHAR(255) NULL
) ;



DROP TABLE Requiem_CorrelationAlert;

CREATE TABLE Requiem_CorrelationAlert (
 _message_ident INT8 NOT NULL PRIMARY KEY,
 name VARCHAR(255) NOT NULL
) ;



DROP TABLE Requiem_OverflowAlert;

CREATE TABLE Requiem_OverflowAlert (
 _message_ident INT8 NOT NULL PRIMARY KEY,
 program VARCHAR(255) NOT NULL,
 size INT8 NULL,
 buffer BYTEA NULL
) ;



DROP TABLE Requiem_Heartbeat;

CREATE TABLE Requiem_Heartbeat (
 _ident BIGSERIAL PRIMARY KEY,
 messageid VARCHAR(255) NULL,
 heartbeat_interval INT4 NULL
) ;



DROP TABLE Requiem_Analyzer;

CREATE TABLE Requiem_Analyzer (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H')) NOT NULL, 
 _index INT2 NOT NULL,
 analyzerid VARCHAR(255) NULL,
 name VARCHAR(255) NULL,
 manufacturer VARCHAR(255) NULL,
 model VARCHAR(255) NULL,
 version VARCHAR(255) NULL,
 class VARCHAR(255) NULL,
 ostype VARCHAR(255) NULL,
 osversion VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type,_message_ident,_index)
) ;

CREATE INDEX requiem_analyzer_analyzerid ON Requiem_Analyzer (_parent_type,_index,analyzerid);
CREATE INDEX requiem_analyzer_index_model ON Requiem_Analyzer (_parent_type,_index,model);



DROP TABLE Requiem_Classification;

CREATE TABLE Requiem_Classification (
 _message_ident INT8 NOT NULL PRIMARY KEY,
 ident VARCHAR(255) NULL,
 text VARCHAR(255) NOT NULL
) ;

CREATE INDEX requiem_classification_index_text ON Requiem_Classification (text);



DROP TABLE Requiem_Reference;

CREATE TABLE Requiem_Reference (
 _message_ident INT8 NOT NULL,
 _index INT2 NOT NULL,
 origin VARCHAR(32) CHECK ( origin IN ('unknown','vendor-specific','user-specific','bugtraqid','cve','osvdb')) NOT NULL,
 name VARCHAR(255) NOT NULL,
 url VARCHAR(255) NOT NULL,
 meaning VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _index)
) ;

CREATE INDEX requiem_reference_index_name ON Requiem_Reference (name);



DROP TABLE Requiem_Source;

CREATE TABLE Requiem_Source (
 _message_ident INT8 NOT NULL,
 _index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 spoofed VARCHAR(32) CHECK ( spoofed IN ('unknown','yes','no')) NOT NULL,
 interface VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _index)
) ;



DROP TABLE Requiem_Target;

CREATE TABLE Requiem_Target (
 _message_ident INT8 NOT NULL,
 _index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 decoy VARCHAR(32) CHECK ( decoy IN ('unknown','yes','no')) NOT NULL,
 interface VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _index)
) ;



DROP TABLE Requiem_File;

CREATE TABLE Requiem_File (
 _message_ident INT8 NOT NULL,
 _parent0_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 path VARCHAR(255) NOT NULL,
 name VARCHAR(255) NOT NULL,
 category VARCHAR(32) CHECK ( category IN ('current', 'original')) NULL,
 create_time TIMESTAMP NULL,
 create_time_gmtoff INT4 NULL,
 modify_time TIMESTAMP NULL,
 modify_time_gmtoff INT4 NULL,
 access_time TIMESTAMP NULL,
 access_time_gmtoff INT4 NULL,
 data_size INT8 NULL,
 disk_size INT8 NULL,
 fstype VARCHAR(32) CHECK ( fstype IN ('ufs', 'efs', 'nfs', 'afs', 'ntfs', 'fat16', 'fat32', 'pcfs', 'joliet', 'iso9660')) NULL,
 file_type VARCHAR(255) NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _index)
) ;



DROP TABLE Requiem_FileAccess;

CREATE TABLE Requiem_FileAccess (
 _message_ident INT8 NOT NULL,
 _parent0_index INT2 NOT NULL,
 _parent1_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) ;



DROP TABLE Requiem_FileAccess_Permission;

CREATE TABLE Requiem_FileAccess_Permission (
 _message_ident INT8 NOT NULL,
 _parent0_index INT2 NOT NULL,
 _parent1_index INT2 NOT NULL,
 _parent2_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 permission VARCHAR(255) NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _parent2_index, _index)
) ;



DROP TABLE Requiem_Linkage;

CREATE TABLE Requiem_Linkage (
 _message_ident INT8 NOT NULL,
 _parent0_index INT2 NOT NULL,
 _parent1_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 category VARCHAR(32) CHECK ( category IN ('hard-link','mount-point','reparse-point','shortcut','stream','symbolic-link')) NOT NULL,
 name VARCHAR(255) NOT NULL,
 path VARCHAR(255) NOT NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) ;



DROP TABLE Requiem_Inode;

CREATE TABLE Requiem_Inode (
 _message_ident INT8 NOT NULL,
 _parent0_index INT2 NOT NULL,
 _parent1_index INT2 NOT NULL,
 change_time TIMESTAMP NULL,
 change_time_gmtoff INT4 NULL, 
 number INT8 NULL,
 major_device INT8 NULL,
 minor_device INT8 NULL,
 c_major_device INT8 NULL,
 c_minor_device INT8 NULL,
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index)
) ;



DROP TABLE Requiem_Checksum;

CREATE TABLE Requiem_Checksum (
 _message_ident INT8 NOT NULL,
 _parent0_index INT2 NOT NULL,
 _parent1_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 algorithm VARCHAR(32) CHECK ( algorithm IN ('MD4', 'MD5', 'SHA1', 'SHA2-256', 'SHA2-384', 'SHA2-512', 'CRC-32', 'Haval', 'Tiger', 'Gost')) NOT NULL,
 value VARCHAR(255) NOT NULL,
 checksum_key VARCHAR(255) NULL, 
 PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index)
) ;


DROP TABLE Requiem_Impact;

CREATE TABLE Requiem_Impact (
 _message_ident INT8 NOT NULL PRIMARY KEY,
 description TEXT NULL,
 severity VARCHAR(32) CHECK ( severity IN ('info', 'low','medium','high')) NULL,
 completion VARCHAR(32) CHECK ( completion IN ('failed', 'succeeded')) NULL,
 type VARCHAR(32) CHECK ( type IN ('admin', 'dos', 'file', 'recon', 'user', 'other')) NOT NULL
) ;

CREATE INDEX requiem_impact_index_severity ON Requiem_Impact (severity);
CREATE INDEX requiem_impact_index_completion ON Requiem_Impact (completion);
CREATE INDEX requiem_impact_index_type ON Requiem_Impact (type);



DROP TABLE Requiem_Action;

CREATE TABLE Requiem_Action (
 _message_ident INT8 NOT NULL,
 _index INT2 NOT NULL,
 description VARCHAR(255) NULL,
 category VARCHAR(32) CHECK ( category IN ('block-installed', 'notification-sent', 'taken-offline', 'other')) NOT NULL,
 PRIMARY KEY (_message_ident, _index)
) ;



DROP TABLE Requiem_Confidence;

CREATE TABLE Requiem_Confidence (
 _message_ident INT8 NOT NULL PRIMARY KEY,
 confidence FLOAT NULL,
 rating VARCHAR(32) CHECK ( rating IN ('low', 'medium', 'high', 'numeric')) NOT NULL
) ;



DROP TABLE Requiem_Assessment;

CREATE TABLE Requiem_Assessment (
 _message_ident INT8 NOT NULL PRIMARY KEY
) ;



DROP TABLE Requiem_AdditionalData;

CREATE TABLE Requiem_AdditionalData (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A', 'H')) NOT NULL,
 _index INT2 NOT NULL,
 type VARCHAR(32) CHECK ( type IN ('boolean','byte','character','date-time','integer','ntpstamp','portlist','real','string','byte-string','xml')) NOT NULL,
 meaning VARCHAR(255) NULL,
 data BYTEA NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _index)
) ;



DROP TABLE Requiem_CreateTime;

CREATE TABLE Requiem_CreateTime (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H')) NOT NULL, 
 time TIMESTAMP NOT NULL,
 usec INT8 NOT NULL,
 gmtoff INT4 NOT NULL,
 PRIMARY KEY (_parent_type,_message_ident)
) ;

CREATE INDEX requiem_createtime_index ON Requiem_CreateTime (_parent_type,time);


DROP TABLE Requiem_DetectTime;

CREATE TABLE Requiem_DetectTime (
 _message_ident INT8 NOT NULL PRIMARY KEY,
 time TIMESTAMP NOT NULL,
 usec INT8 NOT NULL,
 gmtoff INT4 NOT NULL
) ;

CREATE INDEX requiem_detecttime_index ON Requiem_DetectTime (time);


DROP TABLE Requiem_AnalyzerTime;

CREATE TABLE Requiem_AnalyzerTime (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H')) NOT NULL, 
 time TIMESTAMP NOT NULL,
 usec INT8 NOT NULL,
 gmtoff INT4 NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident)
) ;

CREATE INDEX requiem_analyzertime_index ON Requiem_AnalyzerTime (_parent_type,time);



DROP TABLE Requiem_Node;

CREATE TABLE Requiem_Node (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H','S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 category VARCHAR(32) CHECK ( category IN ('unknown','ads','afs','coda','dfs','dns','hosts','kerberos','nds','nis','nisplus','nt','wfw')) NULL,
 location VARCHAR(255) NULL,
 name VARCHAR(255) NULL,
 PRIMARY KEY(_parent_type, _message_ident, _parent0_index)
) ;

CREATE INDEX requiem_node_index_location ON Requiem_Node (_parent_type,_parent0_index,location);
CREATE INDEX requiem_node_index_name ON Requiem_Node (_parent_type,_parent0_index,name);



DROP TABLE Requiem_Address;

CREATE TABLE Requiem_Address (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H','S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 category VARCHAR(32) CHECK ( category IN ('unknown','atm','e-mail','lotus-notes','mac','sna','vm','ipv4-addr','ipv4-addr-hex','ipv4-net','ipv4-net-mask','ipv6-addr','ipv6-addr-hex','ipv6-net','ipv6-net-mask')) NOT NULL,
 vlan_name VARCHAR(255) NULL,
 vlan_num INT8 NULL,
 address VARCHAR(255) NOT NULL,
 netmask VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;

CREATE INDEX requiem_address_index_address ON Requiem_Address (_parent_type,_parent0_index,_index,address);



DROP TABLE Requiem_User;

CREATE TABLE Requiem_User (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 category VARCHAR(32) CHECK ( category IN ('unknown','application','os-device')) NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;



DROP TABLE Requiem_UserId;

CREATE TABLE Requiem_UserId (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('S','T', 'F')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 _parent1_index INT2 NOT NULL,
 _parent2_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 type VARCHAR(32) CHECK ( type IN ('current-user','original-user','target-user','user-privs','current-group','group-privs','other-privs')) NOT NULL,
 name VARCHAR(255) NULL,
 tty VARCHAR(255) NULL,
 number INT8 NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _parent1_index, _parent2_index, _index) 
) ;



DROP TABLE Requiem_Process;

CREATE TABLE Requiem_Process (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H','S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 name VARCHAR(255) NOT NULL,
 pid INT8 NULL,
 path VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;



DROP TABLE Requiem_ProcessArg;

CREATE TABLE Requiem_ProcessArg (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H','S','T')) NOT NULL DEFAULT 'A', 
 _parent0_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 arg VARCHAR(255) NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;



DROP TABLE Requiem_ProcessEnv;

CREATE TABLE Requiem_ProcessEnv (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('A','H','S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 env VARCHAR(255) NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;



DROP TABLE Requiem_Service;

CREATE TABLE Requiem_Service (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 ident VARCHAR(255) NULL,
 ip_version INT2 NULL,
 name VARCHAR(255) NULL,
 port INT4 NULL,
 iana_protocol_number INT2 NULL,
 iana_protocol_name VARCHAR(255) NULL,
 portlist VARCHAR (255) NULL,
 protocol VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;

CREATE INDEX requiem_service_index_protocol_port ON Requiem_Service (_parent_type,_parent0_index,protocol,port);
CREATE INDEX requiem_service_index_protocol_name ON Requiem_Service (_parent_type,_parent0_index,protocol,name);



DROP TABLE Requiem_WebService;

CREATE TABLE Requiem_WebService (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 url VARCHAR(255) NOT NULL,
 cgi VARCHAR(255) NULL,
 http_method VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;



DROP TABLE Requiem_WebServiceArg;

CREATE TABLE Requiem_WebServiceArg (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 _index INT2 NOT NULL,
 arg VARCHAR(255) NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index, _index)
) ;



DROP TABLE Requiem_SnmpService;

CREATE TABLE Requiem_SnmpService (
 _message_ident INT8 NOT NULL,
 _parent_type VARCHAR(1) CHECK (_parent_type IN ('S','T')) NOT NULL, 
 _parent0_index INT2 NOT NULL,
 snmp_oid VARCHAR(255) NULL, 
 message_processing_model INT8 NULL,
 security_model INT8 NULL,
 security_name VARCHAR(255) NULL,
 security_level INT8 NULL,
 context_name VARCHAR(255) NULL,
 context_engine_id VARCHAR(255) NULL,
 command VARCHAR(255) NULL,
 PRIMARY KEY (_parent_type, _message_ident, _parent0_index)
) ;
