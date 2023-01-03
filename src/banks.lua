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
local banks = {}

do
  local mt =
  {
    __index = banks,
    __name = 'SmipsBank',

    __gc = function (self)
      checkArg (0, self, 'SmipsBank')
      self:close ()
    end,
  }

  function banks.new (output)
    checkArg (1, output, 'string')
    if (output == '-') then
      local st = { stream = io.stdout, }
        st.stream:write ('v2.0 raw\n')
      return setmetatable (st, mt)
    else
      local file = assert (io.open (output, 'w'))
      local st = { stream = file, }
        st.stream:write ('v2.0 raw\n')
      return setmetatable (st, mt)
    end
  end

  function banks.emit8 (self, byte)
    checkArg (0, self, 'SmipsBank')
    checkArg (1, byte, 'number')
    self.stream:write (('%02x'):format (byte))
  end

  function banks.emit32 (self, word)
    checkArg (0, self, 'SmipsBank')
    checkArg (1, word, 'number')

    local step1 = word
    local byte1 = step1 % 256
    local step2 = ((step1 - byte1) / 256)
    local byte2 = step2 % 256
    local step3 = ((step2 - byte2) / 256)
    local byte3 = step3 % 256
    local step4 = ((step3 - byte3) / 256)
    local byte4 = step4 % 256

    self:emit8 (byte1)
    self:emit8 (byte2)
    self:emit8 (byte3)
    self:emit8 (byte4)
  end

  function banks.emit (self, word)
    checkArg (0, self, 'SmipsBank')
    checkArg (1, word, 'number')
    self:emit32 (word)
    self.stream:write ('\n')
  end

  function banks.close (self)
    checkArg (0, self, 'SmipsBank')
    self.stream:write ('ffffffff')
    self.stream:close ()
  end
return banks
end
