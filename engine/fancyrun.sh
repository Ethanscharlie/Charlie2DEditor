#!/bin/bash

# Folder path containing header files
if [ -f "prevProject.txt" ]; then
  FOLDER_PATH=$(<prevProject.txt)
else
    echo "File not found."
fi

# Create a temporary include file
INCLUDE_FILE="include/include_tmp.h"

# Create or truncate the temporary include file
echo "" > "$INCLUDE_FILE"

# Iterate over header files and generate #include directives
for header_file in "$FOLDER_PATH"/src/*.h; do
  if [ -e "$header_file" ]; then
     # Include the header file in the include file
     echo "#include \"$header_file\"" >> "$INCLUDE_FILE"
  fi
done

while true; do
    # Clean and create the build directory
    rm -rf build
    mkdir -p build
    cd build || exit 1

    # Generate CMake cache with additional include directory
    if cmake -DPROJECT_PATH=$FOLDER_PATH -DCMAKE_INCLUDE_PATH="../$INCLUDE_DIR/src" .. && cmake --build . && make; then
        # Run the executable
        ./index $FOLDER_PATH
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
