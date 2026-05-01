Spells = {}

local function GetDirection(startX, startY, targetX, targetY)
    local dx = targetX - startX
    local dy = targetY - startY
    local len = math.sqrt(dx * dx + dy * dy)
    if len > 0 then return dx / len, dy / len end
    return 1, 0
end

function Spells.Cast(spell_type, startX, startY, targetX, targetY, entropy_bonus, base_attack)
    local dx, dy = GetDirection(startX, startY, targetX, targetY)

    if spell_type == "MELEE" then
        SpawnProjectile(startX + (dx * 15), startY + (dy * 15), dx, dy, 50.0, base_attack, 0.1, "PHYSICAL", 255, 255, 255,
            16.0, 16.0)
        SpawnParticles(startX + (dx * 20), startY + (dy * 20), 5, 200, 200, 200)
    elseif spell_type == "FIRE" then
        local charge = ConsumeTiles(startX, startY, 2, "FIRE")

        if charge <= 1 then
            local dmg = 15 + entropy_bonus
            SpawnProjectile(startX, startY, dx, dy, 300.0, dmg, 1.5, "FIRE", 255, 100, 0, 16.0, 16.0)
            ApplyDrawback(5, 0)
            SpawnParticles(startX, startY, 5, 255, 100, 0)
        elseif charge >= 2 and charge <= 4 then
            local dmg = 35 + (entropy_bonus * 2)
            local spawnX = targetX - 30.0
            local spawnY = targetY - 30.0

            SpawnProjectile(spawnX, spawnY, 0, 0, 0.0, dmg, 0.2, "FIRE", 255, 50, 0, 60.0, 60.0)
            ApplyDrawback(15, 0)
            SpawnParticles(targetX, targetY, 30, 255, 50, 0)
        elseif charge >= 5 then
            local dmg = 80 + (entropy_bonus * 4)
            local spawnX = targetX - 60.0
            local spawnY = targetY - 60.0

            SpawnProjectile(spawnX, spawnY, 0, 0, 0.0, dmg, 0.4, "FIRE", 255, 0, 0, 120.0, 120.0)
            ApplyDrawback(35, 10)

            SpawnParticles(targetX, targetY, 80, 255, 20, 0)
            SpawnParticles(targetX, targetY, 40, 50, 50, 50)
        end
    elseif spell_type == "CRYO" then
        local charge = ConsumeTiles(startX, startY, 2, "WATER")
        local baseAngle = math.atan(dy, dx)

        if charge <= 1 then
            local spread = 0.25
            for i = -1, 1 do
                local angle = baseAngle + (i * spread)
                local pDx = math.cos(angle)
                local pDy = math.sin(angle)
                SpawnProjectile(startX, startY, pDx, pDy, 200.0, 15 + entropy_bonus, 1.5, "CRYO", 50, 200, 255, 12.0,
                    12.0)
            end
            ApplyDrawback(5, 0)
            SpawnParticles(startX + (dx * 10), startY + (dy * 10), 10, 150, 255, 255)
        elseif charge >= 2 and charge <= 4 then
            local spread = 0.20
            for i = -2, 2 do
                local angle = baseAngle + (i * spread)
                local pDx = math.cos(angle)
                local pDy = math.sin(angle)
                SpawnProjectile(startX, startY, pDx, pDy, 250.0, 25 + (entropy_bonus * 2), 1.5, "CRYO", 50, 220, 255,
                    16.0, 16.0)
            end
            ApplyDrawback(15, 0)
            SpawnParticles(startX + (dx * 15), startY + (dy * 15), 20, 100, 255, 255)
        elseif charge >= 5 then
            local num_projectiles = 12
            for i = 1, num_projectiles do
                local angle = (i / num_projectiles) * (2 * math.pi)
                local pDx = math.cos(angle)
                local pDy = math.sin(angle)
                SpawnProjectile(startX, startY, pDx, pDy, 150.0, 20 + entropy_bonus, 1.0, "CRYO", 100, 255, 255, 16.0,
                    16.0)
            end

            SpawnProjectile(startX, startY, dx, dy, 400.0, 60 + (entropy_bonus * 4), 2.0, "CRYO", 0, 150, 255, 40.0, 40.0)

            ApplyDrawback(35, 10)
            SpawnParticles(startX, startY, 60, 50, 200, 255)
        end
    end
end
