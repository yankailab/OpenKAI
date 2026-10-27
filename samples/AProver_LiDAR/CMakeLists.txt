add_executable(AProver_LiDAR
  main.cpp
  _AProver_LiDAR.cpp
  _AProver_LiDAR.h
)
target_link_libraries(AProver_LiDAR PRIVATE OpenKAI::OpenKAI)
