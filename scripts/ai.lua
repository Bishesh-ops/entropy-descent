function GetAggroRadius(entropy_level)
    local base_radius = 15

    if entropy_level >= 86 then
        return 30
    elseif entropy_level >= 61 then
        return 22
    end

    return base_radius
end
