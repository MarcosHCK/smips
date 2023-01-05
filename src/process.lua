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
local tags = require ('tags')

do
  local function process (unit)
    local source, linen
    local offset = 0

    local function compe (...)

      local function collect (...)
        if (select ('#', ...) > 1) then
          return string.format (...)
        else
          return (...)
        end
      end

      local literal = collect (...)
      local where = ('%s: %i'):format (source, linen)
      log.error (collect ('%s: %s', where, literal))
    end

    local function expression (expr)
      local env, mt, reason
      local chunk, result

      env =
      {
        _ = offset,
        tonumber = tonumber,
        tostring = tostring,
        math = setmetatable ({}, { __mode = 'protected', __index = _G.math, }),
        string = setmetatable ({}, { __mode = 'protected', __index = _G.string, }),
      }

      mt =
      {
        __index = function (self, key)
          local tags = unit.tags
          if (tags [key]) then
            return tags [key]
          else
            compe ('Undefined tag \'%s\'', key)
          end
        end,
      }

      setmetatable (env, mt)

      expr = ('do return %s; end'):format (expr)
      chunk, reason = load (expr, '=expression', 't', env)
      if (not chunk) then
        compe (reason)
      else
        return (chunk ())
      end
    end

    local function calculate (tag)
      checkArg (1, tag, 'SmipsTag')

      local type, subtype = tag:type ()
      if (type == 'value') then
        if (subtype == 'absolute') then
          return tag.value
        elseif (subtype == 'relative') then
          local block = unit.block
          local idx = tag.value
          local at = block [idx]
          return at.offset + at.size
        else
          error ('Unknown type ' .. subtype)
        end
      else
        local left = tag.left
        local right = tag.right

        if (subtype == 'add') then
          return calculate (left) + calculate (right)
        elseif (subtype == 'sub') then
          return calculate (left) - calculate (right)
        elseif (subtype == 'mul') then
          return calculate (left) * calculate (right)
        elseif (subtype == 'div') then
          return calculate (left) / calculate (right)
        elseif (subtype == 'mod') then
          return calculate (left) % calculate (right)
        elseif (subtype == 'unm') then
          return -calculate (left)
        else
          error ('Unknown operation ' .. subtype)
        end
      end
    end

    for i, ent in ipairs (unit.block) do
      if (not ent.loc) then
        source = '?'
        linen = -1
      else
        source = ent.loc.source
        linen = ent.loc.line
      end

      if (ent.inst ~= nil) then
        local inst = ent.inst
        local cs = ent.extra [1]
        local style = ent.extra [2]

        if (cs) then
          local const = expression (cs)
          if (pcall (checkArg, 1, const, 'SmipsTag')) then
            if (style == 'r') then
              ent.const = ((const - tags.rel (i - 1)) / 4) - 1
            elseif (style == 'j') then
              ent.const = const / 4
            elseif (style == 'a') then
              ent.const = const
            else
              compe ('Non tagable instruction')
            end
          elseif (type (const) == 'number') then
            if (style == 'r') then
              ent.const = ((const - tags.rel (i - 1)) / 4) - 1
            else
              inst.constant = const
            end
          elseif (type (const) == 'string') then
            if (#const < 5) then
              inst.constant = const:byte (1, #const)
            else
              compe ('Data too big to fit into a register')
            end
          else
            compe ('Value should be constant number')
          end
        end
      elseif (ent.data) then
        local size = #ent.data
        local mis = size % 4
        local cors = mis > 0 and 4 - mis or 0
        local corz = (string.char (0)):rep (cors)
          ent.size = size + cors
          ent.data = ent.data .. corz
      elseif (ent.size) then
        local size = ent.size
        local char = string.char (0)
        local data = char:rep (size)
          ent.data = data
      end

      ent.offset = offset
      offset = offset + ent.size
    end

    for _, ent in ipairs (unit.block) do
      if (not ent.loc) then
        source = '?'
        linen = -1
      else
        source = ent.loc.source
        linen = ent.loc.line
      end

      if (ent.const) then
        local tag = ent.const
        local inst = ent.inst
        inst.constant = calculate (tag)
      end
    end
  end
return process
end
