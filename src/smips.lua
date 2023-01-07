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
local feed = require ('feed')
local opt = require ('options')
local process = require ('process')
local splitters = require ('splitters')
local units = require ('unit')
local utils = require ('utils')

do
  local function printout (unit, bank)
    for _, ent in ipairs (unit.block) do
      if (ent.inst) then
        bank:emit32 (ent.inst:encode ())
      elseif (ent.data) then
        bank:emits (ent.data)
      elseif (ent.size) then
        bank:zero (ent.size)
      end
    end

    if (pcall (checkArg, 1, bank, 'SmipsBank')) then
      bank:emit32 (-1)
    end
      bank:close ()
  end

  local function main (...)
    local files = {...}
    local split = opt:getopt ('s')
    local output = opt:getopt ('o')
    local unit = units.new ()

    for _, file in ipairs (files) do
      if (file == '-') then
        feed (unit, '(stdin)')
      else
        io.input (assert (io.open (file, 'r')))
        feed (unit, file)
      end
    end

    process (unit)

    if (not split) then
      printout (unit, banks.new (output or '-'))
    else
      if (output ~= nil) then
        printout (unit, splitters.new (output, split))
      else
        printout (unit, splitters.new (utils.pwd (), split))
      end
    end
  end
return main (opt:parse (...))
end
