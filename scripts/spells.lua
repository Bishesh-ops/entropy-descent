Spells = {}

local function GetDirection(startX, startY, targetX, targetY)
    local dx = targetX - startX
    local dy = targetY - startY
    local len = math.sqrt(dx * dx + dy * dy)
    if len > 0 then
        return dx / len, dy / len
    end
    return 1, 0
end

function Spells.Cast(spell_type, startX, startY, targetX, targetY, entropy_bonus, base_attack)
    local dx, dy = GetDirection(startX, startY, targetX, targetY)

    if spell_type == "MELEE" then
        SpawnProjectile(startX + (dx * 15), startY + (dy * 15), dx, dy, 50.0, base_attack, 0.1, "PHYSICAL", 255, 255, 255)
        SpawnParticles(startX + (dx * 20), startY + (dy * 20), 5, 200, 200, 200)
    elseif spell_type == "FIRE" then
        local dmg = 15 + (entropy_bonus * 2)
        SpawnProjectile(startX, startY, dx, dy, 250.0, dmg, 2.0, "FIRE", 255, 80, 0)
    elseif spell_type == "CRYO" then
        local dmg = 10 + entropy_bonus
        SpawnProjectile(startX, startY, dx, dy, 180.0, dmg, 1.5, "CRYO", 50, 200, 255)
    end
end
