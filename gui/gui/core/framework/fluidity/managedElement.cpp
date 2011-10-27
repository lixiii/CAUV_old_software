#include "managedElement.h"

using namespace cauv;
using namespace cauv::gui::f;

ManagedElement::ManagedElement(ManagedElement const& other)
    : m_manager(other.m_manager){
}

ManagedElement::ManagedElement(Manager& manager)
    : m_manager(manager){
}

Manager& ManagedElement::manager(){
    return m_manager;
}
        
