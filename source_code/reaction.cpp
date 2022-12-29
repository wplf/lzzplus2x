#include "reaction.h"

// 以引用的方式传入pos1，pos2， 得到需要反应的两个pos，和返回的bool值

// 更改check_dist 功能，基于网格划分， 将三种边界条件集成到此函数中
// 判断三种边界条件并调用
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

// pos1不动， 不断遍历pos2
bool check_dist_3free_inner_loop(const std::vector<Object*>& _obj_ptr_list, const int& _pos1, int& _pos2, const Setting& _set)
{
	auto iter1 = _obj_ptr_list.cbegin() + _pos1;
	for (auto iter2 = _obj_ptr_list.cbegin() + _pos2; iter2 != _obj_ptr_list.cend(); iter2++, _pos2++)
	{
		if (iter2 != iter1)
		{
			//首先判断是否需要检查距离
			std::array<int, 3> delta_grid;
			delta_grid[0] = (*iter1)->grid_pos[0] - (*iter2)->grid_pos[0];
			delta_grid[1] = (*iter1)->grid_pos[1] - (*iter2)->grid_pos[1];
			delta_grid[2] = (*iter1)->grid_pos[2] - (*iter2)->grid_pos[2];
			//如果三边的grid之差小于2，则需要检查距离
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
			//检查3维间的距离
			std::array<int, 3> delta_grid = {0};
			delta_grid[0] = (*iter1)->grid_pos[0] - (*iter2)->grid_pos[0];
			delta_grid[1] = (*iter1)->grid_pos[1] - (*iter2)->grid_pos[1];
			delta_grid[2] = (*iter1)->grid_pos[2] - (*iter2)->grid_pos[2];
			//如果三边的grid之差小于2（说明直接相邻），或者等于box_grid_num（说明两者有可能跨周期结合），则需要检查距离
			// 按理说应该box_grid_num-1的,但-1在初始化box_grid_num的时候已经内置了。
			// 删掉 abs(delta_grid[2]) == _set.box_grid_num[2] 即改变了z轴的跨周期结合
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

				//如果是跨周期的结合，则把Object1平移一下，并返回true
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
			//首先判断是否需要检查距离
			std::array<int, 3> delta_grid = {0};
			delta_grid[0] = (*iter1)->grid_pos[0] - (*iter2)->grid_pos[0];
			delta_grid[1] = (*iter1)->grid_pos[1] - (*iter2)->grid_pos[1];
			delta_grid[2] = (*iter1)->grid_pos[2] - (*iter2)->grid_pos[2];
			//如果三边的grid之差小于2（说明直接相邻），或者等于box_grid_num（说明两者有可能跨周期结合），则需要检查距离
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
				{//如果是跨周期的结合，则把Object1平移一下，并返回true
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

// 输入pos1和pos2，执行合并反应
void carry_out_reaction(std::vector<Object*>& _obj_ptr_list, int& _pos1, const int& _pos2, Object& _obj1, Object& _obj2, const Database& _database, const Setting& _set)
{	// SIA与V结合情况，即空位湮灭情况
	if ((_obj1.size[0] && _obj2.size[1]) || (_obj1.size[1] && _obj2.size[2])) {
		Object::annihilation_num += (_obj1.size[0] && _obj2.size[1]) ? std::min(_obj1.size[0], _obj2.size[1]) : std::min(_obj1.size[1], _obj2.size[0]);
	}
	//如果为完全湮灭，则判断是否输出湮灭反应
	if (_obj1.size[2] == 0 && _obj1.size[3] == 0 && _obj2.size[2] == 0 && _obj2.size[3] == 0
		&& (_obj1.size[0] == _obj2.size[1] && _obj2.size[0] == _obj1.size[1]) )
	{
		if (_set.output_annihilation_reaction > 0)
		{
			output_react_2_0(_obj1, _obj2, _set);
		}
	}
	//如果不是完全湮灭，则为结合
	else {
		create_object(_obj1, _obj2, _obj_ptr_list, _database, _set); // 同原版程序的区别是在obj创建过程中刷新出type
		     // 同原版程序的区别是在obj创建过程中刷新出type
		if (_set.output_combine_reaction > 0)
		{ // 两个团簇变成一个团簇2-1， 需要传入刚生成的团簇。
			output_react_2_1(_obj1, _obj2, **(_obj_ptr_list.end() - 1), _set);
		}
	}
	//先杀掉Object，释放内存
	delete _obj_ptr_list.at(_pos1);
	delete _obj_ptr_list.at(_pos2);
	//再erase vector中相应位置的指针，先删后面的元素，再删前面的元素
	if (_pos2 > _pos1) {
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos2);
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos1);
	}
	else  {
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos1);
		_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _pos2);
		//如果pos2在pos1前面，则删除了两个object后，下一个待循环的object的索引是pos1-1
		_pos1--;
	}

}

// 生成两个obj合并成的新obj, 将新obj的指针压入vector中， type通过刷新生成
void create_object(Object& _obj1, Object& _obj2, std::vector<Object*>& _obj_ptr_list,
	const Database& _database, const Setting& _set) {
	std::array<double, 3> new_pos = { 0,0,0 };
	generate_new_pos(_obj1, _obj2, new_pos);

	if (!check_boundary(new_pos[0], new_pos[1], new_pos[2], _set)) {
		std::cout << "发生合并并出界" << new_pos[0] << ' ' << new_pos[1] << ' ' << new_pos[2] << '\n';
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

	// 如果new obj的sia数量大于0 并且He Be都为零
	if (new_size[1] > 0 && new_size[2] == 0 && new_size[3] == 0) { // 判断两个obj中谁的sia数目大，选大的那个。
		new_dir = _obj1.size[1] > _obj2.size[1] ? _obj1.dir : _obj2.dir;
	}

	int new_type = size2type(new_size);
	Object* ptr_new_obj = new Object(new_type, new_pos.at(0), new_pos.at(1), new_pos.at(2),
		new_size[0], new_size[1], new_size[2], new_size[3], new_dir, _database, _set);
	_obj_ptr_list.push_back(ptr_new_obj);
}



////计算新object的位置， 通过引用传参，返回值可以为void
////如果包含空位的obj和包含自间隙的obj相遇，则新Obj在大obj的基础上，比size加权更靠近大团簇一些。
////否则，新obj在大obj和小obj之间，目前用total_size加权平均
//void generate_new_pos(Object& _obj1, Object& _obj2, std::array<double, 3>& _new_pos)
//{
//	double dist = pow(pow((_obj1.pos.at(0) - _obj2.pos.at(0)), 2) + pow((_obj1.pos.at(1) - _obj2.pos.at(1)), 2)
//		+ pow((_obj1.pos.at(2) - _obj2.pos.at(2)), 2), 0.5);
//	//如果两个obj的位置正好重叠，则新obj的位置与老obj相同
//	if (dist < 1e-6)
//	{
//		for (int i = 0; i < 3; i++) {
//			_new_pos.at(i) = _obj1.pos.at(i);
//		}
//	}
//	else {
//		//空位型Obj与自间隙型obj相遇， 两obj之间存在湮灭
//		if (   (_obj1.size[0] && _obj2.size[1]) || (_obj1.size[1] && _obj2.size[0])   )
//		{
//			//当obj2的size=0时，obj1的位置不变；当obj2的size=obj1的size时，Obj1的位置应该平移obj1的radius
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
//		//两obj不存在湮灭。, 取size加权获得最终位置， 目前sia、v、Be、He四者的原子权重相同。
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

// true~界内， false~界外
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
	std::cout << "reaction.cpp中check_boundary函数出错，其原因为边界条件异常：" << _set.boundary_condition << '\n';
	return false;
}

// 空位型与自间隙型obj相遇， 两obj之间存在湮灭， 
//     如果某个团簇 V or SIA 数目大于50， 且并一个小于 5， 位置直接选取大团簇位置。
//     其余情况选取大团簇外侧 
// 空位型与非空位型相遇，且不是自间隙，最终取空位位置
// 空位型与空位型相遇，其位置为位置对空位数目加权
// 自间隙型与自间隙型相遇， 位置为位置对自间隙数目加权
// 其余的情况为目前四种元素数目的加权
// 问题， 如果一个大空位碰到小空位， 小空位本不能让大空位移动，但他要移动到最近格点，所以会出现跃迁。

void generate_new_pos(Object& _obj1, Object& _obj2, std::array<double, 3>& _new_pos)
{
	double dist = pow(pow((_obj1.pos.at(0) - _obj2.pos.at(0)), 2) + pow((_obj1.pos.at(1) - _obj2.pos.at(1)), 2)
		+ pow((_obj1.pos.at(2) - _obj2.pos.at(2)), 2), 0.5);
	//如果两个obj的位置正好重叠，则新obj的位置与老obj相同
	if (dist < 1e-6)
	{
		for (int i = 0; i < 3; i++) {
			_new_pos.at(i) = _obj1.pos.at(i);
		}
	}
	else {
		// 空位型与自间隙型obj相遇， 两obj之间存在湮灭， 
		//     如果某个团簇 V or SIA 数目大于50， 且并一个小于 5， 位置直接选取大团簇位置。
		//     其余情况选取大团簇外侧 
		if ((_obj1.size[0] && _obj2.size[1]) || (_obj1.size[1] && _obj2.size[0])) {
			int _o1 = std::max(_obj1.size[0], _obj1.size[1]), _o2 = std::max(_obj2.size[0], _obj2.size[1]);
			//if (_o1 > 50 && _o2 < 5) {
			//	// 如果某个团簇 V or SIA 数目大于50， 且另一个小于 5， 位置直接选取大团簇位置。
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
				// 取大团簇位置
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
				// 大团簇向小团簇方向移动
				for (int i = 0; i < 3; i++) {
					_new_pos.at(i) = _obj2.pos.at(i);
				}
			}
		}
		// 空位型与空位型相遇，其位置为位置对空位数目加权
		else if (_obj1.size[0] > 0 && _obj2.size[0] > 0) {
			for (int i = 0; i < 3; i++) {
				_new_pos[i] =( _obj1.size[0] * _obj1.pos[i] + _obj2.size[0] * _obj2.pos[i] ) /((double)_obj1.size[0] + (double)_obj2.size[0]);
				//_new_pos.at(i) = _obj1.pos.at(i) +
				//	(-_obj1.pos.at(i) + _obj2.pos.at(i)) / ((double)_obj1.size[0] + _obj2.size[0]) * _obj2.size[0];
			}
		}
		// 自间隙型与自间隙型相遇， 位置为位置对自间隙数目加权
		else if (_obj1.size[1] > 0 && _obj2.size[1] > 0) {
			for (int i = 0; i < 3; i++) {
				_new_pos.at(i) = (_obj1.pos.at(i) * _obj1.size[1] + _obj2.pos.at(i) * (double)_obj2.size[1]) / (_obj1.size[1] + (double)_obj2.size[1]);
				//_new_pos[i] = (_obj1.size[1] * _obj1.pos[i] + _obj2.size[1] * _obj2.pos[i]) / ((double)_obj1.size[1] + (double)_obj2.size[1]);
			}
		}
		//  空位型与其他相遇
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
		//自间隙其他相遇, 位置取间隙位置
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
		// 其余的情况为目前四种元素数目的加权
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
	//	std::cout << "generate_new_pos时位置出问题, pos[0], pos[1] ,pos[2]" << _new_pos[0] << ' ' << _new_pos[1] << ' ' << _new_pos[2] << "\n";
	//	system("pause");
	//}
}
