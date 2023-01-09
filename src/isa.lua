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
local utils = require ('utils')
local isa = {}

--
-- S-MIPS O32 calling convention
-- +---------+---------+-----+---------------------+
-- | Name    | Number  |  P  | Usage               |
-- +---------+---------+-----+---------------------+
-- | $zero   | r0      |  -  | Constant 0          |
-- +---------+---------+-----+---------------------+
-- | $at     | r1      |  -  | Assembler temporary |
-- +---------+---------+-----+---------------------+
-- | $v0-$v1 | r2-r3   |  N  | Function returns    |
-- +---------+---------+-----+---------------------+
-- | $a0-$a3 | r4-r7   |  N  | Function arguments  |
-- +---------+---------+-----+---------------------+
-- | $t0-$t7 | r8-r15  |  N  | Temporaries         |
-- +---------+---------+-----+---------------------+
-- | $s0-$s7 | r16-r23 |  Y  | Saved temporaries   |
-- +---------+---------+-----+---------------------+
-- | $t8-$t9 | r24-r25 |  N  | Temporaries         |
-- +---------+---------+-----+---------------------+
-- | $k0-$k1 | r26-r27 |  -  | Reserved for OS     |
-- +---------+---------+-----+---------------------+
-- | $gp     | r28     |  Y  | Global pointer      |
-- +---------+---------+-----+---------------------+
-- | $ra     | r29     |  Y  | Return address      |
-- +---------+---------+-----+---------------------+
-- | $fp     | r30     |  Y  | Frame pointer       |
-- +---------+---------+-----+---------------------+
-- | $sp     | r31     |  Y  | Stack pointer       |
-- +---------+---------+-----+---------------------+
--
-- - Procedures with more than four arguments they are pushed
--   onto the stack in normal order (fifth arguments is pushed
--   first, then sixth)
-- - Argument registers $a0-$a3 may not be saved by the caller
-- - *P stands for 'preserve', it indicated whether the callee
--   must preserve register contents
--

do
  isa.r_insts =
  {
    add = { opcode = 0, func = 32, takes = { rd = true, rt = true, rs = true, }, },
    ['and'] = { opcode = 0, func = 36, takes = { rd = true, rt = true, rs = true, }, },
    div = { opcode = 0, func = 26, takes = { rd = false, rt = true, rs = true, }, },
    divu = { opcode = 0, func = 27, takes = { rd = false, rt = true, rs = true, }, },
    jr = { opcode = 0, func = 8, takes = { rd = false, rt = false, rs = true, }, },
    mfhi = { opcode = 0, func = 16, takes = { rd = true, rt = false, rs = false, }, },
    mflo = { opcode = 0, func = 18, takes = { rd = true, rt = false, rs = false, }, },
    mul = { opcode = 0, func = 24, takes = { rd = false, rt = true, rs = true, }, },
    mulu = { opcode = 0, func = 25, takes = { rd = false, rt = true, rs = true, }, },
    nop = { opcode = 0, func = 0, takes = { rd = false, rt = false, rs = false, }, },
    ['nor'] = { opcode = 0, func = 39, takes = { rd = true, rt = true, rs = true, }, },
    ['or'] = { opcode = 0, func = 37, takes = { rd = true, rt = true, rs = true, }, },
    slt = { opcode = 0, func = 42, takes = { rd = true, rt = true, rs = true, }, },
    sltu = { opcode = 0, func = 43, takes = { rd = true, rt = true, rs = true, }, },
    sub = { opcode = 0, func = 34, takes = { rd = true, rt = true, rs = true, }, },
    ['xor'] = { opcode = 0, func = 40, takes = { rd = true, rt = true, rs = true, }, },

    -- SMIPS specific instructions
    halt = { opcode = 63, func = 63, takes = { rd = false, rt = false, rs = false, }, },
    kbd = { opcode = 63, func = 4, takes = { rd = true, rt = false, rs = false, }, },
    pop = { opcode = 56, func = 0, takes = { rd = true, rt = false, rs = false, }, },
    push = { opcode = 56, func = 1, takes = { rd = false, rt = false, rs = true, }, },
    rnd = { opcode = 63, func = 2, takes = { rd = true, rt = false, rs = false, }, },
    tty = { opcode = 63, func = 1, takes = { rd = false, rt = false, rs = true, }, },

    -- Patch to SMIPS
    sll = { opcode = 0, func = 0, takes = { rd = true, rt = true, shamt = true, }, },
    sllv = { opcode = 0, func = 4, takes = { rd = true, rt = true, rs = true, }, },
    srl = { opcode = 0, func = 2, takes = { rd = true, rt = true, shamt = true, }, },
    srlv = { opcode = 0, func = 6, takes = { rd = true, rt = true, rs = true, }, },
  }

  isa.i_insts =
  {
    addi = { opcode = 8, takes = { rt = true, rs = true, cs = true, }, },
    andi = { opcode = 12, takes = { rt = true, rs = true, cs = true, }, },
    beq = { opcode = 4, takes = { rt = true, rs = true, cs = true, rs_first = true, }, tagable = 'r', },
    bne = { opcode = 5, takes = { rt = true, rs = true, cs = true, rs_first = true, }, tagable = 'r', },
    lw = { opcode = 35, takes = { rt = true, rs = false, cs = true, }, address = 'e', },
    ori = { opcode = 13, takes = { rt = true, rs = true, cs = true, }, },
    slti = { opcode = 10, takes = { rt = true, rs = true, cs = true, }, },
    sltiu = { opcode = 11, takes = { rt = true, rs = true, cs = true, }, },
    sw = { opcode = 43, takes = { rt = true, rs = false, cs = true, }, address = 'e', },
    xori = { opcode = 14, takes = { rt = true, rs = true, cs = true, }, },

    -- SMIPS specific instructions
    bgtz = { opcode = 7, takes = { rt = false, rs = true, cs = true, }, tagable = 'r', },
    blez = { opcode = 6, takes = { rt = false, rs = true, cs = true, }, tagable = 'r', },
    bltz = { opcode = 1, takes = { rt = false, rs = true, cs = true, }, tagable = 'r', },

    -- Hacks
    la = { opcode = 8, takes = { rt = true, cs = true, }, tagable = 'a', address = 'l', },
    li = { opcode = 8, takes = { rt = true, cs = true, }, },
    move = { opcode = 8, takes = { rt = true, rs = true, }, },
  }

  isa.j_insts =
  {
    j = { opcode = 2, takes = { cs = true, }, tagable = 'j', },
  }

  isa.defaults = { rd = 0, rs = 0, rt = 0, shamt = '0', cs = '0', }
end

do
  isa.i_directives = {}
  isa.a_directives = {}
end

do
  isa.l_directives =
  {
    ascii = function (arg, unit, compe)
      isa.l_directives.byte (arg, unit, compe)
    end,

    asciiz = function (arg, unit, compe)
      local ent

      isa.l_directives.byte (arg, unit, compe)
      ent = unit:last ()
      ent.data = ent.data .. string.char (0)
    end,

    byte = function (arg, unit, compe)
      local env = {}
      local expr = ('do return %s; end'):format (arg)
      local chunk, reason = load (expr, '=directive', 't', env)

      if (not chunk) then
        compe ('Invalid directive argument (\'%s\')', reason)
      else
        local data = (chunk ())
        if (type (data) == 'string') then
          unit:add_data (data)
        elseif (type (data) == 'number') then
          if (data < 0 or data > 255) then
            compe ('Number %i is too big for byte data', data)
          else
            local byte = string.char (data)
            unit:add_data (byte)
          end
        else
          compe ('Directive argument should be a constant byte string')
        end
      end
    end,

    space = function (arg, unit, compe)
      local env = {}
      local expr = ('do return %s; end'):format (arg)
      local chunk, reason = load (expr, '=directive', 't', env)

      if (not chunk) then
        return true, ('Invalid directive argument (\'%s\')'):format (reason)
      else
        local size = (chunk ())
        if (type (size) == 'number') then
          unit:add_data (size)
        else
          return true, ('Directive argument should be a constant number')
        end
      end
    end,

    half = function (arg, unit, compe)
      local env = {}
      local expr = ('do return %s; end'):format (arg)
      local chunk, reason = load (expr, '=directive', 't', env)

      if (not chunk) then
        compe ('Invalid directive argument (\'%s\')', reason)
      else
        local word = (chunk ())
        if (type (word) ~= 'number') then
          compe ('Directive argument should be a constant number')
        else
          local data = utils.half2buf (word)
          unit:add_data (data)
        end
      end
    end,

    word = function (arg, unit, compe)
      local env = {}
      local expr = ('do return %s; end'):format (arg)
      local chunk, reason = load (expr, '=directive', 't', env)

      if (not chunk) then
        compe ('Invalid directive argument (\'%s\')', reason)
      else
        local word = (chunk ())
        if (type (word) ~= 'number') then
          compe ('Directive argument should be a constant number')
        else
          local data = utils.word2buf (word)
          unit:add_data (data)
        end
      end
    end,
  }
end

do
  local regs =
  {
    zero = 0,
    at = 1,
    gp = 28,
    ra = 29,
    fp = 30,
    sp = 31,
  }

  local function getbase (bases, n)
    local base = bases [n]
    if (base == nil) then
      return getbase (bases, n - 1)
    end
  return base, n
  end

  local function checkn (lim, n)
    if (n < lim.limit) then
      local bases = lim.bases
      local base, at = getbase (bases, n)
      return base + (n - at)
    end
  end

  local lims =
  {
    a = { limit =  4, bases = { [0] =  4,           }, }, -- a0-a3; a0 =  4
    k = { limit =  2, bases = { [0] = 26,           }, }, -- k0-k1; k0 = 26
    s = { limit =  8, bases = { [0] = 16,           }, }, -- s0-s7; s0 = 16
    t = { limit = 10, bases = { [0] =  8, [8] = 24, }, }, -- t0-t9; t0 =  8, t8 = 24
    v = { limit =  2, bases = { [0] =  2,           }, }, -- v0-v1; v0 =  2
  }

  local mt =
  {
    __index = function (_, key)
      if (not key:find ('[a-z]')) then
        local val = tonumber (key)
        if (val >= 0 and val < 32) then
          return val
        end
      else
        local t, n = key:match ('^([a-z])([0-9]+)$')
        if (t and lims [t]) then
          local lim = lims [t]
          local val = tonumber (n)
          return checkn (lim, val)
        end
      end
    end
  }

  isa.regs = setmetatable (regs, mt)
end

return isa
