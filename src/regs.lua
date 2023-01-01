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

return setmetatable (regs, mt)
end
