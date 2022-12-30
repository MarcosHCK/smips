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

local function main (...)
  local files = {...}
  local output, input
  local file

  local function process_stat (stat)
    stat = stat:gsub ('^%s*', '')
    stat = stat:gsub ('%s*$', '')

    local tag = stat:match ('^[a-zA-Z_]+%:$')
    if (tag ~= nil) then
      print (('tag \'%s\''):format (tag))
    else
      print (('stat \'%s\''):format (stat))
    end
  end

  local function process_line (line)
    local stat, left = line:match ('([^:]+:)(.+)')
    if (not stat) then
      process_stat (line)
    else
      process_stat (stat)
      process_line (left)
    end
  end

  local function process_file (file)
    local line;

    repeat
      line = file:read ('*l')
      if (line) then
        line = line:gsub ('#.*$', '')

        for line in line:gmatch ('[^;]+') do
          if (#line > 1) then
            process_line (line)
          end
        end
      end
    until (not line)
  end

  for _, file in ipairs (files) do
    file = assert (io.open (file, 'r'))
    process_file (file)
    file:close ()
  end
end

app:main (main, ...)
