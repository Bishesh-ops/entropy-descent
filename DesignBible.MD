## Entropy Descent — Design Bible v0.1

Core Identity
Entropy Descent is a game about controlled collapse. You don't fight the entropy — you negotiate with it. Every vow, every spell, every action feeds the decay. The player who survives longest isn't the most powerful one — it's the one who understood what they were trading away.

The Core Loop
Enter floor
→ Entropy ticks up passively, bleeds down slowly when playing carefully
→ Player negotiates: melee (safe, slow) vs spells (powerful, costly)
→ Floor entropy quota must be met → stairs appear
→ Entropy overflow → vow offer screen, not just an elite spawn
→ Descend, world looks worse, enemies hit harder
→ Run ends when entropy becomes unmanageable

Entropy — The Central Mechanic
Entropy is oxygen. Too little and you can't progress. Too much and the world falls apart. Players are always uncomfortable, always managing it.
Gain sources:

Passive tick, always rising slowly
Spells (+25-30 per cast, making them genuinely scarce decisions)
Getting hit by Entropic enemies
Descending floors

Loss sources:

Slow passive bleed — every 30 ticks of no spells cast and no kills, entropy drops by 1
Certain vows can accelerate this as a tradeoff

Entropy thresholds — the world reacts:
0-25% → World looks normal. Nothing feels wrong yet.
26-50% → Floor tiles flicker with corruption tint. Enemies slightly faster. Faint ambient particle drift begins. HUD bar pulses softly.
51-75% → Walls visually crumble at edges. Enemy attack ticks up. Random tiles briefly flash corrupted states. Subtle screen vignette appears. Bar actively glows.
76-99% → Floor tiles blink out for frames. Micro camera tremors on every move. Enemies leave particle trails. Bar is strobing. World feels genuinely wrong.
100% → Overflow. Vow offer fires. Entropy resets. One permanent visual corruption stays on the map permanently — stacks across multiple overflows.

Spells — Scarce and Meaningful
Because spells cost 25-30 entropy, and the bar is 100 deep, you get roughly 3-4 casts before hitting dangerous territory. That makes every spell a real decision, not a spam button. Melee becomes the bread and butter. Spells become the punctuation.

Entropic Enemies — Mechanically Distinct
Regular enemies deal HP damage. Entropic variants deal HP damage AND add entropy on hit. They're not just stat-boosted — they're a different threat category. Fighting them safely is about controlling the engagement, not just having enough health.

Vows — Bargains With the Decay
Offered at entropy overflow, not chosen from a menu at the start. Entropy decides what it offers you — two options, pick one, no take-backs. Each vow has three parts: a name, an effect, and a single whisper line of flavor text.
Design lens for every vow: what does the player surrender to the entropy, and what does the entropy give back?
Example vow directions:

Vow of the Crumbling Form — lose 1 max HP every 20 ticks, gain attack for every point lost. Trade your future for power now.
Vow of Static — can't move more than 5 tiles from where you took the vow. Enemies in radius take passive entropy damage. You become an immovable corrupting field.
Vow of the Second Law — overflow events permanently boost spell damage instead of corrupting the map. You start wanting entropy to overflow.
Vow of Borrowed Time — double starting health, entropy ticks twice as fast. The clock is genuinely running out.
Vow of the Still Mind — passive entropy decay rate doubles, spells disabled entirely. Safe but defanged.

Each vow lives in scripts/vows.lua with on_activate and on_tick callbacks, same pattern as spells.

Visual Representation of Decay
Three feedback layers working together:
The world — threshold-based tile corruption using the existing color_val system, tile state flickers, wall edge crumbling (cosmetic)
The player — faint purple particle aura at stage 2, dense and alarming at stage 3, using the existing particle system
The HUD bar — breathes with a sine wave on brightness. Calm at low entropy, strobing at critical. Not a static rectangle anymore.

Floor Structure
Each floor has an entropy quota — a minimum amount of entropy that must be generated before the stairs appear. This inverts the usual roguelike tension. You need entropy to rise, but not so fast you lose control. Deeper floors start visually more corrupted, have more Entropic enemy variants, and have higher quotas.

Immediate Build Order

Entropy threshold proc + tile visual corruption layer
HUD bar pulse behavior
Player particle aura at high entropy
Wire entropy overflow → vow offer screen
Vow system in Lua with on_activate / on_tick
JSON enemy loading — retire the hardcoded roster
Floor entropy quota + staircase tile
Items renamed as fragments with flavor-appropriate effects
Entropic enemies deal entropy damage on hit
