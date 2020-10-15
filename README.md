# glsl-experiments
This is just a simple playground app I built using some very helpful libs:

- [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)
- [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit)
- [json11](https://github.com/dropbox/json11)

The idea is to conveniently load meshes and play/create with shader code inside the app.

![GitHub Logo](screenshots/screenshot.png)

## Build
    ```
    > mkdir build
    > cd build
    > cmake ..
    > make -j8
    > ./glsl_editor
    ```

Hopefully my cmake configurations will take care of all dependencies.

## Features
- Continuous compilation (error listing)
- Save/Load shader files (including a configuration file containing values of light/material)
- Visual cues for debugging (point light, directional light)

## TODO
- Add other useful uniforms such as time, mouse position, etc.
- Add texture support (more than one channel).
- Pass other mesh information to the shader as well.
- Incorporate [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo).
- Skybox/sphere allowing environment mapping.
- Shadows (add a floor to the scene).

## Screenshots
![GitHub Logo](screenshots/toon_shader.png)
![GitHub Logo](screenshots/topographical_shader.png)
![GitHub Logo](screenshots/normal_mapping.png)
