# ZipZap additions are optional and enabled by default in this fork. Core
# loader hooks compile out when disabled; no runtime registration is needed.
option(SK_ZIPZAP_EXTENSIONS "Build ZipZap native ASTC6 loading and texture format preflight" ON)

if (SK_ZIPZAP_EXTENSIONS)
  target_compile_definitions(StereoKitC PUBLIC SK_ZIPZAP_EXTENSIONS)
  target_sources(StereoKitC PRIVATE
    StereoKitC/zipzap/texture_api.h
    StereoKitC/zipzap/texture_format_support.cpp
    StereoKitC/zipzap/astc6_loader.h
    StereoKitC/zipzap/astc6_loader.cpp
    StereoKitC/zipzap/ktx2_astc6.h
    StereoKitC/zipzap/ktx2_astc6.cpp)

  if (SK_BUILD_TESTS AND NOT ANDROID)
    enable_testing()
    add_executable(SKKtx2Tests tools/tests/ktx2_astc6_tests.cpp StereoKitC/zipzap/ktx2_astc6.cpp)
    set_target_properties(SKKtx2Tests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
    target_link_libraries(SKKtx2Tests PRIVATE zstd_decompress)
    target_compile_definitions(SKKtx2Tests PRIVATE SK_ASTC6_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/tools/tests/fixtures/astc6")
    add_test(NAME SKKtx2Tests COMMAND SKKtx2Tests)
  endif()
endif()

# Keep the loader suite in both modes: with extensions disabled it must reject
# native ASTC6 without breaking portable UASTC or the upstream test runner.
if (TARGET SKTests)
  target_sources(SKTests PRIVATE tools/tests/astc6_loader_tests.h tools/tests/astc6_loader_tests.cpp)
  target_compile_definitions(SKTests PRIVATE SK_ASTC6_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/tools/tests/fixtures/astc6")
  enable_testing()
  add_test(NAME ZipZapTextureLoader COMMAND SKTests -astc6test)
endif()
