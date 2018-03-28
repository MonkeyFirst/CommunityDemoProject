set (TARGET_NAME go)

file (GLOB CPP_FILES ./IMUI/imgui/*.cpp ./IMUI/*.cpp ./Cfg/*.cpp ./States/*.cpp ./*.cpp ./Motions/*cpp ./Objects/*cpp ./Camera/*cpp ./Character/*cpp ./Tools/*cpp)
file (GLOB H_FILES ./IMUI/imgui/*.h ./IMUI/*.h ./Cfg/*.h ./States/*.h ./*.h ./Motions/*h ./Objects/*h ./Camera/*h ./Character/*h ./Tools/*h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

#set (INCLUDE_DIRS ./ ./IMUI/imgui ./IMUI ./Cfg ./States ./Motions ./Objects ./Camera ./Character ./Tools)
set (INCLUDE_DIRS ./)

setup_main_executable ()