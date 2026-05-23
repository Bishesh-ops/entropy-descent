// sys_lua.odin
package main

import "base:runtime"
import "core:c"
import "core:fmt"
import "core:text/regex/parser"
import lua "vendor:lua/5.4"

init_lua :: proc(gs: ^Game_State) {
	L := lua.L_newstate()
	lua.L_openlibs(L)
	gs.lua_state = L

	lua.newtable(L)

	register_func :: proc(L: ^lua.State, name: cstring, fn: lua.CFunction, gs: ^Game_State) {
		lua.pushlightuserdata(L, gs) // Push Game_State pointer
		lua.pushcclosure(L, fn, 1) // Create closure with 1 upvalue
		lua.setfield(L, -2, name) // Add to our table
	}

	register_func(L, "is_occupied", l_is_occupied, gs)
	register_func(L, "spawn_particles", l_spawn_particles, gs)
	register_func(L, "deal_damage", l_deal_damage, gs)
	register_func(L, "is_floor", l_is_floor, gs)
	register_func(L, "shake_screen", l_shake_screen, gs)
	register_func(L, "get_position", l_get_position, gs)
	register_func(L, "set_position", l_set_position, gs)
	register_func(L, "log_message", l_log_message, gs)
	register_func(L, "set_tile_state", l_set_tile_state, gs)

	lua.setglobal(L, "game_api")

	if lua.L_dofile(L, "scripts/spells.lua") != 0 {
		fmt.eprintln("Lua Error Loading Spells:", lua.tostring(L, -1))
		lua.pop(L, 1)
	}
}

l_is_occupied :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	x := int(lua.tointeger(L, 1))
	y := int(lua.tointeger(L, 2))
	caster := int(lua.tointeger(L, 3))

	occupied := false
	for id in 0 ..< len(gs.world.entities) {
		if id == caster do continue
		if !gs.world.entities[id].active do continue
		if .Position not_in gs.world.entities[id].components do continue
		if gs.world.entities[id].position.x == x && gs.world.entities[id].position.y == y {
			occupied = true
			break
		}
	}
	lua.pushboolean(L, b32(occupied))
	return 1
}


l_spawn_particles :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	x := int(lua.tointeger(L, 1))
	y := int(lua.tointeger(L, 2))
	r := u8(lua.tointeger(L, 3))
	g := u8(lua.tointeger(L, 4))
	b := u8(lua.tointeger(L, 5))
	count := int(lua.tointeger(L, 6))

	context = runtime.default_context()
	spawn_particle_burst(gs, x, y, {r, g, b, 255}, count)
	return 0
}

l_deal_damage :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	x := int(lua.tointeger(L, 1))
	y := int(lua.tointeger(L, 2))
	dmg := int(lua.tointeger(L, 3))
	caster_id := int(lua.tointeger(L, 4))

	for id in 0 ..< len(gs.world.entities) {
		if !gs.world.entities[id].active do continue
		if id == caster_id do continue

		if .Position not_in gs.world.entities[id].components do continue

		if gs.world.entities[id].position.x == x && gs.world.entities[id].position.y == y {
			if .Health in gs.world.entities[id].components {
				gs.world.entities[id].health -= dmg
				if gs.world.entities[id].health <= 0 {
					gs.world.entities[id].components += {.Pending_Destroy}
				}
			}
		}
	}
	return 0
}
l_shake_screen :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	amount := f32(lua.tonumber(L, 1))
	gs.screen_shake += amount
	return 0
}

l_is_floor :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	x := int(lua.tointeger(L, 1))
	y := int(lua.tointeger(L, 2))

	is_floor := false
	if x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT {
		is_floor = (gs.game_map.tiles[x][y].type == .Floor)
	}

	lua.pushboolean(L, b32(is_floor))
	return 1 // Returning 1 value to Lua
}

l_get_position :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	id := int(lua.tointeger(L, 1))

	x := gs.world.entities[id].position.x
	y := gs.world.entities[id].position.y

	lua.pushinteger(L, lua.Integer(x))
	lua.pushinteger(L, lua.Integer(y))
	return 2 // Returning 2 values!
}

l_set_position :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	id := int(lua.tointeger(L, 1))
	x := int(lua.tointeger(L, 2))
	y := int(lua.tointeger(L, 3))

	gs.world.entities[id].position = {x, y}
	gs.world.entities[id].transform = {f32(x * TILE_SIZE), f32(y * TILE_SIZE)}
	return 0
}

l_log_message :: proc "c" (L: ^lua.State) -> c.int {
	context = runtime.default_context()

	msg := lua.tostring(L, 1)
	fmt.println("[LUA]: ", msg)
	return 0
}

l_set_tile_state :: proc "c" (L: ^lua.State) -> c.int {
	gs := cast(^Game_State)lua.touserdata(L, lua.REGISTRYINDEX - 1)
	x := int(lua.tointeger(L, 1))
	y := int(lua.tointeger(L, 2))
	state_str := lua.tostring(L, 3)

	if x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT {
		if state_str == "Scorched" {
			gs.game_map.tiles[x][y].state = .Scorched
		} else if state_str == "Frozen" {
			gs.game_map.tiles[x][y].state = .Frozen
		} else if state_str == "Charged" {
			gs.game_map.tiles[x][y].state = .Charged
		} else {
			gs.game_map.tiles[x][y].state = .Neutral
		}
	}
	return 0
}

cast_spell :: proc(gs: ^Game_State, spell_id: string, caster_id: int, tx, ty: int) -> bool {
	L := gs.lua_state
	lua.getglobal(L, "SpellRegistry")
	if !lua.istable(L, -1) {
		fmt.eprintln("SpellRegistry not found!")
		lua.pop(L, 1)
		return false
	}

	lua.getfield(L, -1, cstring(raw_data(spell_id)))
	if !lua.istable(L, -1) {
		fmt.eprintln("Spell", spell_id, "not found!")
		lua.pop(L, 2)
		return false
	}

	lua.getfield(L, -1, "cost")
	spell_cost := int(lua.tointeger(L, -1))
	lua.pop(L, 1)
	gs.entropy.entropy = min(gs.entropy.entropy + spell_cost, gs.entropy.max_entropy)

	lua.getfield(L, -1, "on_cast")
	if !lua.isfunction(L, -1) {
		fmt.eprintln("on_cast is not a function!")
		lua.pop(L, 3)
		return false
	}

	lua.pushinteger(L, lua.Integer(caster_id))
	lua.pushinteger(L, lua.Integer(tx))
	lua.pushinteger(L, lua.Integer(ty))

	if lua.pcall(L, 3, 1, 0) != 0 {
		fmt.eprintln("Lua Error Executing Spell:", lua.tostring(L, -1))
		lua.pop(L, 3)
		return false
	}
	success := bool(lua.toboolean(L, -1))
	lua.pop(L, 3)
	return success
}

