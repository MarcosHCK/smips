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

do
  local function main (app, files)
    print (app:getopt ('o'))
  return true
  end

  local opt = require ('options')
  local app = nil

  app =
  {
    context = opt.Context ('files'),

    groups =
    {
      main = opt.Group ('Main', 'Main Options', 'Main Options'),
    },

    getopt = function (self, name)
      for _, group in ipairs (self.groups) do
        local value = group:option (name)
        if (value ~= nil) then
          return value
        end
      end
    end,
  }

  local result = { app.context:parse (...) }

  if (not result [1] and #result > 1) then
    io.stderr:write (result [2], '\n')
  else
    main (app, result)
  end
end
