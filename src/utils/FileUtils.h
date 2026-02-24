// =============================================================================
// FileUtils.h — File Reading Utility
// =============================================================================
// A header-only utility for reading entire files from disk into std::string.
// Primarily used by the Shader class to load .vert and .frag shader files.
//
// Usage:
//   std::string source = FileUtils::ReadFile("shaders/basic.vert");
// =============================================================================

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>       // std::string
#include <fstream>      // std::ifstream — file input stream
#include <sstream>      // std::stringstream — buffer the file contents
#include "utils/Logger.h"  // LOG_ERROR for reporting missing files

// =============================================================================
// FileUtils — Static utility class for file operations
// =============================================================================
class FileUtils {
public:
    // =========================================================================
    // ReadFile — Reads the entire contents of a file into a string
    // =========================================================================
    // Parameters:
    //   filepath — relative or absolute path to the file to read
    // Returns:
    //   The file contents as a std::string, or an empty string on failure
    // =========================================================================
    static std::string ReadFile(const std::string& filepath) {
        // Attempt to open the file
        std::ifstream file(filepath);

        // Check if the file was opened successfully
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file: " + filepath);
            return "";  // Return empty string on failure
        }

        // Read the entire file into a stringstream buffer
        std::stringstream buffer;
        buffer << file.rdbuf();     // rdbuf() gives direct access to the file's buffer

        // Close the file (also happens automatically when ifstream goes out of scope)
        file.close();

        LOG_INFO("Successfully read file: " + filepath);
        return buffer.str();        // Convert the stringstream to a std::string
    }
};

#endif // FILEUTILS_H
