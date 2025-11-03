#include <gtest/gtest.h>
#include <core/CMemory.hpp>

int main(int argc, char **argv) {
    // 首先调用 MemManager::getInstance() 确保它先于 LogManager 构造
    // 这样 MemManager 会后于 LogManager 析构，避免在 Sink 析构时调用已析构的 MemManager
    lap::core::MemManager::getInstance();
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
