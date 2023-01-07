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

  local function getnext (self)
    local banks_ = self.banks
    local next_ = self.next

    if (next_ + 1 > banks_:length ()) then
      self.next = 1
    else
      self.next = next_ + 1
    end
  return banks_ [next_]
  end

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

  function splitters.zero (self, size)
    checkArg (0, self, 'SmipsSplitter')
    checkArg (1, size, 'number')
    local words = (size - (size % 4)) / 4
    local left = size % 4

    if (left > 0) then
      error ('Unaligned write')
    else
      for i = 1, words do
        local _start = 1 + (i - 1) * 4
        local _end = _start + 3;
        (getnext (self)):emit32 (0)
      end
    end
  end

  function splitters.emit32 (self, value)
    checkArg (0, self, 'SmipsSplitter')
  return (getnext (self)):emit32 (value)
  end

  function splitters.emits (self, value)
    checkArg (0, self, 'SmipsSplitter')
    checkArg (1, value, 'string')
    local size = #value
    local next_ = self.next
    local words = (size - (size % 4)) / 4
    local left = size % 4

    if (left > 0) then
      error ('Unaligned write')
    else
      for i = 1, words do
        local _start = 1 + (i - 1) * 4
        local _end = _start + 3
        local sub = value:sub (_start, _end);
        (getnext (self)):emits (sub)
      end
    end
  end

  function splitters.close (self)
    checkArg (0, self, 'SmipsSplitter')
    for _, bank in ipairs (self.banks) do
      bank:close ()
    end
  end
return splitters
end
