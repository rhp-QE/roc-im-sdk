#include <random>

namespace roc::base::util {

inline uint32_t generate_uint32_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<uint32_t> dis(100000000, 4294967295);
    
    uint32_t random_num = dis(gen);

    return random_num;
}  

} // namespace roc::base::util