# Delaunay Flow - Geometric Live Desktop Wallpaper

## Description
Delaunay Flow is a visually striking and interactive live desktop wallpaper that uses Constrained Delaunay Triangulation to generate and color a field of triangles across your screen. Each triangle's color is determined by its vertical position, creating a smooth gradient that flows from top to bottom.

## Features

- 🔺 Dynamic triangle mesh generated via Constrained Delaunay Triangulation
- 🌈 Smooth vertical color gradient with multiple color stops
- ⚙️ Fully customizable via `settings.json`
- 🚀 High-performance OpenGL rendering
- 💻 Built in C++ with real-time responsiveness

## Requirements

- OS: Windows 10/11
- GPU: OpenGL-compatible
- Compiler: Visual Studio 2022 (recommended)

## Installation

1. **Download**: Get the latest version of Delaunay Flow from the [Releases](https://github.com/erfan-ops/Delaunay-Flow/releases) page.
2. **Launch**: Run `Delaunay-Flow.exe` to enjoy your new live wallpaper.

## Building

1. Clone the repository:

   ```bash
   git clone https://github.com/erfan-ops/Delaunay-Flow.git
   ```

2. Open the project in Visual Studio 2022.
3. Build the solution.
4. Run the executable from the output directory.

## Configuration

Customize the wallpaper by editing `settings.json`:

```json
{
    "fps": 120,
    "vsync": false,

    "background-colors": [
        [ 0.28, 0.09, 0.65, 1 ],
        [ 0.98, 0.33, 0.33, 1 ],
        [ 0.98, 0.55, 0.15, 1 ],
        [ 0.96, 0.82, 0.2, 1 ],
        [ 0.47, 0.3, 0.58, 1 ]
    ],
  
    "stars": {
        "draw": false,
        "segments": 12,
        "radius": 0.01,
        "count": 150,
        "min-speed": 0.005,
        "max-speed": 0.026,
        "color": [ 0, 0, 0, 0.66 ]
    },

    "edges": {
        "draw": true,
        "width": 0.0038,
        "color": [ 0, 0, 0, 0.69 ]
    },

    "interaction": {
        "mouse-interaction": true,
        "distance-from-mouse": 0.25,
        "speed-based-mouse-distance-multiplier": 0
    },

    "mouse-barrier": {
        "draw": true,
        "radius": 0.25,
        "color": [ 0.2, 0.05, 0.7, 0.3 ],
        "blur": 25
    },

    "offset-bounds": 0.3,

    "MSAA": 4
}
```

- `fps`: Target frames per second.
- `vsync`: uses vertical synchronization.
- `background-colors`: Gradient stops (RGBA format) interpolated based on triangle Y position.
- `stars`: Star configurations (speed, count, radius, color, etc.).
- `edges`: Configuration for drawing triangle edges.
- `interaction`: enables the mouse to move the stars away.
- `mouse-barrier`: Configuration for drawing a glow around the mouse.
- `offset-bounds`: The screen offset which enables the stars to go pass though screen boundaries.
- `MSAA`: enables multi-sample anti-aliasing

## Contribution

Contributions are welcome! If you have an idea for improvement or want to submit a fix, open an issue or send a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgments

- OpenGL for rendering
- CDT algorithms for triangle generation

🎨 Transform your desktop into a geometric canvas with **Delaunay Flow**! 🔻✨
