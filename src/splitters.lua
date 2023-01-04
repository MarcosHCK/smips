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
local banks = require ('banks')
local utils = require ('utils')
local vector = require ('vector')
local splitters = {}

do
  local mt =
  {
    __index = splitters,
    __name = 'SmipsSplitter',
  }

  function splitters.new (dir, names_)
    checkArg (1, dir, 'string')
    checkArg (2, names_, 'string')
    local banks_ = vector.new ()
    local st = { banks = banks_, next = 1, }

    do
      local names = {}

      for name in names_:gmatch ('([^,]+)') do
        names [#names + 1] = name
      end

      for _, name in ipairs (names) do
        local path = utils.build_path (dir, name)
        local bank = banks.new (path)
        banks_:append (bank)
      end
    end
  return setmetatable (st, mt)
  end

  function splitters.emit (self, ...)
    checkArg (0, self, 'SmipsSplitter')
    local banks_ = self.banks
    local main_ = self.main
    local next_ = self.next
    local bank = banks_ [next_]
    

    if (next_ + 1 > banks_:length ()) then
      self.next = 1
    else
      self.next = next_ + 1
    end
  return bank:emit (...)
  end

  function splitters.close (self)
    checkArg (0, self, 'SmipsSplitter')
    for _, bank in ipairs (self.banks) do
      bank:close ()
    end
  end
return splitters
end
