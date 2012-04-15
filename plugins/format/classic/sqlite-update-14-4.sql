UPDATE _format SET version='14.4'; 

BEGIN;

ALTER TABLE Requiem_AdditionalData RENAME TO Requiem_AdditionalDataOld;

CREATE TABLE Requiem_AdditionalData (
 _message_ident INTEGER NOT NULL,
 _parent_type TEXT NOT NULL,
 _index INTEGER NOT NULL,
 type TEXT NOT NULL,
 meaning TEXT NULL,
 data BLOB NOT NULL,
 PRIMARY KEY (_parent_type, _message_ident, _index)
) ;

INSERT INTO Requiem_AdditionalData SELECT * FROM Requiem_AdditionalDataOld;
DROP TABLE Requiem_AdditionalDataOld;

COMMIT;
