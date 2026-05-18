# Viva Explanation Sheet - ComputerGraphicsProject_TheHomeModule

## 1) Project Goal
This module simulates a short day-night home story using OpenGL:
- **Scene 1**: Morning car departure from home.
- **Scene 9**: Night return, garage parking, and house lights turning on.

The design combines:
- 2D primitive drawing (`rect`, `circle`, `ellipse`)
- time-based animation (`update` timer)
- state machines (`anim1`, `anim9`)
- user interaction (`keyboard`, `specialKeys`)

## 2) Main Architecture
The program follows the classic GLUT architecture:
1. `main()` initializes window and callbacks.
2. `display()` renders current frame.
3. `update()` runs every ~16 ms and updates animation data.
4. Input callbacks modify global state.

Important callbacks:
- `glutDisplayFunc(display)`
- `glutTimerFunc(TIMER_INTERVAL_MS, update, 0)`
- `glutKeyboardFunc(keyboard)`
- `glutSpecialFunc(specialKeys)`

## 3) How Scene Changing Works
- `currentScene` stores active scene.
- Available scenes in this module: **1 and 9**.
- `nextScene()` advances scene and calls `resetScene(currentScene)`.
- Left arrow manually toggles scene 1 <-> 9.

Why reset is needed:
- `resetScene()` reinitializes car positions, door status, lights, and state counters.
- This guarantees deterministic replay each time a scene starts.

## 4) How Animation Works
Animation is done by **changing global variables over time** in `update()`:
- Car position: `scene1_carPosX/Y`, `scene9_carPosX/Y`
- Wheel spin: `wheelRotationAngle`
- Garage door: `garageDoorOpenAmount`
- Sky effects: cloud offsets, sun pulse, window flicker timers

`display()` only draws current values. It does not compute movement.

### Scene 1 state machine (`anim1`)
- **State 0**: transition to start.
- **State 1**: garage opens.
- **State 2**: car descends to road.
- **State 3**: car drives right and exits screen.
- On exit -> call `nextScene()`.

### Scene 9 state machine (`anim9`)
- **State 0**: car approaches garage.
- **State 1**: garage opens.
- **State 2**: car enters garage vertically.
- **State 3**: garage closes, scene pauses, returns to scene 1 reset.

## 5) How Rain Works
Rain has two parts:
1. **Initialization**: `initRain()` sets random start positions.
2. **Per-frame update** in `update()`:
   - `rainDropY` decreases (falling effect)
   - `rainDropX` shifts slightly (wind slant)
   - particles reset to top when below screen

Rendering:
- `drawRain()` draws each drop as a slanted line segment.
- `display()` overlays rain when `isRainEnabled == true`.

Weather impact on other visuals:
- Cloud color darkens in rain mode.
- Scene 9 fireflies are disabled in rain mode.

## 6) How Car Movement Works
Car motion is not physics-based; it is scripted by states:
- Position increments are fixed speeds scaled by `animationSpeed`.
- Wheel rotation is coupled to movement for realism.
- Headlight and taillight behavior is controlled by scene state.

In Scene 9:
- Headlights are ON only before entering garage (`carState_scene9 < 2`).
- House lights turn ON after parking is complete.

## 7) How Buttons/Keys Work
### Normal keys (`keyboard`)
- `Esc`: Exit app
- `R`: Rain ON
- `V`: Rain OFF
- `Space`: Pause/resume animation updates
- `N`: Reset module to Scene 1 start state

### Special keys (`specialKeys`)
- `Left Arrow`: Toggle between scene 1 and scene 9
- `Right Arrow`: Move to next scene via `nextScene()`

## 8) Rendering Order (Important for explanation)
Each scene renders in layered order:
1. Sky/background
2. Ground and environment
3. Trees/house
4. Car and light cone
5. Garage door overlay (so it appears in front when needed)
6. Rain overlay (if enabled)

This order creates correct depth in a 2D pipeline.

## 9) Key Viva Talking Points
- "I separated **drawing** and **animation logic**."
- "I used **state machines** instead of complicated physics for narrative control."
- "Every frame is deterministic because scene reset functions restore baseline values."
- "Rain is particle-based and independent from car state machines."
- "Visual ambience (sun glow, stars, fireflies, window flicker) is timer-driven."

## 10) Short 30-Second Summary
This project is a two-scene OpenGL home module. Scene 1 shows morning departure, Scene 9 shows night return and garage parking. I implemented animation using timer-driven global state updates, scene-specific state machines, and layered rendering. Weather is interactive with keyboard control, and rain is implemented as animated particles. Scene transitions, reset logic, and input handling are designed so the story can be replayed consistently.
