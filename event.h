#pragma once

#ifndef EVENT_H_
#define EVENT_H_


#include "object.h"
#include "reaction.h"


// event 通过 obj_pos和event_type 定位哪个团簇的哪个反应。
class Event {
public:
	/**************members*****************/
	double rate; // rate为当前事件在总事件列表中的位置
	int obj_pos; // pos 为 obj在obj_ptr_list 中的具体位置
	int event_type; //-1~insert_cascade, 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation, 6~trap_mutation
};

/*************************普通函数***************************/
// 返回rate_sum
double build_event_list(std::vector<Object*>& _obj_ptr_list, std::vector<Event>& _event_list, const Setting& _setting);

//二分寻找 _event_list 中的某个反应， 返回该event对象
int find_event(std::vector<Event>& _event_list, double _rate_sum);

// 输入反应类型和obj_pos, 执行反应。
// t_m放到reaction的结合中，不放进 event_list， 如果满足判断条件立即发生， 并将刷新事件列表 重置 true； 
// t_m和四种发射均需要重置event_list, 迁移如果碰撞需要重置，
void carry_out_event(int _event_type, int _obj_pos, std::vector<Object*>& _obj_ptr_list,
	bool& _flag_refresh_event_list, const Database& _database, const Setting& _setting);

//  mig, 如果有团簇变化， _flag_refresh_event_list=true
void carry_out_mig(int _obj_pos, std::vector<Object*>& _obj_ptr_list,
	bool& _flag_refresh_event_list, const Database& _database, const Setting& _setting);

void carry_out_emit(int _emit_element, std::vector<Object*>& _obj_ptr_list, Object& new_obj, int _obj_pos, 
	const Database& _database, const Setting& _setting);

// 周围radius处发射sia一个，原地的V+1
void carry_out_tm(std::vector<Object*>& _obj_ptr_list, Object& new_obj, int _obj_pos,
	const Database& _database, const Setting& _setting);

std::string choose_cascade_to_insert(Setting& _set);

bool check_boundary(double _x, double _y, double _z, const Setting& _set);

void insert_obj(int _insert_element, std::vector<Object*>& _obj_ptr_list, const Database& _database, const Setting& _setting);
#endif