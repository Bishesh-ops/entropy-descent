import json
import random
import os

os.makedirs("../data", exist_ok=True)

bases = {
    "Goblin": {"hp": 20, "attack": 5, "speed": 100},
    "Skeleton": {"hp": 15, "attack": 8, "speed": 80},
    "Phantom": {"hp": 10, "attack": 12, "speed": 150},
    "Orc": {"hp": 40, "attack": 12, "speed": 90}
}

affixes = {
    "Fierce": {"hp_mult": 1.0, "attack_mult": 1.5, "speed_mult": 1.2},
    "Armored": {"hp_mult": 2.0, "attack_mult": 0.8, "speed_mult": 0.8},
    "Nimble": {"hp_mult": 0.8, "attack_mult": 1.0, "speed_mult": 1.5},
    "Entropic": {"hp_mult": 1.5, "attack_mult": 1.5, "speed_mult": 1.0}
}

def generate_database(count=20):
    db = []
    for i in range(count):
        base_name, base_stats = random.choice(list(bases.items()))
        
        if random.random() < 0.3:
            affix_name, affix_stats = random.choice(list(affixes.items()))
            name = f"{affix_name} {base_name}"
            hp = int(base_stats["hp"] * affix_stats["hp_mult"])
            atk = int(base_stats["attack"] * affix_stats["attack_mult"])
            spd = float(base_stats["speed"] * affix_stats["speed_mult"])
        else:
            name = base_name
            hp = base_stats["hp"]
            atk = base_stats["attack"]
            spd = float(base_stats["speed"])

        db.append({
            "id": f"enemy_{i:03d}",
            "name": name,
            "components": {
                "Health": {"max": hp, "current": hp},
                "CombatStats": {"attack": atk},
                "Velocity": {"dx": 0.0, "dy": 0.0, "speed": spd}
            }
        })
    return db

if __name__ == "__main__":
    enemies = generate_database(30)
    filepath = "../data/enemies.json"
    with open(filepath, "w") as f:
        json.dump(enemies, f, indent=4)
    print(f"SUCCESS: Generated {len(enemies)} enemies into {filepath}")