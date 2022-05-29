# OpenGL Developer Tool

---

Image

---

OpenGL Version:
> 4.60

Visual Studios Version:
> Microsoft Visual Studio Community 2019
> 
> Version 16.11.15

Operating System:
> Windows 10 Home

## Interacting with the executable

Double click on the executable (.exe) application

<img align = "centre" src= "https://raw.githubusercontent.com/FearTheRogue/COMP3015-Coursework_2/main/readme%20images/imgui_image.PNG" width = "400">

Upon launching, an ImGui interface can be used to change certain values in the scene.

<img align = "centre" src= "https://raw.githubusercontent.com/FearTheRogue/COMP3015-Coursework_2/main/readme%20images/togglerotate_image.PNG" width="400">

First of all, the "Toggle Rotation" checkbox is deactivated - meaning the user can press the 'A' & 'D' keyboard keys to rotate the camera around the centre pivot point, and adjust the rotation speed using the slider. By pressing 'W' & 'S' keyboard keys will allow the user to zoom in or out.

By enabling the checkbox will result in user input being disabled. 

The next few collapsable headers can be used to change the following:

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/lightproperties_image.PNG?raw=true" width="400">
  
**Light Properties** - The top 3 sliders control the light's x, y and z position. The next slider sets the light to either a point or directional light: 0 is directional and 1 is point light.  

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/particlessystem_image.PNG?raw=true" width="400">

**Particle System** - By default the fountains particle system is disabled, and can be enabled by toggling the "Begin Particles" checkbox. The "Particle Accelaration" sliders controls the which direction the particles will flow. "Particle Size" slider changes the size of the particles. The particle lifetime can be updated using the "Particle Lifetime" slider. Similarily, the amount of particles emitted is changed with the subsequent slider. 

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/firesystem_image.PNG?raw=true" width="400">

**Fire System** - Similar to the particle system the fire simulation is disabled by default, and enabled using the checkbox. The next few sliders also control the corresponding fire particles. 

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/watersystem_image.PNG?raw=true" width="400">

**Water System** - These control the water located in the pond model. "Water Frequency" controls the length between each wave. "Water Velocity" controls the speed of the waves, and the "Wave Amp" or amplitude controls the wave heigth. 

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/geometryshaders_image.PNG?raw=true" width="400">

**Geometry Effect Shaders** - Contains 2 different geometry shader techniques for each model in the scene, wireframe and silhouette. By toggling an effect will change that specific models shader.

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/geometrysettings_image.PNG?raw=true" width="400">

After toggling an effect, a collapsable header will appear. Where the user can change some of the settings for that effect. For example, colour or line width. 

---

How do you open and control the software you wrote (exe file)?

How does the program code work? How do the classes and functions fit together and who
does what?

What makes your shader program special and how does it compare to similar things?
(Where did you get the idea from? What did you start with? How did you make yours
unique? Did you start with a given shader/project?)

## YouTube Video:

---

video thumbnail

---

Include a link to a video report that details the above but also goes into more detail on:

o Anything else which will help us understand how your shader works.

o Are there any software engineering issues, such as the trade-off between performance and good practice?

o A (brief) evaluation of what you think you have achieved, and what (if anything) you would do differently, knowing what you now know. Don’t be modest but be realistic!
