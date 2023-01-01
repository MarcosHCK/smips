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
local opt = require ('options')
local regs = require ('regs')
local units = require ('unit')

do
  local r_insts =
  {
    nop = { opcode = 0, func = 0, takes = { rd = false, rt = false, rs = false, }, },
    add = { opcode = 0, func = 32, takes = { rd = true, rt = true, rs = true, }, },
    sub = { opcode = 0, func = 34, takes = { rd = true, rt = true, rs = true, }, },
    mult = { opcode = 0, func = 24, takes = { rd = false, rt = true, rs = true, }, },
    mulu = { opcode = 0, func = 25, takes = { rd = false, rt = true, rs = true, }, },
    div = { opcode = 0, func = 26, takes = { rd = false, rt = true, rs = true, }, },
    divu = { opcode = 0, func = 27, takes = { rd = false, rt = true, rs = true, }, },
    slt = { opcode = 0, func = 42, takes = { rd = true, rt = true, rs = true, }, },
    sltu = { opcode = 0, func = 43, takes = { rd = true, rt = true, rs = true, }, },
    ['and'] = { opcode = 0, func = 36, takes = { rd = true, rt = true, rs = true, }, },
    ['or'] = { opcode = 0, func = 37, takes = { rd = true, rt = true, rs = true, }, },
    ['nor'] = { opcode = 0, func = 39, takes = { rd = true, rt = true, rs = true, }, },
    ['xor'] = { opcode = 0, func = 40, takes = { rd = true, rt = true, rs = true, }, },
    pop = { opcode = 56, func = 0, takes = { rd = true, rt = false, rs = false, }, },
    push = { opcode = 56, func = 1, takes = { rd = false, rt = false, rs = true, }, },
    jr = { opcode = 0, func = 8, takes = { rd = false, rt = false, rs = true, }, },
    mfhi = { opcode = 0, func = 16, takes = { rd = true, rt = false, rs = false, }, },
    mflo = { opcode = 0, func = 18, takes = { rd = true, rt = false, rs = false, }, },
    halt = { opcode = 63, func = 63, takes = { rd = false, rt = false, rs = false, }, },
    tty = { opcode = 63, func = 1, takes = { rd = false, rt = false, rs = true, }, },
    rnd = { opcode = 63, func = 2, takes = { rd = true, rt = false, rs = false, }, },
    kbd = { opcode = 63, func = 4, takes = { rd = true, rt = false, rs = false, }, },
  }

  local i_insts =
  {
    addi = { opcode = 8, takes = { rt = true, rs = true, cs = true, }, },
    slti = { opcode = 10, takes = { rt = true, rs = true, cs = true, }, },
    sltiu = { opcode = 11, takes = { rt = true, rs = true, cs = true, }, },
    andi = { opcode = 12, takes = { rt = true, rs = true, cs = true, }, },
    ori = { opcode = 13, takes = { rt = true, rs = true, cs = true, }, },
    xori = { opcode = 14, takes = { rt = true, rs = true, cs = true, }, },
    lw = { opcode = 35, takes = { rt = true, rs = false, cs = true, }, address = true, },
    sw = { opcode = 43, takes = { rt = true, rs = false, cs = true, }, address = true, },
    beq = { opcode = 4, takes = { rt = true, rs = true, cs = true, }, tagable = 'r', },
    bne = { opcode = 5, takes = { rt = true, rs = true, cs = true, }, tagable = 'r', },
    blez = { opcode = 6, takes = { rt = false, rs = true, cs = true, }, tagable = 'r', },
    bgtz = { opcode = 7, takes = { rt = false, rs = true, cs = true, }, tagable = 'r', },
    bltz = { opcode = 1, takes = { rt = false, rs = true, cs = true, }, tagable = 'r', },
  }

  local j_insts =
  {
    j = { opcode = 2, takes = { cs = true, }, tagable = 'a', },
  }

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

  local anons = 0
  local function anontag ()
    anons = anons + 1
  return ('@anon%i'):format (anons)
  end

  local function feed (unit, source)
    local linen = 0
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

    local function feed_tag (tagname)
      unit:add_tag (tagname)
    end

    local function feed_directive (name, ...)
      error ('Unimplemented')
    end

    local function put_rinst (desc, rt, rs, rd)
      local inst
      inst = insts.new (desc.opcode)
      inst = inst:typer ()
      inst.rt = rt
      inst.rs = rs
      inst.rd = rd
      inst.func = desc.func
      unit:add_inst (inst)
    end

    local function put_iinst (desc, rt, rs, cs)
      local inst
      inst = insts.new (desc.opcode)
      inst = inst:typei ()
      inst.rt = rt
      inst.rs = rs
      inst.cs = cs
      unit:add_inst (inst)
    end

    local function put_jinst (desc, cs)
      local inst
      inst = insts.new (desc.opcode)
      inst = inst:typej ()
      inst.cs = cs
      unit:add_inst (inst)
    end

    local macros =
    {
      jal = function (getnext, ...)
        local return_ = anontag ()
        local target_ = assertcs (getnext (...))

        put_iinst (i_insts.addi, regs ['ra'], 0, return_)
        put_jinst (j_insts.j, target_)
        unit:add_tag (return_)
      end,
      move = function (getnext, ...)
        local rt = assertreg (getnext (...))
        local arg = getnext (...)
        local rs = getreg (arg)

        if (rs ~= nil) then
          put_iinst (i_insts.addi, rt, rs, 0)
        else
          put_iinst (i_insts.addi, rt, 0, arg)
        end
      end,
    }

    local function feed_inst (inst, ...)
      local getnext = argiter ()

      if (macros [inst] ~= nil) then
        macros [inst] (getnext, ...)
      elseif (r_insts [inst] ~= nil) then
        local desc = r_insts [inst]
        local takes = desc.takes
        local rd = takes.rd and assertreg (getnext (...)) or 0
        local rs = takes.rs and assertreg (getnext (...)) or 0
        local rt = takes.rt and assertreg (getnext (...)) or 0
        put_rinst (desc, rt, rs, rd)
      elseif (i_insts [inst] ~= nil) then
        local desc = i_insts [inst]
        local takes = desc.takes
        local rt = takes.rt and assertreg (getnext (...)) or 0
        local rs = takes.rs and assertreg (getnext (...)) or 0
        local cs = takes.cs and assertcs (getnext (...)) or '0'
        put_iinst (desc, rt, rs, cs)
      elseif (j_insts [inst] ~= nil) then
        local desc = j_insts [inst]
        local takes = desc.takes
        local cs = takes.cs and assertcs (getnext (...)) or '0'
        put_jinst (desc, cs)
      else
        compe ('Unknown instruction \'%s\'', inst)
      end

      if (getnext (...) ~= nil) then
        compe ('Junk at end of instruction')
      end
    end

    local function feed_stat (stat)
      local tag = stat:match ('^(%.?[a-zA-Z_][a-zA-Z_0-9]*):$')
      if (tag ~= nil) then
        return feed_tag (tag)
      end

      local name, left = stat:match ('^(%.[a-z]+)(.*)$')
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
    local output = opt:getopt ('0')
    local unit = units.new ()

    for _, file in ipairs (files) do
      if (file == '-') then
        feed (unit, '(stdin)')
      else
        io.input (assert (io.open (file, 'r')))
        feed (unit, file)
      end
    end
  end

  main (opt:parse (...))
end
