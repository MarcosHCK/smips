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
local log = require ('log')

local anontag
do
  local anons = 0
  function anontag ()
    anons = anons + 1
    return ('__anon%i__'):format (anons)
  end
end

do
  local regs = require ('isa').regs
  local r_insts = require ('isa').r_insts
  local i_insts = require ('isa').i_insts
  local j_insts = require ('isa').j_insts
  local defaults = require ('isa').defaults
  local i_directives = require ('isa').i_directives
  local a_directives = require ('isa').a_directives
  local l_directives = require ('isa').l_directives

  local function argiter ()
    local nexti = 0
    return function (...)
      nexti = nexti + 1
      local arg = select (nexti, ...)
      if (arg) then
        arg = arg:gsub ('^%s*', '')
        arg = arg:gsub ('%s*$', '')
        return arg
      end
    end
  end

  local function breakargs (args)
    local arg, left = args:match ('^([^,]+)(.*)$')
    if (arg) then
      left = left:gsub ('^,', '')
      return arg, breakargs (left)
    end
  end

  local function getreg (value)
    local reg = value:match ('^%$([0-9a-z]+)$')

    if (not reg) then
      return nil
    else
      return regs [reg]
    end
  end

  local function feed (unit, source)
    local linen = 0
    local seq = 0
    local line

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

    local function assertreg (value)
      if (value == nil) then
        compe ('Expected register name')
      end

      local reg = getreg (value)

      if (not reg) then
        compe ('Unknown register \'%s\'', value)
      end
    return reg
    end

    local function assertcs (value)
      if (value == nil) then
        compe ('Expected expression')
      end
    return value
    end

    local function put_rinst (desc, rt, rs, rd, shamt)
      local inst, loc
      inst = insts.new (desc.opcode)
      inst = inst:typer ()

      if (not shamt) then
        shamt = 0
      else
        local env = {}
        local expr = ('do return %s; end'):format (shamt)
        local chunk, reason, result

        chunk, reason = load (expr, '=shamt', 't', env)
        if (not chunk) then
          compe (reason)
        else
          result, reason = pcall (chunk)
          if (not result) then
            compe (reason)
          else
            if (type (reason) ~= 'number') then
              compe ('Shift amount shold be a constant number')
            else
              shamt = reason
            end
          end
        end
      end

      inst.rt = rt
      inst.rs = rs
      inst.rd = rd
      inst.shamt = shamt
      inst.func = desc.func
      unit:add_inst (inst)
      unit:annotate (source, linen)
    end

    local function put_iinst (desc, rt, rs, cs)
      local inst, loc, addr
      inst = insts.new (desc.opcode)
      inst = inst:typei ()
      inst.rt = desc.takes.rs_first and rs or rt
      inst.rs = desc.takes.rs_first and rt or rs

      if (desc.address) then
        local offset, left = cs:match ('^(%-?[0-9]+)%(([^%)]+)%)$')
        if (not offset) then
          if (desc.address == 'e') then
            compe ('Invalid address \'%s\'', cs)
          elseif (desc.address ~= 'l') then
            error ('Fix this!')
          end
        else
          local reg = getreg (left)
          if (not reg) then
            compe ('Invalid register \'%s\'', left)
          else
            inst.rs = reg
            inst.constant = offset
            cs = nil
          end
        end
      end

      unit:add_inst (inst, cs, desc.tagable)
      unit:annotate (source, linen)
      unit:sequence (seq)
    end

    local function put_jinst (desc, cs)
      local inst, loc
      inst = insts.new (desc.opcode)
      inst = inst:typej ()
      unit:add_inst (inst, cs, desc.tagable)
      unit:annotate (source, linen)
      unit:sequence (seq)
    end

    local macros =
    {
      jal = function (getnext, ...)
        local return_ = anontag ()
        local target_ = assertcs (getnext (...))

        put_iinst (i_insts.la, regs ['ra'], 0, return_)
        put_jinst (j_insts.j, target_)
        unit:add_tag (return_)
      end,
    }

    local function feed_tag (tagname, islocal)
      if (islocal) then
        local anon = anontag ()
        local alias = tonumber (tagname)
        unit:add_local (alias, seq, anon)
      else
        if (tagname:match ('__anon([0-9]+)__')) then
          compe ('Tag name \'%s\' is reserved')
        else
          if (unit.tags [tagname] ~= nil) then
            compe ('Redefined tag \'%s\'', tagname)
          else
            unit:add_tag (tagname)
          end
        end
      end
    end

    local function feed_directive (name, ...)
      if (i_directives [name] ~= nil) then
        local directive = i_directives [name]
        if ((...) ~= nil) then
          compe ('Directive \'%s\' takes no arguments', name)
        else
          directive (unit, compe)
        end
      elseif (a_directives [name] ~= nil) then
        local directive = a_directives [name]
        local arg, left = ...

        if (left ~= nil) then
          compe ('Directive takes only one argument')
        else
          directive (arg, unit, compe)
        end
      elseif (l_directives [name] ~= nil) then
        local directive = l_directives [name]
        local getnext = argiter ()
        local first = true

        while (true) do
          local arg = getnext (...)
          if (not arg and first) then
            compe ('Directive takes at least an argument')
          elseif (arg) then
            directive (arg, unit, compe)
            first = false
          else
            break
          end
        end
      else
        compe ('Unknown directive \'%s\'', name)
      end
    end

    local function feed_inst (inst, ...)
      local getnext = argiter ()

      if (macros [inst] ~= nil) then
        macros [inst] (getnext, ...)
      elseif (r_insts [inst] ~= nil) then
        local desc = r_insts [inst]
        local takes = desc.takes or {}
        local _defaults = desc.defaults or defaults
        local rd = takes.rd and assertreg (getnext (...)) or _defaults.rd
        local rs = takes.rs and assertreg (getnext (...)) or _defaults.rs
        local rt = takes.rt and assertreg (getnext (...)) or _defaults.rt
        local shamt = takes.shamt and assertcs (getnext (...)) or _defaults.shamt
        put_rinst (desc, rt, rs, rd, shamt)
      elseif (i_insts [inst] ~= nil) then
        local desc = i_insts [inst]
        local takes = desc.takes or {}
        local _defaults = desc.defaults or defaults
        local rt = takes.rt and assertreg (getnext (...)) or _defaults.rt
        local rs = takes.rs and assertreg (getnext (...)) or _defaults.rs
        local cs = takes.cs and assertcs (getnext (...)) or _defaults.cs
        put_iinst (desc, rt, rs, cs)
      elseif (j_insts [inst] ~= nil) then
        local desc = j_insts [inst]
        local takes = desc.takes or {}
        local _defaults = desc.defaults or defaults
        local cs = takes.cs and assertcs (getnext (...)) or _defaults.cs
        put_jinst (desc, cs)
      else
        compe ('Unknown instruction \'%s\'', inst)
      end

      if (getnext (...) ~= nil) then
        compe ('Junk at end of instruction')
      end
    end

    local function feed_stat (stat)
      local tag = stat:match ('^([0-9]+):$')
              or stat:match ('^([a-zA-Z_][a-zA-Z_0-9]*):$')
      if (tag ~= nil) then
        return feed_tag (tag, not not tag:find ('^[0-9]'))
      end

      local name, left = stat:match ('^%.([a-z]+)(.*)$')
      if (name ~= nil) then
        if (#left == 0) then
          return feed_directive (name)
        else
          local args = left:match ('^%s+(.*)$')
          if (not args) then
            compe ('Malformed line \'%s\'', line)
          else
            return feed_directive (name, breakargs (args))
          end
        end
      end

      local inst, left = stat:match ('^([a-z]+)(.*)$')
      if (inst ~= nil) then
        if (#left == 0) then
          return feed_inst (inst)
        else
          local args = left:match ('^%s+(.*)$')
          if (not args) then
            compe ('Malformed line \'%s\'', line)
          else
            return feed_inst (inst, breakargs (args))
          end
        end
      end

      compe ('Malformed line \'%s\'', line)
    end

    local function feed_rstat (stat)
      stat = stat:gsub ('^%s*', '')
      stat = stat:gsub ('%s*$', '')
      if (#stat > 0) then
        seq = seq + 1
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
return feed
end
