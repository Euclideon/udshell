
function tprint (val, indent, fmt)
  if not indent then indent = 0 end
  if type(val) == 'table' then
    for k, v in pairs(val) do
      formatting = string.rep('  ', indent) .. k .. ': '
      tprint(v, indent+1, formatting)
    end
  else
    print((fmt or '') .. tostring(val))
  end
end
