ALTER TABLE Requiem_Impact CHANGE severity severity ENUM("info", "low", "medium", "high") NULL;
UPDATE _format SET version="14.1";
