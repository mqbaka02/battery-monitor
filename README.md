# Battery Monitor

A C++ application built with **SDL3**, **Dear ImGui**, and **ImPlot**.

## Technologies

- **C++17** — application development
- **SDL3** — window management, rendering, and events
- **Dear ImGui** — graphical user interface
- **ImPlot** — plotting and data visualization
- **Conan 2** — C++ dependency management
- **GNU Make** — build automation

## Requirements

The following tools must be installed:

- GCC / G++
- GNU Make
- Python 3 and Conan 2
- SDL3 development libraries
- `pkg-config`

The project uses Conan to manage Dear ImGui and ImPlot. SDL3 is currently provided by the system rather than Conan.

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/mqbaka02/battery-monitor.git
cd battery-monitor
```


### 2. Install dependencies

Install the required Conan packages and generate the dependency configuration:

```bash
conan install . --build=missing
```

This installs Dear ImGui and ImPlot and generates the `pkg-config` files used by the Makefile.

### 3. Build the application

```bash
make
```

The compiled executable will be created at:

```text
exec/battery-monitor
```

### 4. Run the application

```bash
./exec/battery-monitor
```

## Build Commands

| Command | Description |
|---|---|
| `make` | Build the application |
| `make clean` | Remove compiled objects and the executable |
| `make clean && make` | Perform a clean rebuild |
| `make logger` | Build the logger daemon executable |
| `make install_logger` | Installs and run the logger daemon in your system  |
| `make uninstall_logger` | Uninstall the logger daemon |
| `make logger_status` | Check the current status of the logger daemon in your system (active, inactive, not installed) |

## Project Structure

```text
battery-monitor/
├── assets/       # Application assets
├── exec/         # Compiled executable
├── include/      # Application headers
├── logger/       # Code and executable for the logger daemon
├── src/          # Application source files
├── Makefile      # Build configuration
├── conanfile.txt # Conan dependencies
├── .gitignore
└── README.md
```

## Dependencies and Third-Party Code

Dear ImGui and ImPlot are managed through Conan rather than vendored into this repository.

The ImGui SDL3 backend source files are compiled from the Conan package cache during the build. They are not copied into the project.

SDL3 must be installed separately and made available to `pkg-config`.


## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE.md) file for details.
