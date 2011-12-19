#pragma once
#include "ApolloCommon.h"

// 
// This is a memory allocator tailored towards ray tracing.  Objects
// tend to have well defined life times (per scene, per ray, etc) so allocation
// and destruction can be batched.
// This is also great for memory management since these pointers are effectively 
// garbage collected.
//
// The expected usage is that a highligh object (i.e. Scene) contains an allocator
// and exposes it.  Other objects that have a lifetime associated with the scene
// (i.e. Camera) will be allocated from the scene's allocator.  When the scene
// is destroyed, all of it's associated objects are destroyed as well
//

namespace Apollo {
    class PoolAllocator {
    public:
         template <typename T>
         T* Allocate();
         
         template <typename T>
         void Add(T*);

         // Free all memory associated with this alloctor
         void FreeAll();
    
         PoolAllocator() {}
         ~PoolAllocator() { FreeAll(); }

    private:
        struct GenericElement {
            virtual ~GenericElement() {}
        };
        template<class T>
        struct TypedElement : GenericElement {
            TypedElement(T* t) : t(t) {}  
            ~TypedElement() { delete t; }
            T* t;
        };
        std::vector<GenericElement*> m_elements;

        // Disable assignment and copy
        PoolAllocator(const PoolAllocator&);
        PoolAllocator& operator=(const PoolAllocator&);
    };

    template <typename T>
    inline T* PoolAllocator::Allocate() {
        T* value = new T;
        Add<T>(value);
        return value;
    }

    template <typename T>
    inline void PoolAllocator::Add(T* t) {
        m_elements.push_back(new TypedElement<T>(t));
    }

    inline void PoolAllocator::FreeAll() {
        for (std::vector<GenericElement*>::iterator iter = m_elements.begin();
            iter != m_elements.end();
            ++iter) {
            delete *iter;
        }
    }
}
