cmake_minimum_required(VERSION 3.12)
project(SecuredCam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(OpenCV REQUIRED)

set(FAISS_ROOT "/opt/homebrew/opt/faiss")
set(FAISS_INCLUDE_DIRS "${FAISS_ROOT}/include")
set(FAISS_LIBRARIES "${FAISS_ROOT}/lib/libfaiss.dylib")

include_directories(${FAISS_INCLUDE_DIRS})

set(SOURCES
    src/face_detector.cpp
    src/face_recognizer.cpp
    src/vector_db.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${FAISS_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
