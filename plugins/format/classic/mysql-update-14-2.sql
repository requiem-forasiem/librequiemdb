ALTER TABLE Requiem_Impact CHANGE severity severity ENUM("info", "low", "medium", "high") NULL;
UPDATE _format SET version="14.2";

ALTER TABLE Requiem_Address DROP INDEX _parent_type;
CREATE INDEX requiem_address_index_address ON Requiem_Address (_parent_type,_parent0_index,_index,address(10)); 

ALTER TABLE Requiem_Analyzer DROP INDEX _parent_type;
ALTER TABLE Requiem_Analyzer DROP INDEX _parent_type_2;
CREATE INDEX requiem_analyzer_index_analyzerid ON Requiem_Analyzer (_parent_type, _index, analyzerid); 
CREATE INDEX requiem_analyzer_index_model ON Requiem_Analyzer (_parent_type, _index, model); 

ALTER TABLE Requiem_Classification DROP INDEX text;
CREATE INDEX requiem_classification_index_text ON Requiem_Classification (text(40));  

ALTER TABLE Requiem_File ADD COLUMN file_type VARCHAR(255) NULL;

ALTER TABLE Requiem_Node DROP INDEX _parent_type;
ALTER TABLE Requiem_Node DROP INDEX _parent_type_2;
CREATE INDEX requiem_node_index_location ON Requiem_Node (_parent_type,_parent0_index,location(20));
CREATE INDEX requiem_node_index_name ON Requiem_Node (_parent_type,_parent0_index,name(20)); 

ALTER TABLE Requiem_Reference DROP INDEX name;
CREATE INDEX requiem_reference_index_name ON Requiem_Reference (name(40)); 


ALTER TABLE Requiem_Service  DROP INDEX requiem_service_index_protocol_port;
ALTER TABLE Requiem_Service  DROP INDEX requiem_service_index_protocol_name;
CREATE INDEX requiem_service_index_protocol_port ON Requiem_Service (_parent_type,_parent0_index,protocol(10),port);
CREATE INDEX requiem_service_index_protocol_name ON Requiem_Service (_parent_type,_parent0_index,protocol(10),name(10));  
