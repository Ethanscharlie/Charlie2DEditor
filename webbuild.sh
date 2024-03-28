source ~/emsdk/emsdk_env.sh

if [ -f "prevProject.txt" ]; then
  FOLDER_PATH=$(<prevProject.txt)
else
    echo "File not found."
fi

rm -rf build && mkdir build && cd build && emcmake cmake -DPROJECT_PATH=$FOLDER_PATH -DCMAKE_INCLUDE_PATH="../$INCLUDE_DIR/src"  .. && emmake make

