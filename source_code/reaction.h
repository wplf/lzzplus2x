#pragma once

#ifndef REACTION_H_
#define REACTION_H_

#include"object.h"
#include"output.h"	//发生reaction之后，需要调用Output中的函数，更新"react.txt"


/***************************************函数声明区***************************************/

// 根据边界条件判断pos1与pos2处是否发生反应
bool check_dist(std::vector<Object*>& _obj_ptr_list, int& pos1, int& pos2, const Setting& _set);

// 将三种边界条件的内循环封装到此函数内 
bool check_dist_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set);

// 3free边界条件
bool check_dist_3free_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set);

//zfree 边界条件
bool check_dist_zfree_inner_loop(const std::vector<Object*>& _obj_ptr_list,const int& pos1, int& pos2, const Setting& _set);

//周期型边界条件
bool check_dist_0free_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2,const Setting& _set);


//输入pos1和pos2指向的两个object
//如果不是完全湮灭，则动态生成一个新的oject,并将指针压入obj_ptr_list
//删除两个反应object，释放它们的内存，并将指向它们的指针从vector中去除
//该函数不会改变pos1,pos2的值
void carry_out_reaction(std::vector<Object*>& _obj_ptr_list, int& _pos1, const int& _pos2, Object& _obj1, Object& _obj2,
	const Database& _database,  const Setting& _set);


//根据x, y, z, size, size, dir，创建合并反应中的新object
void create_object(Object& _obj1, Object& _obj2, std::vector<Object*>& _obj_ptr_list,
	const Database& _database, const Setting& _set);

bool check_boundary(double _x, double _y, double _z, const Setting& _set); 


//计算新object的位置,目前用total_size 加权平均
void generate_new_pos(Object& _obj1, Object& _obj2, std::array<double, 3>& _new_pos);


#endif // !REACTION_H_
