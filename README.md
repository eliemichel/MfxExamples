MfxExamples
===========

This is a simple example of [OpenMfx](https://openmesheffect.org/) plugin. You can use it as a base to write your own plugin and get inspiration.

Building
--------

This project uses CMake in a standard way:

```
mkdir build
cd build
cmake ..
cmake --build . --config Release # or "make"
```

Usage
-----

Run an OpenMfx host software, for instance [the OpenMfx branch of Blender](https://github.com/eliemichel/OpenMfxForBlender) and load in there the .ofx file that results from building.
