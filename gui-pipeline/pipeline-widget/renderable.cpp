#include "renderable.h"
#include "pipelineWidget.h"

Renderable::Renderable(container_ptr_t c, Point const& at)
    : m_pos(at), m_context(c){
}

Point Renderable::topLevelPos() const{
    return m_context->referUp(m_pos);
}

