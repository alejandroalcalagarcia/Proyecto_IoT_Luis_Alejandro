cmake_minimum_required(VERSION 3.16)

# Añadimos esta línea para que encuentre la librería de ejemplos
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_c3_lcdkit)