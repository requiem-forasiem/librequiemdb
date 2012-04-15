UPDATE _format SET version='14.2';

ALTER TABLE Requiem_Alertident ADD COLUMN _index INT4;
ALTER TABLE Requiem_Alertident ALTER COLUMN _index SET NOT NULL;
ALTER TABLE Requiem_Alertident ADD PRIMARY KEY (_parent_type, _message_ident, _index);
DROP INDEX requiem_alert_ident;

DROP INDEX requiem_analyzer_index_model;
CREATE INDEX requiem_analyzer_index_model ON Requiem_Analyzer (_parent_type,_index,model);

/* Update the data type of the column _index from Requiem_Analyzer */
ALTER TABLE Requiem_Analyzer ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Analyzer SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Analyzer DROP COLUMN _index;
ALTER TABLE Requiem_Analyzer RENAME temp_useless_column to _index;
/* Requiem_Analyzer->_index  data type updated*/

DROP INDEX requiem_classification_index;
CREATE INDEX requiem_classification_index_text ON Requiem_Classification (text);

ALTER TABLE Requiem_Reference ADD PRIMARY KEY (_message_ident, _index);
DROP INDEX requiem_reference_index_message_ident;

/* Update the data type of the column _index from Requiem_Reference */
ALTER TABLE Requiem_Reference ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Reference SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Reference DROP COLUMN _index;
ALTER TABLE Requiem_Reference RENAME temp_useless_column to _index;
/* Requiem_Reference->_index  data type updated*/

/* Update the data type of the column _index from Requiem_Source */
ALTER TABLE Requiem_Source ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Source SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Source DROP COLUMN _index;
ALTER TABLE Requiem_Source RENAME temp_useless_column to _index;
/* Requiem_Source->_index  data type updated*/

/* Update the data type of the column _index from Requiem_Target */
ALTER TABLE Requiem_Target ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Target SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Target DROP COLUMN _index;
ALTER TABLE Requiem_Target RENAME temp_useless_column to _index;
/* Requiem_Target->_index  data type updated*/

ALTER TABLE Requiem_File ADD COLUMN file_type VARCHAR(255) NULL;
/* Update the data type of the column _parent0_index from Requiem_File */
ALTER TABLE Requiem_File ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_File SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_File DROP COLUMN _parent0_index;
ALTER TABLE Requiem_File RENAME temp_useless_column to _parent0_index;
/* Requiem_File->_parent0_index  data type updated*/
/* Update the data type of the column _index from Requiem_File */
ALTER TABLE Requiem_File ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_File SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_File DROP COLUMN _index;
ALTER TABLE Requiem_File RENAME temp_useless_column to _index;
/* Requiem_File->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_FileAccess */
ALTER TABLE Requiem_FileAccess ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_FileAccess SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_FileAccess DROP COLUMN _parent0_index;
ALTER TABLE Requiem_FileAccess RENAME temp_useless_column to _parent0_index;
/* Requiem_FileAccess->_parent0_index  data type updated*/
/* Update the data type of the column _parent1_index from Requiem_FileAccess */
ALTER TABLE Requiem_FileAccess ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_FileAccess SET temp_useless_column = CAST(_parent1_index AS INT4);
ALTER TABLE Requiem_FileAccess DROP COLUMN _parent1_index;
ALTER TABLE Requiem_FileAccess RENAME temp_useless_column to _parent1_index;
/* Requiem_FileAccess->_parent1_index  data type updated*/
/* Update the data type of the column _index from Requiem_FileAccess */
ALTER TABLE Requiem_FileAccess ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_FileAccess SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_FileAccess DROP COLUMN _index;
ALTER TABLE Requiem_FileAccess RENAME temp_useless_column to _index;
/* Requiem_FileAccess->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_FileAccess_Permission */
ALTER TABLE Requiem_FileAccess_Permission ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_FileAccess_Permission SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_FileAccess_Permission DROP COLUMN _parent0_index;
ALTER TABLE Requiem_FileAccess_Permission RENAME temp_useless_column to _parent0_index;
/* Requiem_FileAccess_Permission->_parent0_index  data type updated*/
/* Update the data type of the column _parent1_index from Requiem_FileAccess_Permission */
ALTER TABLE Requiem_FileAccess_Permission ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_FileAccess_Permission SET temp_useless_column = CAST(_parent1_index AS INT4);
ALTER TABLE Requiem_FileAccess_Permission DROP COLUMN _parent1_index;
ALTER TABLE Requiem_FileAccess_Permission RENAME temp_useless_column to _parent1_index;
/* Requiem_FileAccess_Permission->_parent1_index  data type updated*/
/* Update the data type of the column _parent2_index from Requiem_FileAccess_Permission */
ALTER TABLE Requiem_FileAccess_Permission ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_FileAccess_Permission SET temp_useless_column = CAST(_parent2_index AS INT4);
ALTER TABLE Requiem_FileAccess_Permission DROP COLUMN _parent2_index;
ALTER TABLE Requiem_FileAccess_Permission RENAME temp_useless_column to _parent2_index;
/* Requiem_FileAccess_Permission->_parent2_index  data type updated*/
/* Update the data type of the column _index from Requiem_FileAccess_Permission */
ALTER TABLE Requiem_FileAccess_Permission ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_FileAccess_Permission SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_FileAccess_Permission DROP COLUMN _index;
ALTER TABLE Requiem_FileAccess_Permission RENAME temp_useless_column to _index;
/* Requiem_FileAccess_Permission->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_Linkage */
ALTER TABLE Requiem_Linkage ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Linkage SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_Linkage DROP COLUMN _parent0_index;
ALTER TABLE Requiem_Linkage RENAME temp_useless_column to _parent0_index;
/* Requiem_Linkage->_parent0_index  data type updated*/
/* Update the data type of the column _parent1_index from Requiem_Linkage */
ALTER TABLE Requiem_Linkage ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Linkage SET temp_useless_column = CAST(_parent1_index AS INT4);
ALTER TABLE Requiem_Linkage DROP COLUMN _parent1_index;
ALTER TABLE Requiem_Linkage RENAME temp_useless_column to _parent1_index;
/* Requiem_Linkage->_parent1_index  data type updated*/
/* Update the data type of the column _index from Requiem_Linkage */
ALTER TABLE Requiem_Linkage ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Linkage SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Linkage DROP COLUMN _index;
ALTER TABLE Requiem_Linkage RENAME temp_useless_column to _index;
/* Requiem_Linkage->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_Inode */
ALTER TABLE Requiem_Inode ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Inode SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_Inode DROP COLUMN _parent0_index;
ALTER TABLE Requiem_Inode RENAME temp_useless_column to _parent0_index;
/* Requiem_Inode->_parent0_index  data type updated*/
/* Update the data type of the column _parent1_index from Requiem_Inode */
ALTER TABLE Requiem_Inode ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Inode SET temp_useless_column = CAST(_parent1_index AS INT4);
ALTER TABLE Requiem_Inode DROP COLUMN _parent1_index;
ALTER TABLE Requiem_Inode RENAME temp_useless_column to _parent1_index;
/* Requiem_Inode->_parent1_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_Checksum */
ALTER TABLE Requiem_Checksum ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Checksum SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_Checksum DROP COLUMN _parent0_index;
ALTER TABLE Requiem_Checksum RENAME temp_useless_column to _parent0_index;
/* Requiem_Checksum->_parent0_index  data type updated*/
/* Update the data type of the column _parent1_index from Requiem_Checksum */
ALTER TABLE Requiem_Checksum ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Checksum SET temp_useless_column = CAST(_parent1_index AS INT4);
ALTER TABLE Requiem_Checksum DROP COLUMN _parent1_index;
ALTER TABLE Requiem_Checksum RENAME temp_useless_column to _parent1_index;
/* Requiem_Checksum->_parent1_index  data type updated*/
/* Update the data type of the column _index from Requiem_Checksum */
ALTER TABLE Requiem_Checksum ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Checksum SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Checksum DROP COLUMN _index;
ALTER TABLE Requiem_Checksum RENAME temp_useless_column to _index;
/* Requiem_Checksum->_index  data type updated*/

/* Update the data type of the column _index from Requiem_Action */
ALTER TABLE Requiem_Action ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Action SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Action DROP COLUMN _index;
ALTER TABLE Requiem_Action RENAME temp_useless_column to _index;
/* Requiem_Action->_index  data type updated*/

/* Update the data type of the column _index from Requiem_AdditionalData */
ALTER TABLE Requiem_AdditionalData ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_AdditionalData SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_AdditionalData DROP COLUMN _index;
ALTER TABLE Requiem_AdditionalData RENAME temp_useless_column to _index;
/* Requiem_AdditionalData->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_Node */
ALTER TABLE Requiem_Node ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Node SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_Node DROP COLUMN _parent0_index;
ALTER TABLE Requiem_Node RENAME temp_useless_column to _parent0_index;
/* Requiem_Node->_parent0_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_Address */
ALTER TABLE Requiem_Address ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Address SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_Address DROP COLUMN _parent0_index;
ALTER TABLE Requiem_Address RENAME temp_useless_column to _parent0_index;
/* Requiem_Address->_parent0_index  data type updated*/
/* Update the data type of the column _index from Requiem_Address */
ALTER TABLE Requiem_Address ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Address SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_Address DROP COLUMN _index;
ALTER TABLE Requiem_Address RENAME temp_useless_column to _index;
/* Requiem_Address->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_User */
ALTER TABLE Requiem_User ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_User SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_User DROP COLUMN _parent0_index;
ALTER TABLE Requiem_User RENAME temp_useless_column to _parent0_index;
/* Requiem_User->_parent0_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_UserId */
ALTER TABLE Requiem_UserId ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_UserId SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_UserId DROP COLUMN _parent0_index;
ALTER TABLE Requiem_UserId RENAME temp_useless_column to _parent0_index;
/* Requiem_UserId->_parent0_index  data type updated*/
/* Update the data type of the column _parent1_index from Requiem_UserId */
ALTER TABLE Requiem_UserId ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_UserId SET temp_useless_column = CAST(_parent1_index AS INT4);
ALTER TABLE Requiem_UserId DROP COLUMN _parent1_index;
ALTER TABLE Requiem_UserId RENAME temp_useless_column to _parent1_index;
/* Requiem_UserId->_parent1_index  data type updated*/
/* Update the data type of the column _parent2_index from Requiem_UserId */
ALTER TABLE Requiem_UserId ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_UserId SET temp_useless_column = CAST(_parent2_index AS INT4);
ALTER TABLE Requiem_UserId DROP COLUMN _parent2_index;
ALTER TABLE Requiem_UserId RENAME temp_useless_column to _parent2_index;
/* Requiem_UserId->_parent2_index  data type updated*/
/* Update the data type of the column _index from Requiem_UserId */
ALTER TABLE Requiem_UserId ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_UserId SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_UserId DROP COLUMN _index;
ALTER TABLE Requiem_UserId RENAME temp_useless_column to _index;
/* Requiem_UserId->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_Process */
ALTER TABLE Requiem_Process ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_Process SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_Process DROP COLUMN _parent0_index;
ALTER TABLE Requiem_Process RENAME temp_useless_column to _parent0_index;
/* Requiem_Process->_parent0_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_ProcessArg */
ALTER TABLE Requiem_ProcessArg ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_ProcessArg SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_ProcessArg DROP COLUMN _parent0_index;
ALTER TABLE Requiem_ProcessArg RENAME temp_useless_column to _parent0_index;
/* Requiem_ProcessArg->_parent0_index  data type updated*/
/* Update the data type of the column _index from Requiem_ProcessArg */
ALTER TABLE Requiem_ProcessArg ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_ProcessArg SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_ProcessArg DROP COLUMN _index;
ALTER TABLE Requiem_ProcessArg RENAME temp_useless_column to _index;
/* Requiem_ProcessArg->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_ProcessEnv */
ALTER TABLE Requiem_ProcessEnv ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_ProcessEnv SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_ProcessEnv DROP COLUMN _parent0_index;
ALTER TABLE Requiem_ProcessEnv RENAME temp_useless_column to _parent0_index;
/* Requiem_ProcessEnv->_parent0_index  data type updated*/
/* Update the data type of the column _index from Requiem_ProcessEnv */
ALTER TABLE Requiem_ProcessEnv ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_ProcessEnv SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_ProcessEnv DROP COLUMN _index;
ALTER TABLE Requiem_ProcessEnv RENAME temp_useless_column to _index;
/* Requiem_ProcessEnv->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_WebService */
ALTER TABLE Requiem_WebService ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_WebService SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_WebService DROP COLUMN _parent0_index;
ALTER TABLE Requiem_WebService RENAME temp_useless_column to _parent0_index;
/* Requiem_WebService->_parent0_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_WebServiceArg */
ALTER TABLE Requiem_WebServiceArg ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_WebServiceArg SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_WebServiceArg DROP COLUMN _parent0_index;
ALTER TABLE Requiem_WebServiceArg RENAME temp_useless_column to _parent0_index;
/* Requiem_WebServiceArg->_parent0_index  data type updated*/
/* Update the data type of the column _index from Requiem_WebServiceArg */
ALTER TABLE Requiem_WebServiceArg ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_WebServiceArg SET temp_useless_column = CAST(_index AS INT4);
ALTER TABLE Requiem_WebServiceArg DROP COLUMN _index;
ALTER TABLE Requiem_WebServiceArg RENAME temp_useless_column to _index;
/* Requiem_WebServiceArg->_index  data type updated*/

/* Update the data type of the column _parent0_index from Requiem_SNMPService */
ALTER TABLE Requiem_SNMPService ADD COLUMN temp_useless_column INT4;
UPDATE Requiem_SNMPService SET temp_useless_column = CAST(_parent0_index AS INT4);
ALTER TABLE Requiem_SNMPService DROP COLUMN _parent0_index;
ALTER TABLE Requiem_SNMPService RENAME temp_useless_column to _parent0_index;
/* Requiem_SNMPService->_parent0_index  data type updated*/

ALTER TABLE Requiem_Linkage ADD PRIMARY KEY (_message_ident, _parent0_index, _parent1_index, _index);

ALTER TABLE Requiem_Impact ADD CHECK (severity  IN ('info', 'low', 'medium', 'high')); 

DROP INDEX requiem_node_index_name;
DROP INDEX requiem_node_index_location;
CREATE INDEX requiem_node_index_location ON Requiem_Node (_parent_type,_parent0_index,location);
CREATE INDEX requiem_node_index_name ON Requiem_Node (_parent_type,_parent0_index,name);
DROP INDEX requiem_address_index_address;
CREATE INDEX requiem_address_index_address ON Requiem_Address (_parent_type,_parent0_index,_index,address);

DROP INDEX requiem_service_index_protocol_port;
DROP INDEX requiem_service_index_protocol_name;
CREATE INDEX requiem_service_index_protocol_port ON Requiem_Service (_parent_type,_parent0_index,protocol, port);
CREATE INDEX requiem_service_index_protocol_name ON Requiem_Service (_parent_type,_parent0_index,protocol, name);

