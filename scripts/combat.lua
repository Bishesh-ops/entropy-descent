function CalculateMeleeDamage(attacker_attack, target_defense)
    local damage = attacker_attack - target_defense

    if damage < 1 then
        damage = 1
    end

    return damage
end
