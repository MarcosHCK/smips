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
local insts = require ('insts')
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
    return setmetatable (st, mt)
  end

  function unit.getpos (self, tagname)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, tagname, 'nil', 'string')

    if (not tagname) then
      local block = self.block
      if (block:length () == 0) then
        return 0
      else
        local last = block:last ()
        local offset = last.offset
        local size = last.size
        return offset + size
      end
    else
      return self.tags [tagname]
    end
  end

  function unit.add_tag (self, tagname)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, tagname, 'string')

    if (self.tags [tagname] ~= nil) then
      error (('Redefined tag %s'):format (tagname))
    else
      local pos = self:getpos ()
      self.tags [tagname] = pos
    end
  end

  function unit.add_inst (self, inst)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, inst, 'SmipsInst')
    inst.offset = self:getpos ()
    inst.size = 4
    self.block:append (inst)
  end
return unit
end
