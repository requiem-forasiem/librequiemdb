BEGIN;

UPDATE _format SET version='14.7';
ALTER TABLE Requiem_Impact ALTER COLUMN description TYPE TEXT;
ALTER TABLE Requiem_Impact ALTER COLUMN description DROP NOT NULL;

COMMIT;
