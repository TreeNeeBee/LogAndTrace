#include <gtest/gtest.h>
#include <lap/core/CInitialization.hpp>

int main(int argc, char **argv) {
    // Initialize Core module
    auto initResult = lap::core::Initialize();
    if (!initResult.HasValue()) {
        return 1;
    }
    
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    
    // Deinitialize Core module
    lap::core::Deinitialize();
    
    return result;
}
