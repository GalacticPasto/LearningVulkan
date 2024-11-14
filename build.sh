#!/bin/bash

DEFINES="-std=c++17 -O2"
LDFLAGS="-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi"

set echo on 
if [ -f "build/VulkanTest" ]; then 
    rm -rf build/VulkanTest    
fi 
if [ -f "build/frag.spv" ]; then 
    rm -rf build/frag.spv    
fi 
if [ -f "build/vert.spv" ]; then 
    rm -rf build/vert.spv    
fi 

glslc src/shader.vert -o build/vert.spv
glslc src/shader.frag -o build/frag.spv

g++ $CFLAGS src/main.cpp -o build/VulkanTest $LDFLAGS

