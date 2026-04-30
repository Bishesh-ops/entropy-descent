function GetAggroRadius(entropy_level, floor_depth)
    local base_radius = 15 + (floor_depth - 1) * 2

    if entropy_level >= 86 then
        return base_radius + 15
    elseif entropy_level >= 61 then
        return base_radius + 7
    end

    return math.floor(base_radius)
end
