set(SRCROOT "${PROJECT_SOURCE_DIR}/src/common")

set(SRC
    ${SRCROOT}/public.h
    ${SRCROOT}/refcounted.cpp
    ${SRCROOT}/refcounted.h
    ${SRCROOT}/intrusive_ptr.cpp
    ${SRCROOT}/intrusive_ptr.h
    ${SRCROOT}/atomic_intrusive_ptr.cpp
    ${SRCROOT}/atomic_intrusive_ptr.h
    ${SRCROOT}/weak_ptr.cpp
    ${SRCROOT}/weak_ptr.h
    ${SRCROOT}/threadpool.cpp
    ${SRCROOT}/threadpool.h
    ${SRCROOT}/periodic_executor.cpp
    ${SRCROOT}/periodic_executor.h
    ${SRCROOT}/getopts.cpp
    ${SRCROOT}/getopts.h
    ${SRCROOT}/format.cpp
    ${SRCROOT}/format.h
    ${SRCROOT}/logging.cpp
    ${SRCROOT}/logging.h
    ${SRCROOT}/exception.cpp
    ${SRCROOT}/exception.h
    ${SRCROOT}/json.cpp
    ${SRCROOT}/json.h
    ${SRCROOT}/config.cpp
    ${SRCROOT}/config.h

)

add_library(common STATIC ${SRC})

target_link_libraries(common)

target_include_directories(common PUBLIC 
    ${PROJECT_SOURCE_DIR}/src
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
