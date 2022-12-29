#include "reaction.h"

// �����õķ�ʽ����pos1��pos2�� �õ���Ҫ��Ӧ������pos���ͷ��ص�boolֵ

// ����check_dist ���ܣ��������񻮷֣� �����ֱ߽��������ɵ��˺�����
// �ж����ֱ߽�����������
bool check_dist(std::vector<Object*>& _obj_ptr_list, int& _pos1, int& _pos2, const Setting& _set)
{
	for (; (unsigned int)_pos1 < _obj_ptr_list.size(); _pos1++) {
		if (check_dist_inner_loop(_obj_ptr_list, _pos1, _pos2, _set)) return true;
		_pos2 = 0;
	}
	return false;
}

bool check_dist_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set) {
	if (_set.boundary_condition == 0) {
		if (check_dist_0free_inner_loop(_obj_ptr_list, _pos1, _pos2, _set)) return true;
		_pos2 = 0;
	}
	else if (_set.boundary_condition == 3) {
		if (check_dist_3free_inner_loop(_obj_ptr_list, _pos1, _pos2, _set)) return true;
		_pos2 = 0;
	}
	else if (_set.boundary_condition == 1) {
		if (check_dist_zfree_inner_loop(_obj_ptr_list, _pos1, _pos2, _set)) return true;
		_pos2 = 0;
	}
	return false;
}

// pos1������ ���ϱ���pos2
bool check_dist_3free_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set)
{
	auto iter1 = _obj_ptr_list.cbegin() + _pos1;
	for (auto iter2 = _obj_ptr_list.cbegin() + _pos2; iter2 != _obj_ptr_list.cend(); iter2++, _pos2++)
	{
		if (iter2 != iter1)
		{
			//�����ж��Ƿ���Ҫ������
			std::array<int, 3> delta_grid;
			delta_grid[0] = (*iter1)->grid_pos[0] - (*iter2)->grid_pos[0];
			delta_grid[1] = (*iter1)->grid_pos[1] - (*iter2)->grid_pos[1];
			delta_grid[2] = (*iter1)->grid_pos[2] - (*iter2)->grid_pos[2];
			//������ߵ�grid֮��С��2������Ҫ������
			if (abs(delta_grid[0]) < 2 && abs(delta_grid[1]) < 2 && abs(delta_grid[2]) < 2)
			{
				std::array<double, 3> dr;
				dr[0] = (*iter1)->pos[0] - (*iter2)->pos[0];
				dr[1] = (*iter1)->pos[1] - (*iter2)->pos[1];
				dr[2] = (*iter1)->pos[2] - (*iter2)->pos[2];
				double dist = pow(pow(dr[0], 2) + pow(dr[1], 2) + pow(dr[2], 2), 0.5);
				double total_radius = (*iter1)->radius + (*iter2)->radius;
				if (dist < total_radius) {
					return true;
				}
			}
		}
	}
	return false;
}

bool check_dist_zfree_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set)
{
	auto iter1 = _obj_ptr_list.cbegin() + _pos1;
	for (auto iter2 = _obj_ptr_list.cbegin() + _pos2; iter2 != _obj_ptr_list.cend(); iter2++, _pos2++)
	{
		if (iter2 != iter1)
		{
			//���3ά��ľ���
			std::array<int, 3> delta_grid = {0};
			delta_grid[0] = (*iter1)->grid_pos[0] - (*iter2)->grid_pos[0];
			delta_grid[1] = (*iter1)->grid_pos[1] - (*iter2)->grid_pos[1];
			delta_grid[2] = (*iter1)->grid_pos[2] - (*iter2)->grid_pos[2];
			//������ߵ�grid֮��С��2��˵��ֱ�����ڣ������ߵ���box_grid_num��˵�������п��ܿ����ڽ�ϣ�������Ҫ������
			// ����˵Ӧ��box_grid_num-1��,��-1�ڳ�ʼ��box_grid_num��ʱ���Ѿ������ˡ�
			// ɾ�� abs(delta_grid[2]) == _set.box_grid_num[2] ���ı���z��Ŀ����ڽ��
			if ((abs(delta_grid[0]) < 2 || abs(delta_grid[0]) == _set.box_grid_num[0])
				&& (abs(delta_grid[1]) < 2 || abs(delta_grid[1]) == _set.box_grid_num[1])
				&& (abs(delta_grid[2]) < 2)) {

				std::array<double, 3> dr, dr_new;
				dr[0] = (*iter1)->pos[0] - (*iter2)->pos[0];
				dr[1] = (*iter1)->pos[1] - (*iter2)->pos[1];
				dr[2] = (*iter1)->pos[2] - (*iter2)->pos[2];
				for (int i = 0; i < 2; i++)
				{
					if (dr[i] > _set.box_length[i] / 2)
					{
						dr_new[i] = dr[i] - _set.box_length[i];
					}
					else if (dr[i] < _set.box_length[i] / (-2))
					{
						dr_new[i] = dr[i] + _set.box_length[i];
					}
					else {
						dr_new[i] = dr[i];
					}
				}
				dr_new[2] = dr[2];
				double dist = pow(pow(dr_new[0], 2) + pow(dr_new[1], 2) + pow(dr_new[2], 2), 0.5);
				double total_radius = (*iter1)->radius + (*iter2)->radius;

				//����ǿ����ڵĽ�ϣ����Object1ƽ��һ�£�������true
				if (dist <= total_radius)
				{
					for (int i = 0; i < 2; i++)
					{
						if (dr[i] > _set.box_length[i] / 2)
						{
							(*iter1)->pos[i] -= _set.box_length[i];
						}
						else if (dr[i] < _set.box_length[i] / -2)
						{
							(*iter1)->pos[i] += _set.box_length[i];
						}
					}
					return true;
				}
			}
		}
	}
	return false;
}

bool check_dist_0free_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set)
{
	auto iter1 = _obj_ptr_list.cbegin() + _pos1;
	for (auto iter2 = _obj_ptr_list.cbegin() + _pos2; iter2 != _obj_ptr_list.cend(); iter2++, _pos2++)
	{
		if (iter2 != iter1)
		{
			//�����ж��Ƿ���Ҫ������
			std::array<int, 3> delta_grid = {0};
			delta_grid[0] = (*iter1)->grid_pos[0] - (*iter2)->grid_pos[0];
			delta_grid[1] = (*iter1)->grid_pos[1] - (*iter2)->grid_pos[1];
			delta_grid[2] = (*iter1)->grid_pos[2] - (*iter2)->grid_pos[2];
			//������ߵ�grid֮��С��2��˵��ֱ�����ڣ������ߵ���box_grid_num��˵�������п��ܿ����ڽ�ϣ�������Ҫ������
			if ((abs(delta_grid[0]) < 2 || abs(delta_grid[0]) == _set.box_grid_num[0])
				&& (abs(delta_grid[1]) < 2 || abs(delta_grid[1]) == _set.box_grid_num[1])
				&& (abs(delta_grid[2]) < 2 || abs(delta_grid[2]) == _set.box_grid_num[2]))
			{
				std::array<double, 3> dr, dr_new;
				dr[0] = (*iter1)->pos[0] - (*iter2)->pos[0];
				dr[1] = (*iter1)->pos[1] - (*iter2)->pos[1];
				dr[2] = (*iter1)->pos[2] - (*iter2)->pos[2];
				for (int i = 0; i < 3; i++)
				{
					if (dr[i] > _set.box_length[i] / 2)
					{
						dr_new[i] = dr[i] - _set.box_length[i];
					}
					else if (dr[i] < _set.box_length[i] / -2)
					{
						dr_new[i] = dr[i] + _set.box_length[i];
					}
					else {
						dr_new[i] = dr[i];
					}
				}
				double dist = pow(pow(dr_new[0], 2) + pow(dr_new[1], 2) + pow(dr_new[2], 2), 0.5);
				double total_radius = (*iter1)->radius + (*iter2)->radius;

				
				if (dist <= total_radius)
				{//����ǿ����ڵĽ�ϣ����Object1ƽ��һ�£�������true
					for (int i = 0; i < 3; i++)
					{
						if (dr[i] > _set.box_length[i] / 2)
						{
							(*iter1)->pos[i] -= _set.box_length[i];
						}
						else if (dr[i] < _set.box_length[i] / -2)
						{
							(*iter1)->pos[i] += _set.box_length[i];
						}
					}
					return true;
				}
			}
		}
	}
	return false;
}

// ����pos1��pos2��ִ�кϲ���Ӧ
void carry_out_reaction(std::vector<Object*>& _obj_ptr_list, int& _pos1, const int& _pos2, Object& _obj1, Object& _obj2, const Database& _database, const Setting& _set)
{	// SIA��V������������λ�������
	if ((_obj1.size[0] && _obj2.size[1]) || (_obj1.size[1] && _obj2.size[2])) {
		Object::annihilation_num += (_obj1.size[0] && _obj2.size[1]) ? std::min(_obj1.size[0], _obj2.size[1]) : std::min(_obj1.size[1], _obj2.size[0]);
	}
	//���Ϊ��ȫ�������ж��Ƿ��������Ӧ
	if (_obj1.size[2] == 0 && _obj1.size[3] == 0 && _obj2.size[2] == 0 && _obj2.size[3] == 0
		&& (_obj1.size[0] == _obj2.size[1] && _obj2.size[0] == _obj1.size[1]) )
	{
		if (_set.output_annihilation_reaction > 0)
		{
			output_react_2_0(_obj1, _obj2, _set);
		}
	}
	//���������ȫ������Ϊ���
	else {
		create_object(_obj1, _obj2, _obj_ptr_list, _database, _set); // ͬԭ��������������obj����������ˢ�³�type
		     // ͬԭ��������������obj����������ˢ�³�type
		if (_set.output_combine_reaction > 0)
		{ // �����Ŵر��һ���Ŵ�2-1�� ��Ҫ��������ɵ��Ŵء�
			output_react_2_1(_obj1, _obj2, **(_obj_ptr_list.end() - 1), _set);
		}
	}
	//��ɱ��Object���ͷ��ڴ�
	delete _obj_ptr_list.at(_pos1);
	delete _obj_ptr_list.at(_pos2);
	//��erase vector����Ӧλ�õ�ָ�룬��ɾ�����Ԫ�أ���ɾǰ���Ԫ��
	if (_pos2 > _pos1) {
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos2);
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos1);
	}
	else  {
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos1);
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos2);
		//���pos2��pos1ǰ�棬��ɾ��������object����һ����ѭ����object��������pos1-1
		_pos1--;
	}

}

// ��������obj�ϲ��ɵ���obj, ����obj��ָ��ѹ��vector�У� typeͨ��ˢ������
void create_object(Object& _obj1, Object& _obj2, std::vector<Object*>& _obj_ptr_list,
	const Database& _database, const Setting& _set) {
	std::array<double, 3> new_pos = { 0,0,0 };
	generate_new_pos(_obj1, _obj2, new_pos);

	if (!check_boundary(new_pos[0], new_pos[1], new_pos[2], _set)) {
		std::cout << "�����ϲ�������" << new_pos[0] << ' ' << new_pos[1] << ' ' << new_pos[2] << '\n';
		return;
	}

	std::array<int, 4> new_size = { 0 };
	int new_v = _obj1.size[0] + _obj2.size[0];
	int new_sia = _obj1.size[1] + _obj2.size[1];
	if (new_v > new_sia) new_size[0] = new_v - new_sia;
	else if(new_v < new_sia) new_size[1] = new_sia - new_v;
	new_size[2] = _obj1.size[2] + _obj2.size[2]; // Be
	new_size[3] = _obj1.size[3] + _obj2.size[3]; // He
	int new_dir = 0;

	// ���new obj��sia��������0 ����He Be��Ϊ��
	if (new_size[1] > 0 && new_size[2] == 0 && new_size[3] == 0) { // �ж�����obj��˭��sia��Ŀ��ѡ����Ǹ���
		new_dir = _obj1.size[1] > _obj2.size[1] ? _obj1.dir : _obj2.dir;
	}

	int new_type = size2type(new_size);
	Object* ptr_new_obj = new Object(new_type, new_pos.at(0), new_pos.at(1), new_pos.at(2),
		new_size[0], new_size[1], new_size[2], new_size[3], new_dir, _database, _set);
	_obj_ptr_list.push_back(ptr_new_obj);
}



////������object��λ�ã� ͨ�����ô��Σ�����ֵ����Ϊvoid
////���������λ��obj�Ͱ����Լ�϶��obj����������Obj�ڴ�obj�Ļ����ϣ���size��Ȩ���������Ŵ�һЩ��
////������obj�ڴ�obj��Сobj֮�䣬Ŀǰ��total_size��Ȩƽ��
//void generate_new_pos(Object& _obj1, Object& _obj2, std::array<double, 3>& _new_pos)
//{
//	double dist = pow(pow((_obj1.pos.at(0) - _obj2.pos.at(0)), 2) + pow((_obj1.pos.at(1) - _obj2.pos.at(1)), 2)
//		+ pow((_obj1.pos.at(2) - _obj2.pos.at(2)), 2), 0.5);
//	//�������obj��λ�������ص�������obj��λ������obj��ͬ
//	if (dist < 1e-6)
//	{
//		for (int i = 0; i < 3; i++) {
//			_new_pos.at(i) = _obj1.pos.at(i);
//		}
//	}
//	else {
//		//��λ��Obj���Լ�϶��obj������ ��obj֮���������
//		if (   (_obj1.size[0] && _obj2.size[1]) || (_obj1.size[1] && _obj2.size[0])   )
//		{
//			//��obj2��size=0ʱ��obj1��λ�ò��䣻��obj2��size=obj1��sizeʱ��Obj1��λ��Ӧ��ƽ��obj1��radius
//			int o1_size = std::max(_obj1.size[1], _obj1.size[0]), o2_size = std::max(_obj2.size[1], _obj2.size[0]);
//			if (o1_size > o2_size) {
//				for (int i = 0; i < 3; i++) {
//					_new_pos.at(i) = _obj1.pos.at(i) + (_obj1.pos.at(i) - _obj2.pos.at(i)) / dist *
//						(double)o2_size / (double)o1_size * _obj1.radius;
//					/*
//					_new_pos.at(i) = _obj1.pos.at(i) +
//						(_obj1.pos.at(i) - _obj2.pos.at(i)) / (_obj1.size1 + _obj2.size1) * _obj2.size1;
//					*/
//				}
//			}
//			else {
//				for (int i = 0; i < 3; i++) {
//					_new_pos.at(i) = _obj2.pos.at(i) + (_obj2.pos.at(i) - _obj1.pos.at(i)) / dist *
//						(double)o1_size / (double)o2_size * _obj2.radius;
//					/*
//					_new_pos.at(i) = _obj2.pos.at(i) +
//						(_obj2.pos.at(i) - _obj1.pos.at(i)) / (_obj1.size1 + _obj2.size1)*_obj1.size1;
//						*/
//				}
//			}
//		}
//		//��obj����������, ȡsize��Ȩ�������λ�ã� Ŀǰsia��v��Be��He���ߵ�ԭ��Ȩ����ͬ��
//		else {
//			for (int i = 0; i < 3; i++)
//			{
//				int o1_size_sum = _obj1.size[0] + _obj1.size[1] + _obj1.size[2] + _obj1.size[3];
//				int o2_size_sum = _obj2.size[0] + _obj2.size[1] + _obj2.size[2] + _obj2.size[3];
//				_new_pos.at(i) = (_obj1.pos.at(i) * o1_size_sum + _obj2.pos.at(i) * o2_size_sum)
//					/ (o1_size_sum + o2_size_sum);
//			}
//		}
//	}
//}

// true~���ڣ� false~����
bool check_boundary(double _x, double _y, double _z, const Setting& _set) {
	if (_set.boundary_condition == 1) {
		return _z < _set.box_max[2] && _z >= _set.box_min[2];
	}
	else if (_set.boundary_condition == 3) {
		return _z < _set.box_max[2] && _z >= _set.box_min[2] && _x < _set.box_max[0] && _x >= _set.box_min[0] && _y < _set.box_max[1] && _y >= _set.box_min[1];
	}
	else if (_set.boundary_condition == 0) {
		return true;
	}
	std::cout << "reaction.cpp��check_boundary����������ԭ��Ϊ�߽������쳣��" << _set.boundary_condition << '\n';
	return false;
}

// ��λ�����Լ�϶��obj������ ��obj֮��������� 
//     ���ĳ���Ŵ� V or SIA ��Ŀ����50�� �Ҳ�һ��С�� 5�� λ��ֱ��ѡȡ���Ŵ�λ�á�
//     �������ѡȡ���Ŵ���� 
// ��λ����ǿ�λ���������Ҳ����Լ�϶������ȡ��λλ��
// ��λ�����λ����������λ��Ϊλ�öԿ�λ��Ŀ��Ȩ
// �Լ�϶�����Լ�϶�������� λ��Ϊλ�ö��Լ�϶��Ŀ��Ȩ
// ��������ΪĿǰ����Ԫ����Ŀ�ļ�Ȩ
// ���⣬ ���һ�����λ����С��λ�� С��λ�������ô��λ�ƶ�������Ҫ�ƶ��������㣬���Ի����ԾǨ��

void generate_new_pos(Object& _obj1, Object& _obj2, std::array<double, 3>& _new_pos)
{
	double dist = pow(pow((_obj1.pos.at(0) - _obj2.pos.at(0)), 2) + pow((_obj1.pos.at(1) - _obj2.pos.at(1)), 2)
		+ pow((_obj1.pos.at(2) - _obj2.pos.at(2)), 2), 0.5);
	//�������obj��λ�������ص�������obj��λ������obj��ͬ
	if (dist < 1e-6)
	{
		for (int i = 0; i < 3; i++) {
			_new_pos.at(i) = _obj1.pos.at(i);
		}
	}
	else {
		// ��λ�����Լ�϶��obj������ ��obj֮��������� 
		//     ���ĳ���Ŵ� V or SIA ��Ŀ����50�� �Ҳ�һ��С�� 5�� λ��ֱ��ѡȡ���Ŵ�λ�á�
		//     �������ѡȡ���Ŵ���� 
		if ((_obj1.size[0] && _obj2.size[1]) || (_obj1.size[1] && _obj2.size[0])) {
			int _o1 = std::max(_obj1.size[0], _obj1.size[1]), _o2 = std::max(_obj2.size[0], _obj2.size[1]);
			//if (_o1 > 50 && _o2 < 5) {
			//	// ���ĳ���Ŵ� V or SIA ��Ŀ����50�� ����һ��С�� 5�� λ��ֱ��ѡȡ���Ŵ�λ�á�
			//	for (int i = 0; i < 3; i++) {
			//		_new_pos.at(i) = _obj1.pos.at(i) ;
			//	}
			//}
			//else if(_o2 > 50 && _o1 < 10) {
			//	for (int i = 0; i < 3; i++) {
			//		_new_pos.at(i) = _obj2.pos.at(i);
			//	}
			//}
			if (_o1 > _o2) {
				// ȡ���Ŵ�λ��
				//for (int i = 0; i < 3; i++) {
				//	_new_pos.at(i) = _obj1.pos.at(i);
				//}
				for (int i = 0; i < 3; i++) {
					_new_pos.at(i) = _obj1.pos.at(i);
				}
			}
			else {
				/*for (int i = 0; i < 3; i++) {
					_new_pos.at(i) = _obj2.pos.at(i) ;
				}*/
				// ���Ŵ���С�Ŵط����ƶ�
				for (int i = 0; i < 3; i++) {
					_new_pos.at(i) = _obj2.pos.at(i);
				}
			}
		}
		// ��λ�����λ����������λ��Ϊλ�öԿ�λ��Ŀ��Ȩ
		else if (_obj1.size[0] > 0 && _obj2.size[0] > 0) {
			for (int i = 0; i < 3; i++) {
				_new_pos[i] =( _obj1.size[0] * _obj1.pos[i] + _obj2.size[0] * _obj2.pos[i] ) /((double)_obj1.size[0] + (double)_obj2.size[0]);
				//_new_pos.at(i) = _obj1.pos.at(i) +
				//	(-_obj1.pos.at(i) + _obj2.pos.at(i)) / ((double)_obj1.size[0] + _obj2.size[0]) * _obj2.size[0];
			}
		}
		// �Լ�϶�����Լ�϶�������� λ��Ϊλ�ö��Լ�϶��Ŀ��Ȩ
		else if (_obj1.size[1] > 0 && _obj2.size[1] > 0) {
			for (int i = 0; i < 3; i++) {
				_new_pos.at(i) = (_obj1.pos.at(i) * _obj1.size[1] + _obj2.pos.at(i) * (double)_obj2.size[1]) / (_obj1.size[1] + (double)_obj2.size[1]);
				//_new_pos[i] = (_obj1.size[1] * _obj1.pos[i] + _obj2.size[1] * _obj2.pos[i]) / ((double)_obj1.size[1] + (double)_obj2.size[1]);
			}
		}
		//  ��λ������������
		else if (_obj1.size[0] || _obj2.size[0]) {
			if (_obj1.size[0] > 0) {
				for (int i = 0; i < 3; i++) {
					_new_pos[i] = _obj1.pos[i];
				}
			}
			else {
				for (int i = 0; i < 3; i++) {
					_new_pos[i] = _obj2.pos[i];
				}
			}
		}
		//�Լ�϶��������, λ��ȡ��϶λ��
		else if ((_obj1.size[1] || _obj2.size[1])) {
			if (_obj1.size[1] > 0) {
				for (int i = 0; i < 3; i++) {
					_new_pos[i] = _obj1.pos[i];
				}
			}
			else {
				for (int i = 0; i < 3; i++) {
					_new_pos[i] = _obj2.pos[i];
				}
			}
		}
		// ��������ΪĿǰ����Ԫ����Ŀ�ļ�Ȩ
		else {
			for (int i = 0; i < 3; i++)
			{
				int o1_size_sum = _obj1.size[0] + _obj1.size[1] + _obj1.size[2] + _obj1.size[3];
				int o2_size_sum = _obj2.size[0] + _obj2.size[1] + _obj2.size[2] + _obj2.size[3];
				_new_pos.at(i) = (_obj1.pos.at(i) * o1_size_sum + _obj2.pos.at(i) * o2_size_sum)
					/ ((double)o1_size_sum + (double)o2_size_sum);
			}
		}

	}

	//if (_new_pos[0] > 400 || _new_pos[1] > 400 || _new_pos[2] > 400) {
	//	std::cout << "generate_new_posʱλ�ó�����, pos[0], pos[1] ,pos[2]" << _new_pos[0] << ' ' << _new_pos[1] << ' ' << _new_pos[2] << "\n";
	//	system("pause");
	//}
}
