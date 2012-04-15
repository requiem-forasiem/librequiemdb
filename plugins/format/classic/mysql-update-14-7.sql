BEGIN;

UPDATE _format SET version="14.7";
ALTER TABLE Requiem_Impact CHANGE description description TEXT NULL;

COMMIT;
