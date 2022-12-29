#pragma once

#ifndef OBJECT_H_
#define OBJECT_H_

#include<vector>
#include <array>
#include<iostream>
#include<algorithm>
#include<cmath>
#include<set>
#include<map>
#include"database.h"
#include"setting.h"
#include"random.h"


#define PI 3.1415926

// ����һ��obj�Ĺؼ��������Ϊ  key_information �� position_vector



// һ��obj�Ĺؼ�����Ϊ�� ����ӵ�еĲ���Ϊ��
// m_label, m_possib, m_kind, m_size, m_position, m_site

class Object {
public:
	/********************************members****************************************/
	// ��̬����
	static size_t ID;
	static long he_insert_num;
	static long t_m_num;
	static long annihilation_num;
	//1~V---; 2~-W--; 3~--Be-; 4~---He; 5~VW--; 6~V-Be-; 7~V--He; 8~-WBe-;
	//9~-W-He; 10~--BeHe; 11~VWBe-; 12~VW-He; 13~V-BeHe; 14~-WBeHe; 15~VWBeHe;
	int type = 0;
	size_t id = 0;		//Ψһ��ǩ
	std::array<double, 3> pos;
	std::array<int, 3> grid_pos; //��¼object���ĸ�����
	std::array<int, 3> tis_quarter_half_int_pos;
	std::array<double, 3> displacement; //  calculate the displacement to check whether it is out of grain.
	std::array<double, 7> energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation,  6~t_m
	// key->value,   // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation, 6~t_m
	std::map<int, double> frequency;
	std::array<int, 4> size; // 0~V 1~SIA 2~Be 3~He
	double radius = 0;
	//������λ�����յĸ���
	double p_meet_dislocation = 0;
//0	F_mig		5	F2		10	E_mig3		15	site
//1	E_mig		6	E_bind2		11	F4		16	F_rot
//2	F1		7	E_mig2		12	E_bind4		17	E_rot
//3	E_bind1		8	F3		13	E_mig4		18	Ftm
//4	E_mig1		9	E_bind3		14	radius		19	Etm
	std::array<double, 20> property_array;
	// 0~bcc, 1~tis, 2~ois, 3~fcc
	int occupying_site = 0; 
	int dir = 0; // no direction ~ 0;  bcc: 111~1, -111~2, 1-11~3, 11-1~4;

	/********************************functions****************************************/

	//// �������캯��, ֻ�����emit��Ӧʱʹ��
	//Object(Object* _obj);

	Object(int _type, double x, double y, double z, int _size0, int _size1, int _size2, int _size3, int _dir, 
		const  Database& _possibility_map, const Setting& _setting);

	bool refresh_type();

	void refresh_site(const Database& _database, const Setting& _setting);

	bool refresh_obj(const Database& _database, const Setting& _setting);

	void refresh_property(const Database& _database, const Setting& _setting);

	void parameterize();

	void refresh_radius(const Database& _database, const Setting& _setting);

	void move2bcc(const Setting& _setting);

	void move2tis(const Setting& _setting);

	void move2ois(const Setting& _setting);

	void cal_frequency(const Database& _database, const Setting& _setting);

	~Object() {};					//��������

	//����object�������ĸ�����
	inline void refresh_grid_pos(const Setting& _set);
	inline void check_pbc(const Setting& _setting);
	inline bool check_zfree(const Setting& _setting);
	inline bool check_3free(const Setting& _setting);
	inline bool check_grain_radius(const Setting& _setting);

	//switch ѡ��ǰobj��Ǩ�Ʒ�ʽ
	inline void mig(const Setting& _setting);
	inline void tis_mig(const Setting& _setting);
	inline void ois_mig(const Setting& _setting);
	inline void bcc_mig(const Setting& _setting);
	inline void sia_mig(const Setting& _setting);
	inline void sia_rotate();
	inline void swap(int& a, int& b);
};


/************************��ͨ����********************************/
int size2type(std::array<int,4> _size);

double cal_distance(std::array<double, 3> a, std::array<double, 3> b);

// ��cascade.txt��ÿһ������һ��object����̬�����ڴ棩����ָ������ָ�����obj_ptr_list��
void read_cascade(std::string _filename, std::vector<Object*>& _obj_ptr_list,
	const Database& _database, Setting& _set);

//��ת������˿ռ�����
void multiply(std::array<std::array<double, 3>, 3>& Rx, double& x, double& y, double& z);

/***********************��������********************************/

//����object������objectǨ�Ʋ������߽�����֮��ִ�У�����object���ڵ�����
void Object::refresh_grid_pos(const Setting& _set)
{
	grid_pos.at(0) = int(pos[0]) / _set.box_grid;
	grid_pos.at(1) = int(pos[1]) / _set.box_grid;
	grid_pos.at(2) = int(pos[2]) / _set.box_grid;
}

void Object::check_pbc(const Setting& _setting) {
	for (int i = 0; i < 3; ++i) {
		if (pos[i] < _setting.box_min[i]) {
			pos[i] += _setting.box_length[i];
		}
		else if (pos[i] >= _setting.box_max[i]) {
			pos[i] -= _setting.box_length[i];
		}
	}
}
// ���緵��true
bool Object::check_zfree(const Setting& _setting) {
	if (pos[2] < _setting.box_min[2] || pos[2] >= _setting.box_max[2]) {
		return true;
	}
	for (int i = 0; i < 2; ++i) {
		if (pos[i] < _setting.box_min[i]) {
			 pos[i] += _setting.box_length[i];
		}
		else if (pos[i] >= _setting.box_max[i]) {
			 pos[i] -= _setting.box_length[i];
		}
	}
	return false;
}

bool Object::check_3free(const Setting& _setting) {
	for (int i = 0; i < 3; ++i) {
		if (pos[i] < _setting.box_min[i] || pos[i] >= _setting.box_max[i]) return true;
	}
	return false;
}

bool Object::check_grain_radius(const Setting& _setting) {
	double journey = pow(displacement[0], 2) + pow(displacement[1], 2) + pow(displacement[2], 2);
	if (journey > _setting.grain_radius_square) return true;
	return false;
}

void Object::mig(const Setting& _setting) {
	switch (type) {
	case 2: //SIA�� ֻ��sia��һά��ɢ
		sia_mig(_setting);
		break;
	default:
		if (occupying_site == 0) {
			bcc_mig(_setting);
		}
		else if (occupying_site == 1) {
			tis_mig(_setting);
		}
		else if (occupying_site == 2) {
			ois_mig(_setting);
		}
		break;
	}
}

void Object::sia_mig(const Setting& _setting) {
	int rand_num = 0;
	for (int i = 0; i < _setting.mig_degeneration; ++i) {
		rand_num += uni() > 0.5 ? 1 : -1;
	}
	switch (dir) {
	case 1:
		pos[0] += 0.5 * rand_num;
		pos[1] += 0.5 * rand_num;
		pos[2] += 0.5 * rand_num;
		displacement[0] += 0.5 * rand_num;
		displacement[1] += 0.5 * rand_num;
		displacement[2] += 0.5 * rand_num;
		break;
	case 2:
		pos[0] -= 0.5 * rand_num;
		pos[1] += 0.5 * rand_num;
		pos[2] += 0.5 * rand_num;
		displacement[0] -= 0.5 * rand_num;
		displacement[1] += 0.5 * rand_num;
		displacement[2] += 0.5 * rand_num;
	case 3:
		pos[0] += 0.5 * rand_num;
		pos[1] -= 0.5 * rand_num;
		pos[2] += 0.5 * rand_num;
		displacement[0] += 0.5 * rand_num;
		displacement[1] -= 0.5 * rand_num;
		displacement[2] += 0.5 * rand_num;
	case 4:
		pos[0] += 0.5 * rand_num;
		pos[1] += 0.5 * rand_num;
		pos[2] -= 0.5 * rand_num;
		displacement[0] += 0.5 * rand_num;
		displacement[1] += 0.5 * rand_num;
		displacement[2] -= 0.5 * rand_num;
	}
}

void Object::bcc_mig(const Setting& _setting) {
	std::array<int, 3> rand_num = {0, 0, 0};
	for (int i = 0; i < _setting.mig_degeneration; ++i) {
		for (int j = 0; j < 3; ++j) {
			rand_num[j] += (uni() > 0.5 ? 1 : -1);
		}
	}
	for (int i = 0; i < 3; ++i) {
		pos[i] += rand_num[i] * 0.5;
		displacement[i] += rand_num[i] * 0.5;
	}
}

void Object::ois_mig(const Setting& _setting) {
	// 8������ �������붯�� ��������ֻ��ĳһ����
	// ��һ�����ҵ�����λ��
	// �ڶ���������ҵ���һ������λ������ȡ�� ���ƻ������飩
	// ������������ֱ�Ӽӡ�
	// 
	// ois �� ��һ��Ϊ����λ�� ��integer_pos �洢��
	std::array<double, 3> pre_pos = { pos[0], pos[1], pos[2] };

	int integer_pos = 0;
	for (int i = 0; i < 3; ++i) {
		if (floor(pos[i]) == pos[i]) {
			integer_pos = i;
			break;
		}
	}
	// �ڶ����ı��ֵͨ�� integer_pos + 1 | 2 ����3ȡ���á�
	int chosen_half = (integer_pos + ((uni() > 0.5) ? 1 : 2)) % 3;
	
	std::array<double, 3> rand_num = {0, 0, 0};

	for (int j = 0; j < _setting.mig_degeneration; j++) {
		rand_num[integer_pos] += uni() > 0.5 ? 1 : -1;
		rand_num[chosen_half] += uni() > 0.5 ? 1 : -1;
	}
	for (int i = 0; i < 3; ++i) {
		pos[i] += rand_num[i] * 0.5;
		displacement[i] += rand_num[i] * 0.5;
	}
}
/*ljl
void Object::tis_mig(const Setting& _setting) {
	occupying_site == 1
	// ֻ��4������� �������ɡ�
	for (int i = 0; i < _setting.mig_degeneration; i++) {
		int rand_num = (int)floor(uni() * 4);
		switch(rand_num){
		case 0: 
			pos[tis_quarter_half_int_pos[0]] = pos[tis_quarter_half_int_pos[1]];
			pos[tis_quarter_half_int_pos[1]] += 0.25;
			swap(tis_quarter_half_int_pos[0], tis_quarter_half_int_pos[1]);
			break;
		case 1:
			pos[tis_quarter_half_int_pos[0]] = pos[tis_quarter_half_int_pos[1]];
			pos[tis_quarter_half_int_pos[1]] -= 0.25;
			swap(tis_quarter_half_int_pos[0], tis_quarter_half_int_pos[1]);
			break;
		case 2:
			pos[tis_quarter_half_int_pos[0]] = pos[tis_quarter_half_int_pos[2]];
			pos[tis_quarter_half_int_pos[2]] += 0.25;
			swap(tis_quarter_half_int_pos[2], tis_quarter_half_int_pos[1]);
			break;
		case 3:
			pos[quarter_half_integer_pos[0]] = pos[quarter_half_integer_pos[2]];
			pos[quarter_half_integer_pos[2]] -= 0.25;
			swap(quarter_half_integer_pos[2], quarter_half_integer_pos[1]);
			break;
		}
	}
}
*/

// lzz
void Object::tis_mig(const Setting& _setting) {
	//Ǩ��mig_degeneration��
	for (int j = 0; j < _setting.mig_degeneration; j++)
	{
		double d1, d2;
		d1 = (uni() < 0.5) ? 0.25 : -0.25;
		d2 = (uni() < 0.5) ? 0.25 : -0.25;
		double changed_quarter = pos.at(tis_quarter_half_int_pos.at(0)) + d1;
		if ((changed_quarter - floor(changed_quarter)) < 0.1)	//���quarter�����int
		{
			pos.at(tis_quarter_half_int_pos.at(0)) += d1;
			pos.at(tis_quarter_half_int_pos.at(2)) += d2;	//��int��quarter
			swap(tis_quarter_half_int_pos.at(0), tis_quarter_half_int_pos.at(2));	//����int��quarter
		}
		else
			//���quarter�����half
		{
			pos.at(tis_quarter_half_int_pos.at(0)) += d1;
			pos.at(tis_quarter_half_int_pos.at(1)) += d2;	//��half��quarter
			swap(tis_quarter_half_int_pos.at(0), tis_quarter_half_int_pos.at(1));	//����half��quarter
		}
	}
}



void Object::swap(int&a, int &b) {
	int temp = a;
	a = b;
	b = temp;
}

void Object::sia_rotate() {
	int rand_num = int(floor(uni() * 3));
	std::array<int, 3> dir_choice = { 0, 0, 0 };
	switch (dir) {
	case 1:
		dir_choice = { 2, 3, 4 };
		break;
	case 2:
		dir_choice = { 1, 3, 4 };
		break;
	case 3:
		dir_choice = { 1, 2, 4 };
		break;
	case 4:
		dir_choice = { 1, 2, 3 };
		break;
	}
	if (rand_num == 0) dir = dir_choice[0];
	else if (rand_num == 1) dir = dir_choice[1];
	else if (rand_num == 2) dir = dir_choice[2];
}

#endif // !OBJECT_H_