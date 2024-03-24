#!/bin/bash

# Folder path containing header files
FOLDER_PATH="/home/ethanscharlie/Projects/Code/C++/CharlieGamesv2/testGame"

# Create a temporary include file
INCLUDE_FILE="include/include_tmp.h"

# Create or truncate the temporary include file
echo "" > "$INCLUDE_FILE"

# Iterate over header files and generate #include directives
for header_file in "$FOLDER_PATH"/src/*.h; do
    echo "#include \"$header_file\"" >> "$INCLUDE_FILE"
done

while true; do
    # Clean and create the build directory
    cd $FOLDER_PATH
    rm -rf build
    mkdir -p build
    cd build || exit 1

    # Generate CMake cache with additional include directory
    if cmake -DPROJECT_PATH=$FOLDER_PATH -DCMAKE_INCLUDE_PATH="../$INCLUDE_DIR/src" /home/ethanscharlie/Projects/Code/C++/CharlieGames/Charlie2DEditor && cmake --build . && make; then
        # Run the executable
        $FOLDER_PATH/build/index
        exit_code=$?

        # Handle exit code
        if [ $exit_code -eq 42 ]; then
            echo "Restarting the editor..."
        else
            echo "Exiting the editor."
            break
        fi
    else
        # Build failed, exit loop
        echo "Build failed, exiting the editor."
        break
    fi

    cd ..

done
