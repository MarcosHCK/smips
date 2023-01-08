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
    local st =
    {
      block = vector.new (),
      locals = { },
      tags = { },
    }

      st.block:append ({ size = 0 })
  return setmetatable (st, mt)
  end

  function unit.last (self)
    checkArg (0, self, 'SmipsUnit')
    local block = self.block
  return block:last ()
  end

  function unit.annotate (self, source, line)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, source, 'string')
    checkArg (2, line, 'number')
    local last = self.block:last ()

    if (not last) then
      error ('Empty unit')
    else
      last.loc =
        {
          source = source,
          line = line,
        }
    end
  end

  function unit.sequence (self, seq)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, seq, 'number')
    local last = self.block:last ()

    if (not last) then
      error ('Empty unit')
    else
      last.seq = seq
    end
  end

  function unit.add_data (self, data, ...)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, data, 'string', 'number')

    if (type (data) == 'number') then
      local una = data
      local mis = data % 4
      local size = mis > 0 and una + (4 - mis) or una
      self.block:append ({ size = size, extra = {...}, })
    else
      self.block:append ({ data = data, extra = {...}, })
    end
  end

  function unit.add_inst (self, inst, ...)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, inst, 'SmipsInst')

    self.block:append ({
        size = 4,
        inst = inst,
        extra = {...},
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

  function unit.add_local (self, alias, seq, tagname)
    checkArg (0, self, 'SmipsUnit')
    checkArg (1, alias, 'number')
    checkArg (2, seq, 'number')
    checkArg (3, tagname, 'string')
    local locals = self.locals

    if (not locals [alias]) then
      locals [alias] = vector.new ()
    end do
      locals = locals [alias]
      locals:append ({tagname = tagname, seq = seq, })
      self:add_tag (tagname)
    end
  end
return unit
end
