> This project was generated with Claude 4 Sonnet (version 2025-06-03).

# AI Generated Transport Network 2

A sophisticated transportation and logistics simulation project that models a cargo distribution network using cities from the Game of Thrones universe.

## Project Overview

This project implements a graph-based transportation network where cargo trucks move goods between cities through interconnected road systems. The simulation models real-world logistics challenges including truck capacity constraints, road weight limits, and speed restrictions.

## Network Model

### Cities (Nodes)
The network consists of 6 major cities from the Game of Thrones universe:

- **King's Landing** - The capital city and major trade hub
- **Winterfell** - Northern stronghold with abundant wood resources
- **Braavos** - Free city specializing in iron production
- **Oldtown** - Ancient city and knowledge center
- **Qarth** - Wealthy trading city with diverse goods
- **Pentos** - Free city known for silk trade

### Warehouses
Each city maintains a warehouse with three types of goods:
- **Wood** (measured in tons) - Building materials and fuel
- **Iron** (measured in tons) - Metal for tools and weapons
- **Silk** (measured in tons) - Luxury textiles and trade goods

### Truck Fleet
Every city operates a mixed fleet of cargo vehicles:

| Truck Type | Max Capacity | Max Speed | Use Case |
|------------|--------------|-----------|----------|
| **Ford**   | 10 tons      | 80 km/h   | Light cargo, fast delivery |
| **MAN**    | 25 tons      | 70 km/h   | Heavy cargo, bulk transport |
| **VW**     | 5 tons       | 90 km/h   | Express delivery, high-value goods |

### Road Network (Edges)
The cities are connected by 12 roads with varying characteristics:

#### Road Types:
- **Standard Roads** (Blue) - Regular trade routes
- **Alternative Routes** (Green) - Secondary paths between major cities
- **Long Distance Routes** (Red) - Direct connections over vast distances

#### Road Constraints:
- **Maximum Mass Limit** - Weight restriction per truck (15-45 tons)
- **Speed Limit** - Maximum allowed speed (60-95 km/h)

### Notable Routes:
- **Kingsroad North** - Primary route between King's Landing and Winterfell
- **Rose Road** - Major highway to Oldtown
- **Valyrian Highway** - Ancient road connecting eastern cities
- **Free Cities Highway** - Trade route between Braavos and Pentos

## Technical Implementation

### Dependencies
- **CMake** (>= 3.10)
- **CLI11** - Command line interface library
- **fmt** - Formatting library
- **spdlog** - Logging library
- **Graphviz** (optional) - For network visualization

### File Structure:
- `networks/got.dot` - Graph definition in DOT format (Game of Thrones network)
- `CMakeLists.txt` - Build configuration
- `src/main.cxx` - Core application logic and command handling
- `src/dot_parser.cxx` - Parser for DOT format network files
- `src/city_display.cxx` - City display and information functions
- `include/dot_parser.hxx` - Header definitions for the transportation network
- `README.md` - Project documentation

### Command Line Interface

The simulation provides a command-line interface with several options:

```bash
# Display basic city list
./transport_network show cities

# Display city list with warehouse information
./transport_network show cities --warehouse

# Display city list with fleet information
./transport_network show cities --fleet

# Display city list with connected cities
./transport_network show cities --neighbor

# Display city list with average distances
./transport_network show cities --avg-dist

# Display network statistics
./transport_network --stats

# Ship cargo between cities (with required parameters)
./transport_network ship wood 12.2 Winterfell 12:00

# Ship cargo from specific cities
./transport_network ship iron 10 Winterfell 12:00 --from Braavos,Pentos,Qarth
```

### Development

#### Building the Project
The project includes a build script that supports parallel builds and detects the Ninja build system:

```bash
# Basic build
./build.sh

# Clean debug build with 8 jobs
./build.sh --clean --debug --jobs 8

# For more options
./build.sh --help
```

#### Code Style
The project uses clang-format for consistent code style. Apply formatting with:
```bash
clang-format -i src/*.cxx include/*.hxx
```

#### Visualization
The network can be visualized using Graphviz:
```bash
dot -Tpng networks/got.dot -o network.png
