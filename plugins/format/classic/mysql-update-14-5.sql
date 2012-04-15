UPDATE _format SET version="14.5";

ALTER TABLE Requiem_SnmpService DROP community;
ALTER TABLE Requiem_SnmpService ADD COLUMN message_processing_model INTEGER UNSIGNED NULL;
ALTER TABLE Requiem_SnmpService ADD COLUMN security_model INTEGER UNSIGNED NULL;
ALTER TABLE Requiem_SnmpService ADD COLUMN security_level INTEGER UNSIGNED NULL;