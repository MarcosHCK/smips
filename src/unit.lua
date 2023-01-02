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
local tags = require ('tags')
local vector = require ('vector')
local unit = {}

do
  local mt =
  {
    __index = unit,
    __name = 'SmipsUnit',
  }

  function unit.new ()
    local block = vector.new ()
    local st = { block = block, tags = { }, }
      block:append ({ size = 0 })
    return setmetatable (st, mt)
  end

  function unit.add_data (self, data)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, data, 'string', 'number')

    if (type (data) == 'string') then
      self.block:append ({ data = data, })
    else
      local size
      size = #data
      size = size + (4 - (size % 4))
      self.block:append ({ size = size, })
    end
  end

  function unit.add_inst (self, inst)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, inst, 'SmipsInst')

    self.block:append ({
        size = 4,
        inst = inst,
      })
  end

  function unit.add_tag (self, tagname)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, tagname, 'string')

    if (self.tags [tagname] ~= nil) then
      error (('Redefined tag %s'):format (tagname))
    else
      local value = self.block:length ()
      local tag = tags.rel (value)
      self.tags [tagname] = tag
    end
  end
return unit
end
