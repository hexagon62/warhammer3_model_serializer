# Model (de)serializer for Total War: Warhammer III
Allows for converting to/from binary `.anim` files to a plaintext format.

Intended for use with [Blender](https://www.blender.org/),
[AssetEditor](https://github.com/donkeyProgramming/TheAssetEditor),
and [RME](https://github.com/mr-phazer/RME_Release).

This program is provided as-is and I don't currently plan to update or maintain it.

However, if you experience issues with it, feel free to let Lt. Rocky or me know.

## Credits
The C++ portion of this is a program I wrote which was requested by Lt. Rocky.

The Python portion is a script written by Lt. Rocky, used to help interface with Blender.

## Compiling the (de)serializer
All you need is to compile the `main.cpp` file in your favorite compiler.
Personally, I used Microsoft Visual C++.

I wrote it with C++20 in mind, so make sure to set that in your compiler's options.

It can probably work with older standards with some tweaking.

## Using the (de)serializer
Simply drag-and-drop a `.anim` file to convert it to a `.txt` file.

To do the reverse and convert back to `.anim`, drag-and-drop a `.txt` file.

If working from the shell, you can specify `-s` after the file to make the program immediately quit on completion.

## Getting your skeleton into RME
The script written for this process was tested with Blender 3.9.0 but should work for 4.2.

1) In Blender, grab/create the skeleton you want to convert.
2) Save the `.blend` file to a work folder. This folder must have another folder in it named `Assert SRC`
3) Create a new window in the Blender scene, make the window a text editor.
    - If unsure how to do this, watch [this video](https://www.youtube.com/watch?v=HSm-cq7zd2s)
4) Locate the `Open Text` button in the new window, and select `writeskeleton.py`.
5) Ensure the following:
    - The front of your skeleton points in the -Y direction, and the top points in the +Z direction.
    - The rotation on the X-axis must be 90° (you can see this in the object tab). To achieve this:
        - Rotate the skeleton object by -90°
        - Apply the rotation (Ctrl+A), then rotate the skeleton on the X axis by 90°.
    - The scene's unit scale is set to `0.0254`. This is located in the scene tab under the properties window.
6) Select the skeleton in object mode, then click the `Run Script` button in the text editor window.
    - This button looks like a play button.
7) A `.txt` file with the same name as the skeleton should be generated in the `Asset SRC` folder.
    - An extra armature will be created in the Blender scene. You can remove it.
8) Run the (de)serialization program with the `.txt` file as the input (see above).
9) This should generate a `.anim` file which you can open in AssetEditor and view with the Visual Skeleton Editor.
10) Once you know the result is to your liking, you should save this file inside of AssetEditor.
    - Right now the serializer program doesn't output a `.anim` file that RME can work with.
    - Howver, AssetEditor works with it and saving the `.anim` file gets it to a state compatible with RME.
11) You should be able to import and use animations with the `.anim` file with RME now.
