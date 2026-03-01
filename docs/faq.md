[Back to main README](../README.md)

## FAQ <a name="faq"></a>

### General Questions

**Q: Can I develop games without a R36S device?**  
A: Yes! You can develop and test using the Linux and Web build target on your PC. Only final testing requires R36S.

**Q: What programming languages can I use?**  
A: C with Raylib (C++ possible not tested). You could potentially use any languages that compile to ARM binaries.

**Q: Will my game work on other handheld consoles?**  
A: Possibly if its a Linux based Handheld and if they use similar ARM architecture, support DRM/GLES2. Will need recompilation and adjustments.

### Development Questions

**Q: How do I update my game after making changes?**  
A: Run `make PLATFORM=r36s` inside Docker container, copy _binary_ and _assets_ to R36S.

**Q: Can I use libraries besides Raylib?**  
A: Yes such as [SDL](https://www.libsdl.org/), with modifications to _Dockerfile_ and _Makefile_. Ensure ARM compatibility.

**Q: How do I handle game saves/settings?**  
A: Save to `/home/ark/.config/yourgame/` or local directory. Use [Raylib's file I/O](https://www.raylib.com/examples/core/loader.html?name=core_storage_values) functions.

**Q: What's the best way to organise game assets?**  
A: Keep them in subdirectories (`assets/images/`, `assets/sounds/`) and use relative paths in your code.

### Performance Questions

**Q: What frame rate should I target?**  
A: 30 to 60 FPS is reasonable. The R36S can handle 60 FPS for well-optimised 2D games, carefull not to load large asset or model files.

**Q: How much RAM does the R36S have?**  
A: Typically 512MB to max 1GB depending on model. Keep memory usage conservative.

**Q: Can I use 3D graphics?**  
A: Yes, but keep polycount low. Use optimised models and textures. GLES2 is supported, so you could model in [Blender](https://www.blender.org/) and export as [gltf/glb](https://docs.blender.org/manual/en/2.80/addons/io_scene_gltf2.html) model files.

**Q: How do I optimise for the 640x480 screen?**  
A: Design large UI elements, use readable fonts, test on device.

### Troubleshooting Questions

**Q: Game crashes immediately on R36S but works on Linux; why?**  
A: Check logfile see [logfile configuration section](./advanced.md#r36s-autostart-configuration")

**Q: How do I remove Docker container?**  
A: Remove Docker container with `docker rmi r36s-build`.

**Q: Can I use this setup for other ARM devices?**  
A: Potentially yes, with modifications to _Dockerfile_ and _Makefile_.

[Back to main README](../README.md)