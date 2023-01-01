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
    addi = { opcode = 8, takes = { rt = true, rs = true, }, },
    slti = { opcode = 10, takes = { rt = true, rs = true, }, },
    sltiu = { opcode = 11, takes = { rt = true, rs = true, }, },
    andi = { opcode = 12, takes = { rt = true, rs = true, }, },
    ori = { opcode = 13, takes = { rt = true, rs = true, }, },
    xori = { opcode = 14, takes = { rt = true, rs = true, }, },
    lw = { opcode = 35, takes = { rt = true, rs = false, }, address = true, },
    sw = { opcode = 43, takes = { rt = true, rs = false, }, address = true, },
    beq = { opcode = 4, takes = { rt = true, rs = true, }, tagable = 'r', },
    bne = { opcode = 5, takes = { rt = true, rs = true, }, tagable = 'r', },
    blez = { opcode = 6, takes = { rt = false, rs = true, }, tagable = 'r', },
    bgtz = { opcode = 7, takes = { rt = false, rs = true, }, tagable = 'r', },
    bltz = { opcode = 1, takes = { rt = false, rs = true, }, tagable = 'r', },
  }

  local j_insts =
  {
    j = { opcode = 2, tagable = 'a', },
  }

  local macros =
  {
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

    local function put_rinst (desc, rt, rs, rd)
      local inst
      inst = insts.new (desc.opcode):typer ()
      inst.rt = rt
      inst.rs = rs
      inst.rd = rd
      unit.add_inst (inst)
    end

    local function put_iinst (desc, rt, rs, cs)
      local inst
      inst = insts.new (desc.opcode):typei ()
      inst.rt = rt
      inst.rs = rs
      inst.cs = cs
      unit.add_inst (inst)
    end

    local function put_jinst (desc, cs)
      local inst
      inst = insts.new (desc.opcode):typej ()
      inst.cs = cs
      unit.add_inst (inst)
    end

    local function feed_tag (tagname)
      unit:add_tag (tagname)
    end

    local function feed_inst (inst, ...)
      local nexti = 0

      local function getnext (...)
        nexti = nexti + 1
        local arg = select (nexti, ...)
        if (arg) then
          arg = arg:gsub ('^%s*', '')
          arg = arg:gsub ('%s*$', '')
          return arg
        end
      end

      if (macros [inst] ~= nil) then
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
        local cs = assertcs (getnext (...))
        put_iinst (desc, rt, rs, cs)
      elseif (j_insts [inst] ~= nil) then
        local desc = j_insts [inst]
        local cs = assertcs (getnext (...))
        put_jinst (desc, cs)
      end
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
    local unit = units.new ()

    for _, file in ipairs (files) do
      io.input (assert (io.open (file, 'r')))
      feed (unit)
    end
  end

  main (opt:parse (...))
end
