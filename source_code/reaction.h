#pragma once

#ifndef REACTION_H_
#define REACTION_H_

#include"object.h"
#include"output.h"	//����reaction֮����Ҫ����Output�еĺ���������"react.txt"


/***************************************����������***************************************/

// ���ݱ߽������ж�pos1��pos2���Ƿ�����Ӧ
bool check_dist(std::vector<Object*>& _obj_ptr_list, int& pos1, int& pos2, const Setting& _set);

// �����ֱ߽���������ѭ����װ���˺����� 
bool check_dist_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set);

// 3free�߽�����
bool check_dist_3free_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set);

//zfree �߽�����
bool check_dist_zfree_inner_loop(const std::vector<Object*>& _obj_ptr_list,const int& pos1, int& pos2, const Setting& _set);

//�����ͱ߽�����
bool check_dist_0free_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2,const Setting& _set);


//����pos1��pos2ָ�������object
//���������ȫ������̬����һ���µ�oject,����ָ��ѹ��obj_ptr_list
//ɾ��������Ӧobject���ͷ����ǵ��ڴ棬����ָ�����ǵ�ָ���vector��ȥ��
//�ú�������ı�pos1,pos2��ֵ
void carry_out_reaction(std::vector<Object*>& _obj_ptr_list, int& _pos1, const int& _pos2, Object& _obj1, Object& _obj2,
	const Database& _database,  const Setting& _set);


//����x, y, z, size, size, dir�������ϲ���Ӧ�е���object
void create_object(Object& _obj1, Object& _obj2, std::vector<Object*>& _obj_ptr_list,
	const Database& _database, const Setting& _set);

bool check_boundary(double _x, double _y, double _z, const Setting& _set); 


//������object��λ��,Ŀǰ��total_size ��Ȩƽ��
void generate_new_pos(Object& _obj1, Object& _obj2, std::array<double, 3>& _new_pos);


#endif // !REACTION_H_
