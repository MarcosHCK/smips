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
local app = require ('application')
local log = require ('log')
local units = require ('unit')

local function process (unit)
  local linen = 0

  local function process_stat (stat)
    stat = stat:gsub ('^%s*', '')
    stat = stat:gsub ('%s*$', '')

    local tag = stat:match ('^([a-zA-Z_]+)%:$')
    if (tag ~= nil) then
      unit.add_tag (tag)
    else
      local inst, left = stat:match ('^([a-z]+)%s*(.*)$')
      if (not inst) then
        log.error ('Malformed line')
        return true
      else
        print (inst)
      end
    end
  end

  local function process_line (line)
    local stat, left = line:match ('([^:]+:)(.+)')
    if (not stat) then
      if (process_stat (line)) then
        return true
      end
    else
      if (process_stat (stat)) then
        return true
      end
      if (process_line (left)) then
        return true
      end
    end
  end

  local line;

  repeat
    line = io.read ('*l')
    if (line) then
      line = line:gsub ('#.*$', '')
      linen = linen + 1

      for line in line:gmatch ('[^;]+') do
        if (#line > 1) then
          if (process_line (line)) then
            return true
          end
        end
      end
    end
  until (not line)
end

local function main (...)
  local files = {...}
  local output

  local unit = units.new ()

  for _, file in ipairs (files) do
    io.input (assert (io.open (file, 'r')))
    process (unit)
  end
end

app:main (main, ...)
