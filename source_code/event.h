#pragma once

#ifndef EVENT_H_
#define EVENT_H_


#include "object.h"
#include "reaction.h"


// event ͨ�� obj_pos��event_type ��λ�ĸ��Ŵص��ĸ���Ӧ��
class Event {
public:
	/**************members*****************/
	double rate; // rateΪ��ǰ�¼������¼��б��е�λ��
	int obj_pos; // pos Ϊ obj��obj_ptr_list �еľ���λ��
	int event_type; //-1~insert_cascade, 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation, 6~trap_mutation
};

/*************************��ͨ����***************************/
// ����rate_sum
double build_event_list(std::vector<Object*>& _obj_ptr_list, std::vector<Event>& _event_list, const Setting& _setting);

//����Ѱ�� _event_list �е�ĳ����Ӧ�� ���ظ�event����
int find_event(std::vector<Event>& _event_list, double _rate_sum);

// ���뷴Ӧ���ͺ�obj_pos, ִ�з�Ӧ��
// t_m�ŵ�reaction�Ľ���У����Ž� event_list�� ��������ж��������������� ����ˢ���¼��б� ���� true�� 
// t_m�����ַ������Ҫ����event_list, Ǩ�������ײ��Ҫ���ã�
void carry_out_event(int _event_type, int _obj_pos, std::vector<Object*>& _obj_ptr_list,
	bool& _flag_refresh_event_list, const Database& _database, const Setting& _setting);

//  mig, ������Ŵر仯�� _flag_refresh_event_list=true
void carry_out_mig(int _obj_pos, std::vector<Object*>& _obj_ptr_list,
	bool& _flag_refresh_event_list, const Database& _database, const Setting& _setting);

void carry_out_emit(int _emit_element, std::vector<Object*>& _obj_ptr_list, Object& new_obj, int _obj_pos, 
	const Database& _database, const Setting& _setting);

// ��Χradius������siaһ����ԭ�ص�V+1
void carry_out_tm(std::vector<Object*>& _obj_ptr_list, Object& new_obj, int _obj_pos,
	const Database& _database, const Setting& _setting);

std::string choose_cascade_to_insert(Setting& _set);

bool check_boundary(double _x, double _y, double _z, const Setting& _set);

void insert_obj(int _insert_element, std::vector<Object*>& _obj_ptr_list, const Database& _database, const Setting& _setting);
#endif