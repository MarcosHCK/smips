local function main (...)
  for k, v in ipairs ({...}) do
    print (k, v)
  end
end
main (...)
