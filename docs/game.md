# Game and gameplay

R-Type is a cooperative horizontal shooter. Players pilot ships across an **800 × 600** playfield, dodge enemies and incoming shots, and destroy enemies to increase a shared score. The current game is an ongoing survival challenge: there is no final level, boss, victory condition, or game-over screen yet.

## Playing

Each player starts near the left side of the playfield at **(60, 300)** with **3 health**. A ship can move up, down, left, and right. Its movement is limited to the playfield, so it cannot fly beyond the screen edges. Opposite directions cancel each other out.

Players fire to the right. Holding fire shoots repeatedly, with at least **0.3 seconds** between shots. A player projectile disappears when it hits an enemy or leaves the playfield. Multiple players can fight at the same time and contribute to the same score.

## Enemies and hazards

An enemy appears at the right side roughly **once per second**, at a varying height, and travels left. Each enemy has **1 health**. Roughly **every 1.5 seconds**, each active enemy fires a projectile to the left. Enemies and their shots disappear after leaving the playfield.

A player shot destroys an enemy on contact and awards **100 points**. An enemy shot costs the player **1 health**. Touching an enemy also costs **1 health** and destroys that enemy, without awarding points. A player's ship disappears when its health reaches zero. There is currently no automatic respawn.

## Gameplay loop

1. Move around the playfield and avoid enemies and their shots.
2. Fire at incoming enemies to clear a path and earn points.
3. Survive the next enemies, which continue to arrive at a steady rate.

The current rules do not increase the spawn rate or introduce new enemy types over time. The score continues to accumulate as enemies are destroyed.
