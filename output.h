#pragma once

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include<ctime>
#include"object.h"
#include"setting.h"

void initialize_output_file(std::string _filename);

void initialize_react_file();

// �ڹ̶���ʱ�̣�������object����Ϣ����dump����ʽ���
void output_dump(const std::vector<Object*>& _obj_ptr_list, const Setting& _set, bool _initialize);

//�ڹ̶�ʱ�̣���vac.txt, sia.txt, fia.txt, vac_fia.txt, sia_fia.txt�������ӦObject�ĳߴ���Ϣ
void output_txt(const std::vector<Object*>& _obj_ptr_list, const Setting& _set, bool _initialize);

//���ĳһ֡��cascade
void output_cascade(const std::vector<Object*>& _obj_ptr_list, const Setting& _set);

void write_content(std::string _filename, std::ostringstream& _oss, bool);

//cascade insert
void output_react_0_0(const Setting& _set, std::string str);

//obj reach bound in absorb boundary condition
void output_react_1_0(const Object& _obj1, const Setting& _set);

//obj����������
void output_react_1_0_GB(const Object& _obj1, const Setting& _set);

//obj��λ������
void output_react_1_0_dislocation(const Object& _obj1, const Setting& _set);

//����Object��ȫ���𣬲���0����object
//��������object������Ӧ��Ϣд��"react.txt"
void output_react_2_0(const Object& _obj1, const Object& _obj2, const Setting& _set);

// obj1 + obj2 = obj3
void output_react_2_1(const Object& _obj1, const Object& _obj2, const Object& _obj3, const Setting& _set);

//obj1 -> obj2 + obj3
void output_react_1_2(const Object& _obj1, const Object& _obj2, const Object& _obj3, const Setting& _set);

// trap mutation
void output_trap_mutation(const Object& _obj1, const Object& _obj2, const Object& _obj3, const Setting& _set);

inline double cal_duration();


// ��������cpuʱ��
double cal_duration()
{
	return (double)clock() / CLOCKS_PER_SEC;
}

#endif // !OUTPUT_H_
