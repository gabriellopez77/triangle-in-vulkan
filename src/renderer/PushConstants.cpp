#include "PushConstants.h"

void rk::PushConstants::create(u32 size, i32 stage) {
    m_pushConstantInfo.offset = 0;
    m_pushConstantInfo.size = size;
    m_pushConstantInfo.stageFlags = stage;
}
