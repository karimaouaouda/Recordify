# Recordify - Screen Recorder Project Structure

## Overview
This project follows a modular architecture where each module handles a specific aspect of screen recording functionality.

## Directory Structure

```
Recordify/
├── src/                          # Source files organized by modules
│   ├── core/                     # Core application logic
│   │   ├── main.cpp             # Application entry point
│   │   ├── application.cpp      # Main application class
│   │   ├── recorder.cpp         # Core recording coordinator
│   │   └── settings.cpp         # Global settings management
│   ├── screen_handler/          # Screen capture functionality
│   │   ├── screen_capture.cpp   # Screen capture implementation
│   │   ├── display_detector.cpp # Monitor detection and selection
│   │   └── region_selector.cpp  # Custom region selection
│   ├── audio_handler/           # Audio capture and processing
│   │   ├── audio_capture.cpp    # Audio input capture
│   │   ├── audio_mixer.cpp      # Audio mixing and processing
│   │   └── audio_devices.cpp    # Audio device management
│   ├── video_handler/           # Video processing and encoding
│   │   ├── video_encoder.cpp    # Video encoding (H.264, etc.)
│   │   ├── frame_processor.cpp  # Frame processing and effects
│   │   └── video_compressor.cpp # Video compression utilities
│   ├── file_manager/            # File operations and formats
│   │   ├── file_writer.cpp      # File writing operations
│   │   ├── format_handler.cpp   # Multiple format support (MP4, AVI, etc.)
│   │   └── metadata_manager.cpp # File metadata handling
│   ├── ui/                      # User interface components
│   │   ├── main_window.cpp      # Main application window
│   │   ├── control_panel.cpp    # Recording controls
│   │   ├── settings_dialog.cpp  # Settings interface
│   │   └── preview_window.cpp   # Recording preview
│   ├── config/                  # Configuration management
│   │   ├── config_parser.cpp    # Configuration file parser
│   │   ├── user_preferences.cpp # User preference management
│   │   └── default_settings.cpp # Default configuration values
│   └── utils/                   # Utility functions and helpers
│       ├── logger.cpp           # Logging system
│       ├── timer.cpp            # Timing utilities
│       ├── error_handler.cpp    # Error handling and reporting
│       └── string_utils.cpp     # String manipulation utilities
├── include/                     # Header files (mirrors src structure)
│   ├── core/
│   ├── screen_handler/
│   ├── audio_handler/
│   ├── video_handler/
│   ├── file_manager/
│   ├── ui/
│   ├── config/
│   └── utils/
├── tests/                       # Unit tests (mirrors src structure)
│   ├── core/
│   ├── screen_handler/
│   ├── audio_handler/
│   ├── video_handler/
│   ├── file_manager/
│   ├── ui/
│   ├── config/
│   └── utils/
├── bin/                         # Compiled executables
├── obj/                         # Object files (organized by module)
├── docs/                        # Documentation
├── examples/                    # Example usage and demos
└── external/                    # Third-party dependencies (optional)
```

## Module Responsibilities

### 🎯 Core Module
- Application lifecycle management
- Main recording coordination
- Global settings and state management

### 🖥️ Screen Handler Module
- Screen/monitor detection and selection
- Custom region selection
- Frame capture from display

### 🔊 Audio Handler Module
- Audio input device management
- Audio capture and recording
- Audio mixing and processing

### 🎬 Video Handler Module
- Video encoding and compression
- Frame processing and effects
- Video format conversion

### 📁 File Manager Module
- File I/O operations
- Multiple format support (MP4, AVI, MOV, etc.)
- Metadata management

### 🎨 UI Module
- User interface components
- Recording controls and indicators
- Settings and configuration dialogs

### ⚙️ Config Module
- Configuration file management
- User preferences persistence
- Default settings management

### 🛠️ Utils Module
- Logging and debugging
- Error handling and reporting
- Common utility functions

## Build System
- Modular compilation with automatic subdirectory discovery
- Separate object files for each module
- Independent testing for each module
- Debug and release configurations
