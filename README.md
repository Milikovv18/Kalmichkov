<p align="center">
  <img src="https://github.com/user-attachments/assets/9eadb339-7dd8-44c4-81dd-3369f9e044ff" />
</p>

# 🐍 Kalmichkov

When I was a freshman in university, one of my group-mates struggled with understanding data structures. To help, I created a fun snake game leveraging the unmodified file `Qlmichkov.cpp/h`, originally written by our lecturer, whose last name inspired the project's name. The game incorporates standard snake mechanics with additional custom effects and supports both single-player and two-player modes. It’s built using my custom 2D graphics engine.

---

## 🕹️ How to Play

- **Objective**: Collect as many red apples as possible.
- **Controls**:
  - Use the **left** and **right arrow keys** to rotate the snake counterclockwise and clockwise, respectively.
  - Press the **spacebar** to crawl faster.
- **Special Rule**: Wrapping around edges is possible unless there’s a wall on the opposite side!

---

## 📂 Project Structure

### 1. **`Qlmichkov.cpp`**
   - The file that inspired the project.
   - Created by my university lecturer.
   - Contains basic data structures, especially `Queue`, which represents the snake.

### 2. **`Drawing.cpp`**
   - My custom 2D rendering engine for displaying visuals in a Windows command line.
   - Utilizes **WinAPI's `StretchBlt`** for fast rendering of the pixel buffer.
   - Provides helper functions (`fill`, `loadPic`, `rect`, `circle`, `roundedRect`) for drawing complex objects and utilities like `align` (alignment) and `scale` (pixel density adjustments).

### 3. **`Menu.cpp`**
   - A CLI-based menu for selecting game modes:
     - **Server Mode**: Single-player by default; multiplayer if clients connect.
     - **Client Mode**: Joins a multiplayer session; prompts for server IP.

### 4. **`Maps.h`**
   - Contains:
     - Hard-coded maps.
     - `MapHandler` class with debugging and semantic interpretation methods.

### 5. **`Snake.h`**
   - Defines the player-controlled snake:
     - Turn left/right.
     - Eat food.
     - Die when conditions are met.
   - Note: Opponent snakes are managed via `Enemy.h`.

### 6. **`Enemy.h`**
   - Represents non-controllable opponent snakes.
   - Receives network data and renders accordingly.

### 7. **`Network.cpp`**
   - Implements basic TCP server/client using **WinAPI**.
   - Includes helpers: `sendData`, `recvData`, `sendMap`, `recvMap`.

### 8. **`Effects.h`**
   - Introduces food-based special effects with varying rarities:
     - Rare effects: Washed-out yellow.
     - Common effects: Bright yellow.
   - **Effects**:
     - **Hunger**: Snake shrinks every second (negative).
     - **Blindness**: Darkens the screen except for the snake's head area (negative).
     - **Destroyer**: Removes one obstacle permanently (positive).
     - **Undead**: Turns the main apple into a moving "zombie apple" (negative).
     - **Slowness**: Temporarily slows the snake down, easing gameplay (positive).

### 9. **`Finished.cpp`**
   - Contains two functions:
     - `youLost`: Displays the losing screen.
     - `youWin`: Displays the winning screen.

---

## 🎥 Example Gameplay

https://user-images.githubusercontent.com/89595575/147372614-4ba23934-cd0e-4c6f-ae68-239d131bc7e6.mp4

(*Translations for hints in the video*):

- **[0:00]** Compilation Modes: Client-only, Server-only (+Single-player), 2-in-1.
- **[0:10]** Controls: `←` Counterclockwise, `→` Clockwise, `[space]` Acceleration.
- **[0:17]** Bonus: A "zombie" apple starts moving.
- **[0:29]** Blindness effect: Everything goes dark.
- **[0:36]** Hunger effect: Snake shrinks.
- **[0:46]** Destroyer effect: An obstacle is removed.
- **[0:57]** Slowness effect: Snake slows down.
- **[1:10]** Multiplayer demonstration.

---

## 🔧 Build Instructions

- **Recommended Toolchain**: **MSVC x64** compiler.
- **Setup**: Open the `Kalmichkov.vcxproj` file in **Visual Studio** and build the project.  

Enjoy the game! 🎮
