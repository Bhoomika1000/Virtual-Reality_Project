###🧠 Narnia's Escape & 🪂 Parachute Drop Simulation Projects ###
This repository contains two immersive interactive graphics projects:

Narnia's Escape – A Unity-based VR puzzle adventure

Parachute Drop Simulation – A 2D OpenGL & GLUT animated environment

📁 Project 1: Narnia’s Escape – A Unity Puzzle Adventure
🧙 Description:
Narnia’s Escape is a fantasy-inspired first-person puzzle game developed in Unity. The player begins in a tranquil mystical forest and is transported to a locked escape room. To win, the player must solve a series of logic-based and physics-based puzzles to open the final door and return home.

🔧 Features:
First-person movement and mouse-look camera controls

Scene transitions between forest and room using triggers

Interactive button sequence puzzle

Physics-based throwing puzzle (crystal ball)

Animated door that opens upon puzzle completion

Custom lighting and effects for key objects

Built using Unity Asset Store models and ProBuilder geometry

Clean code with OnMouseDown(), Raycasting, SceneManager, and Coroutines

🧩 Gameplay Flow:
The Mystical Forest – Player spawns and explores to find a large tree tunnel.

Teleportation – Entering the tree teleports the player to an escape room.

Puzzle 1 – Press a series of buttons in the correct order.

Puzzle 2 – Grab the glowing crystal ball and throw it at the door.

Escape – If the throw has enough force, the door opens and the game ends.

📂 File Structure:



<img width="568" height="187" alt="image" src="https://github.com/user-attachments/assets/c543b3c0-aec7-451a-be67-12522d0f99fc" />

▶️ How to Run:

Open the Unity project in the Unity Editor.

Set SampleScene as the startup scene.

Click Play to begin.

📁 Project 2: Parachute Drop Simulation (OpenGL & GLUT)

🌤 Description:

An animated 2D parachute drop simulation using OpenGL and GLUT, showcasing realistic physics like gravity, wind drift, parachute collapse, and post-landing animation. The scenery is alive with butterflies, trees, clouds, and sun animations.

🔧 Features:
Physics-driven vertical fall with gravity and landing drag

Horizontal drift based on wind strength

Swaying parachute animation

Realistic parachute collapse and detachment

Running animation after landing

Moving clouds and flying butterflies

Scenic ground layers with hills, trees, flowers, and sky

Wind strength adjustable via keyboard

Reset animation anytime

🎮 Controls:
Key	Function
R	Reset animation
+	Increase wind strength
-	Decrease wind strength
Esc	Exit

🛠 Technologies Used:
OpenGL + GLUT for rendering

C++ for logic and animation

Sin/Cos-based motion

Custom physics system

Procedural butterfly animation

Transparent clouds via alpha blending

Hierarchical character rendering

📦 File Structure:


<img width="705" height="137" alt="image" src="https://github.com/user-attachments/assets/70f310cf-d6be-4084-9100-cc8eb36dacb6" />

✅ How to Run:
Ensure OpenGL and GLUT are installed on your system.

Compile the program:


<img width="608" height="67" alt="image" src="https://github.com/user-attachments/assets/a0f45604-274c-4bcc-9ef0-38bda58beed2" />

