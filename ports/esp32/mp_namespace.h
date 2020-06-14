/*
 * mp_namespace.h
 *
 *  Created on: Jun 14, 2020
 *      Author: ondiiik
 */
#pragma once



#define MP_NAMESPACE2(_parent_, _member_)              mpy__##_parent_##__##_member_
#define MP_NAMESPACE3(_parent_, _member_, _submember_) mpy__##_parent_##__##_member_##__##_submember_


#define MP_MODULE_BEGIN(_name_) \
    const mp_rom_map_elem_t mpy__##_name_##_members_table[] = \
    { \
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_##_name_) },
        
        
#define MP_MODULE_END(_name_) \
    }; \
    \
    STATIC MP_DEFINE_CONST_DICT(_name_##_members, mpy__##_name_##_members_table); \
    \
    const mp_obj_module_t mp_module_##_name_ = \
    { \
        .base    = { &mp_type_module }, \
        .globals = (mp_obj_dict_t*)&_name_##_members, \
    };


#define MP_CLASS_BEGIN(_parent_,_name_) \
    STATIC const mp_rom_map_elem_t mpy__##_parent_##__##_name_##_T[]; \
    \
    const mp_obj_type_t mpy__##_parent_##__##_name_ = \
    { \
        { &mp_type_type }, \
        .name        =                  MP_QSTR_##_name_, \
        .make_new    =                  mpy__##_parent_##__##_name_##____init__, \
        .print       =                  mpy__##_parent_##__##_name_##____str__, \
        .attr        =                  mpy__##_parent_##__##_name_##____property__, \
        .locals_dict = (mp_obj_dict_t*)&mpy__##_parent_##__##_name_##_T \
    }; \
    \
    STATIC const mp_rom_map_elem_t mpy__##_parent_##__##_name_##_T[] = \
    { \
        { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_##_name_) },


#define MP_CLASS_END };


#define MP_MEMBER(_parent_, _member_) \
    { MP_ROM_QSTR(MP_QSTR_##_member_), MP_ROM_PTR(&mpy__##_parent_##__##_member_) },


#define MP_FN_0(_parent_, _member_) \
    STATIC mp_obj_t                                                 mpy__##_parent_##__##_member_##_##_F(); \
    STATIC MP_DEFINE_CONST_FUN_OBJ_0(mpy__##_parent_##__##_member_, mpy__##_parent_##__##_member_##_##_F); \
    mp_obj_t                                                        mpy__##_parent_##__##_member_##_##_F()
