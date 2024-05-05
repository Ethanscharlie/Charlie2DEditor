#pragma once
#include <Charlie2D.h>

class TransformEdit;
extern std::string projectFolderpath;
extern Entity *selectedEntity;
extern json editorTempRunSave;
extern bool inRunState;
extern std::vector<Entity *> usedChildren;
extern std::stringstream buffer;

enum class ExportTypes { Linux, Windows, Web };

/**
 * @brief Serialize all entities in the GameManager to a JSON format.
 *
 * This function iterates over all Entity objects in the GameManager, excluding
 * those with a tag starting with '$', and serializes them into a JSON object.
 * The serialized data is stored in a hierarchical structure within the JSON
 * object.
 *
 * @return A JSON object representing the serialized entities. The structure of
 * the JSON object is as follows:
 * - "Scene": A top-level key representing the scene
 *   - entity->tag: A key for each entity's tag containing an array of
 * serialized data for that entity
 */
json serializeAllEntities();

/**
 * @brief Writes the given project folder path to a file named
 * "prevProject.txt".
 *
 * This function takes a std::filesystem::path as input and writes it to a file
 * named "prevProject.txt".
 *
 * @param projectFolderpath The path of the project folder to write to the file.
 *
 * @throws std::ios_base::failure if an error occurs during file operations.
 */
void writePrevProject(std::filesystem::path projectFolderpath);

/**
 * @brief Changes the selected entity to the specified entity.
 *
 * This function checks if the specified entity's tag starts with '$'. If it
 * does, the function returns without doing anything. If there is already a
 * selected entity, it removes the TransformEdit component from it. Then, it
 * sets the specified entity as the selected entity and adds a TransformEdit
 * component to it.
 *
 * @param entity Pointer to the Entity object to be set as the selected entity.
 */
void changeSelectedEntity(Entity *entity);

/**
 * @brief Compiles the project for export by setting up the CMake configuration
 * and building the project.
 *
 * @param exportFolder The path to the export folder where the project will be
 * compiled.
 * @return Returns 1 if the compilation for export is successful, 0 otherwise.
 */
int compileForExport(std::filesystem::path exportFolder,
                     ExportTypes exportType);

/**
 * @brief Recursively removes all files and directories within the specified
 * folder.
 *
 * This function traverses through all the entries (files and directories)
 * within the given folder. If an entry is a regular file, it is removed. If an
 * entry is a directory, this function is called recursively on that directory
 * before removing it.
 *
 * @param folder The path of the folder whose contents are to be removed.
 * @throws std::filesystem::filesystem_error if there is an error accessing or
 * removing files/directories.
 */
void removeFolderContents(const std::string &folder);

/**
 * @brief Refreshes assets by removing existing folder contents in "img",
 * copying new assets from the project folderpath into "img", reloading all
 * textures using ResourceManager, and updating image paths for all Sprite
 * components in the GameManager.
 *
 * This function first clears the contents of the "img" folder by calling
 * removeFolderContents("img"). It then iterates over all files in the "img"
 * directory of the projectFolderpath and copies them into the "img" folder.
 * Next, it reloads all textures using ResourceManager::reloadAllTextures().
 * Finally, it updates the image paths for all Sprite components in the
 * GameManager by creating new Image objects based on the existing image paths.
 *
 * @param None
 * @return None
 * @throws None
 */
void refreshAssets();

/**
 * @brief Reads and parses the EditorData.json file from the project folder
 * path.
 *
 * This function reads the EditorData.json file located in the project folder
 * path and parses its contents into a JSON object.
 *
 * @return A JSON object containing the parsed data from EditorData.json.
 */
json getEditorData();

/**
 * @brief Changes the data in the EditorData.json file with the provided JSON
 * data.
 *
 * This function writes the provided JSON data to the EditorData.json file
 * located in the project folder path.
 *
 * @param jsonData The JSON data to be written to the file.
 *
 * @throws std::filesystem::filesystem_error if there is an error accessing the
 * file system.
 */
void changeEditorData(json jsonData);

/**
 * @brief Retrieves the main scene data from a JSON file.
 *
 * This function reads the main scene data from a JSON file specified in the
 * editor data.
 *
 * @return A JSON object containing the main scene data.
 *
 * @throws std::exception if there are any errors during file handling or JSON
 * parsing.
 */
json getMainScene();

/**
 * @brief Changes the main scene to the specified new scene path.
 *
 * This function updates the editor data with the new scene path provided.
 *
 * @param newScenePath The new path of the scene to set as the main scene.
 *
 * @return void
 *
 * @throw std::exception if there is an error accessing or modifying the editor
 * data.
 */
void changeMainScene(std::filesystem::path newScenePath);

/**
 * @brief Inverts the lines in the input string.
 *
 * This function takes a string and reverses the order of lines in it.
 *
 * @param input The input string containing lines to be inverted.
 * @return The input string with lines inverted.
 */
std::string newlineInvertString(const std::string &input);

namespace ImGui {
bool InputString(const char *label, std::string *strPtr,
                 size_t bufferSize = 256, ImGuiInputTextFlags flags = 0);
} // namespace ImGui

void imguiDataPanel(PropertyData data);

void setOutputCode(int code);
