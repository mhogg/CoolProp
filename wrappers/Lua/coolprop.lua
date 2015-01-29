if jit and jit.status() then
    return require "coolprop.ffi"
end

local huge = math.huge

local ok, lib = pcall(require, "coolprop.capi")
assert(ok, "Unable to load CoolProp. Please check that the CoolProp is available. Both the Lua C API wrapper, and the CoolProp shared library.")
local coolprop = {}
function coolprop.err()
    return coolprop.get_global_param_string("errstring")
end
function coolprop.Props1SI(fluidname, output)
    local v = lib.Props1SI(fluidname, output)
    if v == huge then
        return nil, coolprop.err()
    end
    return v
end
function coolprop.PropsSI(output, name1, prop1, name2, prop2, ref)
    local v = lib.PropsSI(output, name1, prop1, name2, prop2, ref)
    if v == huge then
        return nil, coolprop.err()
    end
    return v
end
function coolprop.PhaseSI(name1, prop1, name2, prop2, ref)
    local e, s = lib.PhaseSI(name1, prop1, name2, prop2, ref or "")
    return e == 1 and s or nil
end
function coolprop.get_global_param_string(param)
    local e, s = lib.get_global_param_string(param)
    if e == 1 then
        return s
    end
    return nil
end
function coolprop.get_parameter_information_string(key)
    local e, s = lib.get_parameter_information_string(key)
    return e == 1 and s or nil
end
function coolprop.get_mixture_binary_pair_data(cas1, cas2, key)
    return tonumber(lib.get_mixture_binary_pair_data(cas1, cas2, key))
end
function coolprop.get_fluid_param_string(fluid, param)
    if lib.get_fluid_param_string(fluid, param, bub, 4096) == 1 then
        return ffi_str(bub)
    end
    return nil
end
function coolprop.set_reference_stateS(ref, state)
    return lib.set_reference_stateS(ref, state) == 1
end
function coolprop.set_reference_stateD(ref, t, rho, h0, s0)
    return lib.set_reference_stateD(ref, t, rho, h0, s0) == 1
end
function coolprop.F2K(f)
    return lib.F2K(f)
end
function coolprop.K2F(k)
    return lib.K2F(k)
end
function coolprop.get_param_index(param)
    return lib.get_param_index(param)
end
function coolprop.saturation_ancillary(fluid, output, q, input, value)
    return lib.saturation_ancillary(fluid, output, q, input, value)
end
function coolprop.redirect_stdout(file)
    return lib.redirect_stdout(file) == 1
end
function coolprop.get_debug_level()
    return lib.get_debug_level()
end
function coolprop.set_debug_level(level)
    lib.set_debug_level(level)
end
function coolprop.HAPropsSI(output, name1, prop1, name2, prop2, name3, prop3)
    local v = lib.HAPropsSI(output, name1, prop1, name2, prop2, name3, prop3)
    if v == huge then
        return nil, coolprop.err()
    end
    return v
end
return coolprop