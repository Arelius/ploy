// Copyright (c) 2009, Nicholas "Indy" Ray. All rights reserved.
// See the LICENSE file for usage, modification, and distribution terms.
#include "types.h"
#include "symbol.h"
#include "typeinfo.h"
#include <assert.h>
#include <cstdio>

//TODO: allow compile time code transformation by appending source data (strings, and line info to the type data.

//TODO: Dynamic build this too
// It is essential that this is orderd in the same way the enums are numbered.
static const dynamic_type TypeList[DT_Invalid] =
{
    {DT_Pair, sizeof(pair), NULL},
    {DT_Symbol, sizeof(symbol), NULL},
    {DT_Int, sizeof(int), NULL},
    {DT_Real, sizeof(float), NULL},
    {DT_String, invalid_size, NULL},
    {DT_Char, sizeof(char), NULL},
    {DT_TypeInfo, sizeof(type_info_size), typeinfo_finish}
};

const dynamic_type* get_type(dynamic_types typeId)
{
    assert(typeId < DT_Invalid);
    return &TypeList[typeId];
}

pointer ploy_alloc(const dynamic_type* type, size_t size)
{
    assert(size != invalid_size || type->AllocSize != invalid_size);

    size_t alloc_size = size;
    if(alloc_size == invalid_size)
        alloc_size = type->AllocSize;

    alloc_size += sizeof(dynamic_types);

    void* alloc = malloc(alloc_size);
    memset(alloc, 0, alloc_size); // Remove if we implement type initializers
    *((dynamic_types*)alloc) = type->Id;

    return (pointer)alloc;
}

pointer ploy_alloc(const dynamic_type* type)
{
    return ploy_alloc(type, invalid_size);
}

pointer ploy_static_alloc(const dynamic_type* type, size_t size)
{
    pointer P = ploy_alloc(type, size);
    *((dynamic_types*)P) = (dynamic_types)(type->Id | DT_Static_Flag);
    return P;
}

pointer ploy_static_alloc(const dynamic_type* type)
{
    return ploy_static_alloc(type, invalid_size);
}

void ploy_free(pointer P)
{
    if(P == NIL ||
       *(dynamic_types*)P & DT_Static_Flag)
        return; // Abort free for static or NIL objects.
    const dynamic_type* T = get_type(get_type_id(P));
    if(T->finish)
        T->finish(P);
    free(P);
}

dynamic_types get_type_id(pointer P)
{
    return (dynamic_types)((*(dynamic_types*)P) & ~DT_Static_Flag);
}

bool is_type(pointer P, dynamic_types type)
{
    return (P != NIL) && (get_type_id(P) == type);
}

void* get_value(pointer P)
{
    return (char*)P+sizeof(dynamic_types);
}

pair* get_pair(pointer P)
{
    assert(is_type(P, DT_Pair));
    return (pair*)get_value(P);
}

pointer create_pair(pointer car, pointer cdr)
{
    pointer ret = ploy_alloc(get_type(DT_Pair));

    pair* value = get_pair(ret);

    value->_car = car;
    value->_cdr = cdr;

    return ret;
}

pointer pair_car(pointer P)
{
    assert(is_type(P, DT_Pair));
    return get_pair(P)->_car;
}

pointer pair_cdr(pointer P)
{
    assert(is_type(P, DT_Pair));
    return get_pair(P)->_cdr;
}

pointer* ref_car(pointer P)
{
    assert(is_type(P, DT_Pair));
    return &get_pair(P)->_car;
}

pointer* ref_cdr(pointer P)
{
    assert(is_type(P, DT_Pair));
    return &get_pair(P)->_cdr;
}

// we don't have any complicated trees atm, so we can just free the old when we set
// watch out though, it'll only free the first link in a list.
// also not in the main header so we don't overuse.

void set_car(pointer P, pointer V)
{
    assert(is_type(P, DT_Pair));
    ploy_free(get_pair(P)->_car);
    get_pair(P)->_car = V;
}

void set_cdr(pointer P, pointer V)
{
    assert(is_type(P, DT_Pair));
    ploy_free(get_pair(P)->_cdr);
    get_pair(P)->_cdr = V;
}

void clear_car(pointer P)
{
    assert(is_type(P, DT_Pair));
    get_pair(P)->_car = NIL;
}

void clear_cdr(pointer P)
{
    assert(is_type(P, DT_Pair));
    get_pair(P)->_cdr = NIL;
}

void append_in_place(pointer P, pointer V)
{
    assert(is_type(P, DT_Pair));
    if(cdr(P) == NIL)
        set_cdr(P, V);
    else
        append_in_place(cdr(P), V);
}

symbol* get_symbol(pointer P)
{
    assert(is_type(P, DT_Symbol));
    return (symbol*)get_value(P);
}

pointer create_symbol(symbol_table* tbl, const char* sym)
{
    pointer ret = ploy_alloc(get_type(DT_Symbol));

    *get_symbol(ret) = symbol_from_string(tbl, sym);

    return ret;
}

pointer create_symbol(symbol_table* tbl, const char* sym, size_t len)
{
    pointer ret = ploy_alloc(get_type(DT_Symbol));

    *get_symbol(ret) = symbol_from_string(tbl, sym, len);

    return ret;
}

int* get_int_ref(pointer P)
{
    return (int*)get_value(P);
}

int get_int(pointer P)
{

    return *get_int_ref(P);
}

pointer create_int(int i)
{
    pointer ret = ploy_alloc(get_type(DT_Int));

    *get_int_ref(ret) = i;

    return ret;
}

float* get_real_ref(pointer P)
{
    assert(is_type(P, DT_Real));
    return (float*)get_value(P);
}

float get_real(pointer P)
{
    return *get_real_ref(P);
}

pointer create_real(float f)
{
    pointer ret = ploy_alloc(get_type(DT_Real));

    *get_real_ref(ret) = f;

    return ret;
}

char* get_char_ref(pointer P)
{
    assert(is_type(P, DT_Char));
    return (char*)get_value(P);
}

char get_char(pointer P)
{
    return *get_char_ref(P);
}

pointer create_char(char c)
{
    pointer ret = ploy_alloc(get_type(DT_Char));

    *get_char_ref(ret) = c;

    return ret;
}

char* get_string_ref(pointer P)
{
    assert(is_type(P, DT_String));
    return (char*)get_value(P);
}

const char* get_string(pointer P)
{
    return get_string_ref(P);
}

pointer alloc_string(size_t len)
{
    pointer ret = ploy_alloc(get_type(DT_String), len+1);
    (get_string_ref(ret))[0] = '\0'; // Unneeded if everywhere initializes it directlly.
    return ret;
}

pointer create_string(const char* str)
{
    return create_string(str, strlen(str));
}

pointer create_string(const char* str, size_t len)
{
    pointer ret = ploy_alloc(get_type(DT_String), len+1);

    strncpy(get_string_ref(ret), str, len);

    (get_string_ref(ret))[len+1] = '\0'; // not strictly nessacary while we are zeroing the mem, but for if/when we remove it.

    return ret;
}

void destroy_list(pointer P)
{
    if(is_type(P, DT_Pair))
    {
        if(pair_car(P) != NIL)
            destroy_list(pair_car(P));
        if(pair_cdr(P) != NIL)
            destroy_list(pair_cdr(P));
    }
    ploy_free(P);
}

void print_object(pointer P, symbol_table* table)
{
    FILE* out = stdout;
    if(P == NIL)
    {
        fputs("NIL", out);
        return;
    }

    switch(get_type_id(P))
    {
    case DT_Pair:
        if(is_type(pair_car(P), DT_Pair))
        {
            putc('(', out);
            print_object(pair_car(P), table);
            putc(')', out);
        }
        else
            print_object(pair_car(P), table);

        putc(' ', out);
        if(pair_cdr(P) != NIL)
        {
            if(!is_type(pair_cdr(P), DT_Pair))
                fputs(". ", out);

            print_object(pair_cdr(P), table);
        }
        break;
    case DT_Symbol:
        fputs(string_from_symbol(table, *get_symbol(P)), out);
        break;
    case DT_Int:
        fprintf(out, "%d", get_int(P));
        break;
    case DT_Real:
        fprintf(out, "%f", get_real(P));
        break;
    case DT_String:
        fputs(get_string(P), out);
        break;
    case DT_Char:
        putc(get_char(P), out);
        break;
    case DT_TypeInfo:
        print_typeinfo(P, table, out);
        break;
    case DT_Invalid:
        fputs("#INVALID#", out);
        break;
    case DT_Any:
        fputs("#ANY#", out);
        break;
    }
}
