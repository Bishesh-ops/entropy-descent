import json
import sys
import os

def validate_enemies(filepath):
    if not os.path.exists(filepath):
        print(f"FATAL ERROR: {filepath} not found.")
        return False

    with open(filepath, "r") as f:
        try:
            data = json.load(f)
        except json.JSONDecodeError as e:
            print(f"FATAL ERROR: Invalid JSON syntax in {filepath}: {e}")
            return False

    required_keys = ["id", "name", "components"]
    required_components = ["Health", "CombatStats", "Velocity"]
    ids = set()

    for i, enemy in enumerate(data):
        for key in required_keys:
            if key not in enemy:
                print(f"ERROR: Enemy index {i} missing root key '{key}'.")
                return False
        
        eid = enemy["id"]
        if eid in ids:
            print(f"ERROR: Duplicate ID found: '{eid}'")
            return False
        ids.add(eid)
        
        comps = enemy["components"]
        for req_comp in required_components:
            if req_comp not in comps:
                print(f"ERROR: Enemy '{eid}' missing required component '{req_comp}'.")
                return False

    print(f"SUCCESS: {filepath} passed all validation checks. Safe for C++ parsing.")
    return True

if __name__ == "__main__":
    if not validate_enemies("../data/enemies.json"):
        sys.exit(1)