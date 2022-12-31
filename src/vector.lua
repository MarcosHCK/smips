--[[
-- Copyright 2021-2025 MarcosHCK
--  This file is part of SMIPS Assembler.
--
--  SMIPS Assembler is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  (at your option) any later version.
--
--  SMIPS Assembler is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with SMIPS Assembler.  If not, see <http://www.gnu.org/licenses/>.
]]
local vector = {}

do
  local mt =
  {
    __index = vector,
    __name = 'vector',
  }

  function vector.new ()
    local st = {}
    return setmetatable (st, mt)
  end

  function vector.length (self)
    checkArg (0, self, 'vector')
    return #self
  end

  function vector.append (self, val)
    checkArg (0, self, 'vector')
    table.insert (self, val)
    return val
  end

  function vector.prepend (self, val)
    checkArg (0, self, 'vector')
    table.insert (self, 1, val)
    return val
  end

  function vector.last (self)
    checkArg (0, self, 'vector')
    return self [#self]
  end
return vector
end
