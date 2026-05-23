SpellRegistry = {}

SpellRegistry["scorch_earth"] = {
	name = "Scorch Earth",
	description = "Deals 15 damage in a 3x3 grid and scorches the tiles.",
	cost = 35,
	target_type = "AOE",
	radius = 1,

	on_cast = function(caster_id, target_x, target_y)
		game_api.shake_screen(0.4)
		for dx = -1, 1 do
			for dy = -1, 1 do
				local tx = target_x + dx
				local ty = target_y + dy
				game_api.spawn_particles(tx, ty, 255, 80, 20, 10)
				game_api.deal_damage(tx, ty, 15, caster_id)
				game_api.set_tile_state(tx, ty, "Scorched")
			end
		end
		return true
	end,
}

SpellRegistry["void_step"] = {
	name = "Void Step",
	description = "Teleport to a visible empty floor tile.",
	cost = 25,
	target_type = "Point",
	radius = 0,

	on_cast = function(caster_id, target_x, target_y)
		if not game_api.is_floor(target_x, target_y) then
			game_api.log_message("Target is blocked!")
			return false
		end
		if game_api.is_occupied(target_x, target_y, caster_id) then
			game_api.log_message("Something is already there!")
			return false
		end
		local cx, cy = game_api.get_position(caster_id)
		game_api.spawn_particles(cx, cy, 100, 0, 200, 15)
		game_api.set_position(caster_id, target_x, target_y)
		game_api.spawn_particles(target_x, target_y, 200, 0, 255, 25)
		game_api.shake_screen(0.2)
		return true
	end,
}
