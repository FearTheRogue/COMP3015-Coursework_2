# OpenGL Developer Tool

---

https://user-images.githubusercontent.com/43548013/170889613-7702ac6a-2dd8-4a93-aee2-4c6a25007391.mp4

---

OpenGL Version:
> 4.60

Visual Studios Version:
> Microsoft Visual Studio Community 2019
> 
> Version 16.11.15

Operating System:
> Microsoft Windows 10

## Interacting with the executable

Double click on the executable (.exe) application

<img align = "centre" src= "https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/imgui_image.PNG?raw=true" width = "400">

> Upon launching, an ImGui interface can be used to change certain values in the scene.

<img align = "centre" src= "https://raw.githubusercontent.com/FearTheRogue/COMP3015-Coursework_2/main/readme%20images/togglerotate_image.PNG" width="400">

> First of all, the "Toggle Rotation" checkbox is deactivated - meaning the user can press the 'A' & 'D' keyboard keys to rotate the camera around the centre pivot point, and adjust the rotation speed using the slider. By pressing 'W' & 'S' keyboard keys will allow the user to zoom in or out.
>
> By enabling the checkbox will result in user input being disabled. 

> The next few collapsable headers can be used to change the following:

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/lightproperties_image.PNG?raw=true" width="400">
  
> **Light Properties** - The top 3 sliders control the light's x, y and z position. The next slider sets the light to either a point or directional light: 0 is directional and 1 is point light.  

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/particlessystem_image.PNG?raw=true" width="400">

> **Particle System** - By default the fountains particle system is disabled, and can be enabled by toggling the "Begin Particles" checkbox. The "Particle Accelaration" sliders controls the which direction the particles will flow. "Particle Size" slider changes the size of the particles. The particle lifetime can be updated using the "Particle Lifetime" slider. Similarily, the amount of particles emitted is changed with the subsequent slider. 

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/firesystem_image.PNG?raw=true" width="400">

> **Fire System** - Similar to the particle system the fire simulation is disabled by default, and enabled using the checkbox. The next few sliders also control the corresponding fire particles. 

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/watersystem_image.PNG?raw=true" width="400">

> **Water System** - These control the water located in the pond model. "Water Frequency" controls the length between each wave. "Water Velocity" controls the speed of the waves, and the "Wave Amp" or amplitude controls the wave heigth. 

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/geometryshaders_image.PNG?raw=true" width="400">

> **Geometry Effect Shaders** - Contains 2 different geometry shader techniques for each model in the scene, wireframe and silhouette. By toggling an effect will change that specific models shader.

<img align="centre" src="https://github.com/FearTheRogue/COMP3015-Coursework_2/blob/main/readme%20images/geometrysettings_image.PNG?raw=true" width="400">

After toggling an effect, a collapsable header will appear. Where the user can change some of the settings for that effect. For example, colour or line width. 

---

## How does the program code work?

scene_project.cpp

> In the constructor it loads the models used in the project, and assigns the default values for the shader properties.
>
> initScene()
> 
> * Assigns particle textures, and sets the uniforms for the different shaders.
> 
> initBuffers()
> 
> * Generate the particle buffers (position, velocity and age).
> * Creates vertex arrays for each set of buffers.
> * Sets up the feedback objects.
> 
> compile()
> 
>  * Loads in the different shader files.
>  
>  setupNormalShader()
>  
>  * Handles the main scene lighting.
>  
>  setupWireframeShader()
>  
>  * Handles setting the wireframe uniforms.
>  
>  setupSilhouetteShader()
>  
>  * Handles setting the silhouette uniforms.
>  
>  render()
>  
>  * Handles the ImGui imputs from the user.
>  * Changes the model translation.
>  * Renders the models depending on their applied effect shaders.
>  * Draws the particles.



What makes your shader program special and how does it compare to similar things?
(Where did you get the idea from? What did you start with? How did you make yours
unique? Did you start with a given shader/project?)

The original idea for this project came from the first project I developed in OpenGL, [here](https://github.com/FearTheRogue/COMP3015-Initial_Prototype). I wanted to further develop a "park" theme scene, and therefore the first implementation was the fountain model and particle system.

## External Resources

The project using the [ImGui library](https://github.com/ocornut/imgui) to render the UI.

## YouTube Video:

---

video thumbnail

---

Include a link to a video report that details the above but also goes into more detail on:

o Anything else which will help us understand how your shader works.

o Are there any software engineering issues, such as the trade-off between performance and good practice?

o A (brief) evaluation of what you think you have achieved, and what (if anything) you would do differently, knowing what you now know. Don’t be modest but be realistic!
