import requiem

from _requiemdb import *

class RequiemDBError(requiem.RequiemError):
    def __str__(self):
        if self._strerror:
            return self._strerror
        return requiemdb_strerror(self.errno)
