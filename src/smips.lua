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
local log = require ('log')
local opt = require ('options')

do
  local function feed (unit)
    local linen = 0
    local line

    local function compe (...)
      if (select ('#', ...) > 1) then
        log.error ('%i: %s', linen, string.format (...))
      else
        log.error ('%i: %s', linen, (...))
      end
    end

    local function breakargs (args)
      local arg, left = args:match ('^([^,]+)(.*)$')
      if (arg) then
        left = left:gsub ('^,', '')
        return arg, breakargs (left)
      end
    end

    local function feed_tag (tag)
    end

    local function feed_inst (inst, ...)
    end

    local function feed_stat (stat)
      local tag = stat:match ('^(%.?[a-zA-Z_][a-zA-Z_0-9]*):$')
      if (tag ~= nil) then
        feed_tag (tag)
      else
        local inst, left = stat:match ('^([a-z]+)(.*)$')
        if (not inst) then
          compe ('Malformed line \'%s\'', line)
        else
          if (#left == 0) then
            feed_inst (inst)
          else
            local args = left:match ('^%s+(.*)$')
            if (not args) then
              compe ('Malformed line \'%s\'', line)
            else
              feed_inst (inst, breakargs (args))
            end
          end
        end
      end
    end

    local function feed_rstat (stat)
      stat = stat:gsub ('^%s*', '')
      stat = stat:gsub ('%s*$', '')
      if (#stat > 0) then
        feed_stat (stat)
      end
    end

    local function feed_chunk (chunk)
      local stat, left = chunk:match ('([^:]+:)(.+)')
      if (not stat) then
        feed_rstat (chunk)
      else
        feed_rstat (stat)
        feed_chunk (left)
      end
    end

    repeat
      line = io.read ('*l')
      if (line) then
        line = line:gsub ('#.*$', '')
        linen = linen + 1

        for chunk in line:gmatch ('[^;]+') do
          feed_chunk (chunk)
        end
      end
    until (not line)
  end

  local function main (...)
    local files = {...}
    local unit = nil

    for _, file in ipairs (files) do
      io.input (assert (io.open (file, 'r')))
      feed (unit)
    end
  end

  main (opt:parse (...))
end
