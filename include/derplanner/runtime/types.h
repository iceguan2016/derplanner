//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef DERPLANNER_RUNTIME_TYPES_H_
#define DERPLANNER_RUNTIME_TYPES_H_

#include <stdint.h>
#include <stddef.h>

namespace plnnr {

// Type IDs supported by fact database.
enum Type
{
    Type_None = 0,
    #define PLNNR_TYPE(TYPE_TAG, TYPE_NAME) Type_##TYPE_TAG,
    #include "derplanner/runtime/type_tags.inl"
    #undef PLNNR_TYPE
    Type_Count,
};

enum { Max_Fact_Arity = 16 };

// Format of the fact tuple.
struct Fact_Type
{
    // number of parameters.
    uint8_t arity;
    // type of each parameter.
    Type param_type[Max_Fact_Arity];
};

// Weak pointer to a fact tuple in Fact_Database.
struct Fact_Handle
{
    // index of table in Fact_Database.
    uint64_t table      : 20;
    // index of entry in Fact_Table.
    uint64_t entry      : 20;
    // entry generation when this handle was obtained.
    uint64_t generation : 24;
};

// Collection of tuples of a single fact type.
struct Fact_Table
{
    // current number of entries stored.
    uint32_t        num_entries;
    // total allocated number of entries.
    uint32_t        max_entries;
    // linear block of memory accommodating entries' data.
    void*           blob;
    // tuples laid out in SOA order.
    void*           columns[Max_Fact_Arity];
    // generation per each entry to support weak handles.
    uint32_t*       generations;
};

// A set of fact tables.
struct Fact_Database
{
    // current number of tables.
    uint32_t        num_tables;
    // total allocated number of tables.
    uint32_t        max_tables;
    // fact name hash per each table.
    uint32_t*       hashes;
    // fact name per each table.
    const char**    names;
    // type (format) per each table.
    Fact_Type*      types;
    // table data.
    Fact_Table*     tables;
    // linear block of memory accommodating database data.
    void*           blob;
};

struct Planning_State;
struct Expansion_Frame;

// Generated expansion function prototype.
typedef bool Composite_Task_Expand(Planning_State*, Expansion_Frame*, Fact_Database*);

// Composite task case expansion state.
struct Expansion_Frame
{
    enum Flags
    {
        Flags_None      = 0x0,
        Flags_Expanded  = 0x1,
        Flags_Failed    = 0x2,
    };

    // in-progress/expanded/failed flags.
    uint32_t                flags               : 2;
    // composite task type this frame holds data for, can be used for lookups in `Task_Info`.
    uint32_t                task_type           : 15;
    // index of the expanding case.
    uint32_t                case_index          : 15;
    // stores the number of tasks on the task stack for rewinding.
    uint16_t                task_stack_rewind;
    // jump label to support coroutine-like expansion behaviour.
    uint16_t                expand_label;
    // jump label to support coroutine-like precondition iteration.
    uint16_t                precond_label;
    // number of fact database handles kept by the case precondition.
    uint16_t                num_handles;
    // the expand function of the expanding case.
    Composite_Task_Expand*  expand;
    // pointer to the composite task arguments.
    void*                   arguments;
    // pointer to the precondition outputs.
    void*                   precond_output;
    // fact database handles kept by the case precondition.
    Fact_Handle*            handles;
};

// Primitive (or composite) task in the plan.
struct Task_Frame
{
    // composite/primitive task type.
    uint32_t    task_type;
    // pointer to the task arguments.
    void*       arguments;
};

// Helper: fixed size stack.
template <typename T>
struct Stack
{
    uint32_t    size;
    uint32_t    max_size;
    T*          frames;
};

// Helper: fixed size linear block of memory.
struct Linear_Blob
{
    uint32_t    max_size;
    uint8_t*    top;
    uint8_t*    base;
};

// Planning state to support non-recursive planning.
struct Planning_State
{
    // expansion stack to support back-tracking
    Stack<Expansion_Frame>  expansion_stack;
    // the resulting plan is stored on this stack.
    Stack<Task_Frame>       task_stack;
    // composite task arguments and precondition state storage.
    Linear_Blob             expansion_blob;
    // task arguments storage.
    Linear_Blob             task_blob;
};

// Database construction parameters.
struct Database_Format
{
    // number of tables (fact types).
    uint32_t        num_tables;
    // size hint (max entries) per each table.
    uint32_t*       size_hints;
    // fact format.
    Fact_Type*      types;
    // fact name hash per each table.
    uint32_t*       hashes;
    // fact name per each table
    const char**    names;
};

// Runtime task information specified by generated domain code.
struct Task_Info
{
    // number of tasks in domain.
    uint32_t                num_tasks;
    // number of primitive tasks in domain.
    uint32_t                num_primitive;
    // task name hashes (composite tasks are specified after primitive).
    uint32_t*               hashes;
    // task names (composite tasks are specified after primitive).
    const char**            names;
    // format of task parameters.
    Fact_Type*              parameters;
    // pointer to generated expand function for each composite task.
    Composite_Task_Expand*  expands;
};

// Domain info provided by the generated code.
struct Domain_Info
{
    // generated tasks info.
    Task_Info           task_info;
    // required fact database format.
    Database_Format     database_req;
};

}

#endif
