# Standalone Windows utility; intentionally not linked to the CS2 cheat target.
if(WIN32)
  add_executable(VestaSkinLoadoutManager WIN32 main.cpp)
  target_compile_features(VestaSkinLoadoutManager PRIVATE cxx_std_20)
  target_compile_definitions(VestaSkinLoadoutManager PRIVATE UNICODE _UNICODE)
  target_link_libraries(VestaSkinLoadoutManager PRIVATE comctl32 comdlg32)
endif()
