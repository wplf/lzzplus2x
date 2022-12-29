#include"event.h"
#include <cassert>



//  -1~insert_cascade, 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation, 6~trap_mutation, 7~He_insert_3, 8~Be_insert_2
double build_event_list(std::vector<Object*>& _obj_ptr_list, std::vector<Event>& _event_list, const Setting& _setting)
{
	_event_list.clear();
	double rate_sum = 0;
	
	// 遍历所有obj中可能发生的事件
	// 遍历obj
	for (auto iter = _obj_ptr_list.cbegin(); iter != _obj_ptr_list.cend(); iter++) {
		Event temp_event;
		// 遍历所有frequency
		for (auto freq = (*iter)->frequency.cbegin(); freq != (*iter)->frequency.cend(); freq++) {
			if (freq->second > 0) {
				rate_sum += freq->second;
				temp_event.rate = rate_sum;
				temp_event.obj_pos = iter - _obj_ptr_list.cbegin();
				temp_event.event_type = freq->first;
				_event_list.push_back(temp_event);
			}
		}
	}
	
	// 外部cascade与内部函数引入obj
	if (_setting.rate_cascade > 0) {
		Event temp_event;
		rate_sum += _setting.rate_cascade;
		temp_event.rate = rate_sum;
		//插入cascade这个事件，Pos = -1, which_event = -1
		temp_event.obj_pos = -1;
		temp_event.event_type = -1;
		_event_list.push_back(temp_event);
	}
	if (_setting.He_insert_rate > 0) {
		Event temp_event;
		rate_sum += _setting.He_insert_rate;
		temp_event.rate = rate_sum;
		//He_insert，Pos = 0, which_event = 7
		temp_event.obj_pos = 0;
		temp_event.event_type = 7;
		_event_list.push_back(temp_event);
	}
	if (_setting.Be_insert_rate > 0) {
		Event temp_event;
		rate_sum += _setting.Be_insert_rate;
		temp_event.rate = rate_sum;
		//Be_insert，Pos = 0, which_event = 8
		temp_event.obj_pos = 0;
		temp_event.event_type = 8;
		_event_list.push_back(temp_event);
	}

	if (_event_list.size() == 0) {
		std::cout << "The event list is empty!\n";
	}
	return rate_sum;
}

int find_event(std::vector<Event>& _event_list, double _rate_sum) {
	double rand_num = uni();
	double rand_rate = rand_num * _rate_sum;
	//binary search 二分查找
	int start_pos = 0, end_pos = _event_list.size() - 1;
	while (start_pos < end_pos)
	{
		int mid_pos = (start_pos + end_pos) / 2;
		double product = (rand_rate - _event_list.at(mid_pos).rate) * (rand_rate - _event_list.at(mid_pos + 1).rate);
		if (product < 0) {
			return mid_pos + 1;
		}
		else if (rand_rate > _event_list.at(mid_pos + 1).rate)
		{
			start_pos = mid_pos + 1;
		}
		else if (rand_rate <= _event_list.at(mid_pos).rate)
		{
			end_pos = mid_pos;
		}
	}
	return start_pos;
}

//输入object再obj_ptr_list中的位置，以及它要执行的反应，执行该反应
//如果是迁移事件，迁移之后，应检查碰撞，如果碰撞，_flag_refresh_event_list重置为true，准备重建event_list
//两种发射事件，均需要重建event_list
void carry_out_event(int _which_event, int _obj_pos, std::vector<Object*>& _obj_ptr_list, bool& _flag_refresh_event_list,
	const Database& _database,  const Setting& _set)
{
	switch (_which_event)
	{
	case 0:
	{
		//执行迁移操作
		carry_out_mig(_obj_pos, _obj_ptr_list, _flag_refresh_event_list, _database, _set);
		break;
	}
	case 1:
		carry_out_emit(0,  _obj_ptr_list, *_obj_ptr_list.at(_obj_pos), _obj_pos, _database, _set);
		_flag_refresh_event_list = true;
		break;
	case 2:
		carry_out_emit(1, _obj_ptr_list, *_obj_ptr_list.at(_obj_pos), _obj_pos, _database, _set);
		_flag_refresh_event_list = true;
		break;
	case 3:
		carry_out_emit(2, _obj_ptr_list, *_obj_ptr_list.at(_obj_pos), _obj_pos, _database, _set);
		_flag_refresh_event_list = true;
		break;
	case 4:
		carry_out_emit(3, _obj_ptr_list, *_obj_ptr_list.at(_obj_pos), _obj_pos, _database, _set);
		_flag_refresh_event_list = true;
		break;
	case 5:
		(*_obj_ptr_list.at(_obj_pos)).sia_rotate();
		break;
	case 6:
		carry_out_tm(_obj_ptr_list, *_obj_ptr_list.at(_obj_pos), _obj_pos, _database, _set);
		_flag_refresh_event_list = true;
		break;
	case 7:
		insert_obj(3, _obj_ptr_list, _database, _set);
		_flag_refresh_event_list = true;
		break;
	case 8:
		insert_obj(2, _obj_ptr_list, _database, _set);
		_flag_refresh_event_list = true;
		break;
	default:
		break;
	}
}

void carry_out_mig(int _obj_pos, std::vector<Object*>& _obj_ptr_list, bool& _flag_refresh_event_list, const Database& _database, const Setting& _setting)
{
	// mig内部已经实现不同种类的团簇mig
	_obj_ptr_list.at(_obj_pos)->mig(_setting);

	//执行完迁移事件后，是否需要check_reaction
	bool flag_check_reaction = true;

	// 设置开关来判断是否被sink吸收 1. 晶粒半径 2. 晶界(sink strength), 3 位错密度
	// 如果被吸收，置为false
	bool check_sink_strength = true;


	// 内部为先检查边界条件，再刷新格子，再检查是否需要反应。
	// 周期性边界条件， 此处也可将边界条件封装到某个函数中，但没必要
	if (_setting.boundary_condition == 0)
	{
		_obj_ptr_list.at(_obj_pos)->check_pbc(_setting);
		// 刷新网格
		_obj_ptr_list.at(_obj_pos)->refresh_grid_pos(_setting);

		// 晶界
		if (_setting.grain_radius > 0)
		{
			//如果位移超过晶粒半径
			if (_obj_ptr_list.at(_obj_pos)->check_grain_radius(_setting)) {
				if (_setting.output_GB_absorption > 0) {
					output_react_1_0_GB(*(_obj_ptr_list.at(_obj_pos)), _setting);
				}
				delete _obj_ptr_list.at(_obj_pos);
				_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
				//迁移的obj被杀掉了，所以要更新event list
				_flag_refresh_event_list = true;
				//迁移的那个Obj被杀掉了，所以不用check_reaction
				flag_check_reaction = false;
			}
		}
		//通过sink strength，判断Obj是否穿过晶界
		if (_setting.ss_grain_radius > 0 && check_sink_strength)
		{
			int type = _obj_ptr_list.at(_obj_pos)->type;
			//如果Obj是空位，那么抽一个随机数检查是否小于p_vac_meet_GB，如果小于，kill该obj
			if (type == 1)
			{
				if (uni() < _setting.p_vac_meet_GB)
				{
					if (_setting.output_GB_absorption > 0) {
						output_react_1_0_GB(*(_obj_ptr_list.at(_obj_pos)), _setting);
					}
					delete _obj_ptr_list.at(_obj_pos);
					_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
					//迁移的obj被杀掉了，所以要更新event list
					_flag_refresh_event_list = true;
					//迁移的那个Obj被杀掉了，所以不用check_reaction
					flag_check_reaction = false;
					//迁移的那个Obj被杀掉了，所以不用再判断是否被其他sink吸收
					check_sink_strength = false;
				}
			}
			//如果Obj是sia，那么抽一个随机数检查是否小于p_sia_meet_GB，如果小于，kill该obj
			else if (type == 2)
			{
				if (uni() < _setting.p_sia_meet_GB)
				{
					if (_setting.output_GB_absorption > 0) {
						output_react_1_0_GB(*(_obj_ptr_list.at(_obj_pos)), _setting);
					}
					delete _obj_ptr_list.at(_obj_pos);
					_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
					//迁移的obj被杀掉了，所以要更新event list
					_flag_refresh_event_list = true;
					//迁移的那个Obj被杀掉了，所以不用check_reaction
					flag_check_reaction = false;
					//迁移的那个Obj被杀掉了，所以不用再判断是否被其他sink吸收
					check_sink_strength = false;
				}
			}
		}
		// 位错密度
		if (_setting.dislocation_density > 0 && check_sink_strength)
		{
			if (uni() < _obj_ptr_list.at(_obj_pos)->p_meet_dislocation)
			{
				if (_setting.output_disloaction_absorption > 0) {
					output_react_1_0_dislocation(*(_obj_ptr_list.at(_obj_pos)), _setting);
				}
				delete _obj_ptr_list.at(_obj_pos);
				_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
				//迁移的obj被杀掉了，所以要更新event list
				_flag_refresh_event_list = true;
				//迁移的那个Obj被杀掉了，所以不用check_reaction
				flag_check_reaction = false;
				//迁移的那个Obj被杀掉了，所以不用再判断是否被其他sink吸收
				check_sink_strength = false;
			}
		}

		//周期性边界条件 迁移完之后，看看和其他Obj是否撞上
		if (flag_check_reaction)
		{
			int flag1_pos2 = 0;
			bool flag1 = check_dist_0free_inner_loop(_obj_ptr_list, _obj_pos, flag1_pos2, _setting);
			if (flag1) {
				carry_out_reaction(_obj_ptr_list, _obj_pos, flag1_pos2, *_obj_ptr_list.at(_obj_pos), *_obj_ptr_list.at(flag1_pos2), _database, _setting);
				//如果执行了反应，代表有obj发生变化，因此需要更新event_list
				_flag_refresh_event_list = true;
			}
		}
	}
	//如果是吸收边界条件
	else if (_setting.boundary_condition == 3) {
		//如果撞到边界了，就把obj去掉，并且不再check_reaction
		if (_obj_ptr_list.at(_obj_pos)->check_3free(_setting) ) {
			if ( (_setting.output_out_bound > 0) || (_setting.output_big_clu_out_bound > 0 && 
				_obj_ptr_list.at(_obj_pos)->size[0] + _obj_ptr_list.at(_obj_pos)->size[1] + _obj_ptr_list.at(_obj_pos)->size[2] + _obj_ptr_list.at(_obj_pos)->size[3] > _setting.big_clu_definition
				) ) {
				output_react_1_0(*(_obj_ptr_list.at(_obj_pos)), _setting);
			}
			delete _obj_ptr_list.at(_obj_pos);
			_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
			//迁移的obj被杀掉了，所以要更新event list
			_flag_refresh_event_list = true;
			//迁移的那个Obj被杀掉了，所以不用check_reaction
			flag_check_reaction = false;
		}
		//如果没有撞到边界，就应该更新以下Object所在的网格
		else {
			_obj_ptr_list.at(_obj_pos)->refresh_grid_pos(_setting);
		}

		//迁移完之后，看看和其他Obj是否撞上
		if (flag_check_reaction)
		{
			int pos2 = 0;
			if (check_dist_3free_inner_loop(_obj_ptr_list, _obj_pos, pos2, _setting))
			{
				carry_out_reaction(_obj_ptr_list, _obj_pos, pos2, *_obj_ptr_list.at(_obj_pos), *_obj_ptr_list.at(pos2),
					_database, _setting);
				//如果执行了反应，代表有obj发生变化，因此需要更新event_list
				_flag_refresh_event_list = true;
			}
		}
	}
	//如果是XY周期，Z开放
	else if (_setting.boundary_condition == 1)
	{
		// 先检查边界，再刷新本obj的网格。
		if (_obj_ptr_list.at(_obj_pos)->check_zfree(_setting))
		{
			if ( (_setting.output_out_bound > 0) || (_setting.output_big_clu_out_bound > 0 && 
				_obj_ptr_list.at(_obj_pos)->size[0] + _obj_ptr_list.at(_obj_pos)->size[1] + _obj_ptr_list.at(_obj_pos)->size[2] + _obj_ptr_list.at(_obj_pos)->size[3] > _setting.big_clu_definition
				) ) 
			{
				output_react_1_0(*(_obj_ptr_list.at(_obj_pos)), _setting);
			}
			delete _obj_ptr_list.at(_obj_pos);
			_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
			//迁移的obj被杀掉了，所以要更新event list
			_flag_refresh_event_list = true;
			//迁移的那个Obj被杀掉了，所以不用check_reaction
			flag_check_reaction = false;
		}
		else{
			_obj_ptr_list.at(_obj_pos)->refresh_grid_pos(_setting);
		}
		// 迁移完成检查反应
		if (flag_check_reaction)
		{
			int pos2 = 0;
			if (check_dist_zfree_inner_loop(_obj_ptr_list, _obj_pos, pos2, _setting))
			{
				carry_out_reaction(_obj_ptr_list, _obj_pos, pos2, *_obj_ptr_list.at(_obj_pos), *_obj_ptr_list.at(pos2),
					_database, _setting);
				//如果执行了反应，代表有obj发生变化，因此需要更新event_list
				_flag_refresh_event_list = true;
			}
		}
	}
}

// emit 之后原团簇会减小，所以不用判断原团簇的反应
void carry_out_emit(int _emit_element, std::vector<Object*>& _obj_ptr_list, Object& _obj, int _obj_pos, const Database& _database, const Setting& _setting)
{
	//更改
	// 创建两个新obj
	int o1size0 = _obj.size[0], o1size1 = _obj.size[1], o1size2 = _obj.size[2], o1size3 = _obj.size[3];
	int o2size0 = 0, o2size1 = 0, o2size2 = 0, o2size3 = 0;
	int o1type = -1, o2type = -1, o1dir = 0, o2dir = 0;
	double o1x = _obj.pos[0], o1y = _obj.pos[1], o1z = _obj.pos[2];
	double o2x = 0, o2y = 0, o2z = 0;
	

	// 0~V 1~SIA 2Be 3He
	if (_emit_element == 0)  {
		o2size0 = 1;
		o1size0--;
	}
	else if (_emit_element == 1) {
		o1size1--;
		o2size1 = 1;
	}
	else if (_emit_element == 2) {
		o1size2--;
		o2size2 = 1;
	}
	else if (_emit_element == 3) {
		o1size3--;
		o2size3 = 1;
	}
	else { std::cerr << "发射元素中元素出错\n"; }

	//产生一个0-2pi之间的随机弧度值，确定被发射元素的坐标
	double angle1 = uni() * 2 * 3.1415926;
	double angle2 = uni() * 2 * 3.1415926;

	//球半径为母obj的半径加3
	double r = _obj.radius + 3;
	o2x = o1x + r * sin(angle1) * cos(angle2);
	o2y = o1y + r * sin(angle1) * sin(angle2);
	o2z = o1z + r * cos(angle1);

	bool flag_o2 = check_boundary(o2x, o2y, o2z, _setting);

	Object* ptr_o1 = new Object(o1type, o1x, o1y, o1z, o1size0, o1size1, o1size2, o1size3, o1dir, _database, _setting);
	Object* ptr_o2 = new Object(o2type, o2x, o2y, o2z, o2size0, o2size1, o2size2, o2size3, o2dir, _database, _setting);

	// 是否输出发射事件, 发射事件传入新生成的两个团簇， 在内部函数中获得老团簇size。
	if (_setting.output_emit_reaction > 0) {
		output_react_1_2(_obj, *ptr_o1, *ptr_o2, _setting);
	}

	// 删除老团簇
	delete(_obj_ptr_list.at(_obj_pos));
	_obj_ptr_list.erase(_obj_ptr_list.begin() + _obj_pos);

	// 先加入o1
	_obj_ptr_list.push_back(ptr_o1);

	// 如果o2存活就加入o2
	if (flag_o2) {
		_obj_ptr_list.push_back(ptr_o2);
		//check_reaction，对吐出来的obj循环一轮即可，pos1是被吐出来的obj的索引
		int pos1 = _obj_ptr_list.size() - 1, pos2 = 0;
		if (check_dist_inner_loop(_obj_ptr_list, pos1, pos2, _setting))
		{
			carry_out_reaction(_obj_ptr_list, pos1, pos2, *_obj_ptr_list.at(pos1), *_obj_ptr_list.at(pos2), _database
				, _setting);
		}
	}
	else {
		delete(ptr_o2);
	}
}

// t_m 之后原团簇会变大，所以需要判断原团簇的反应
void carry_out_tm(std::vector<Object*>& _obj_ptr_list, Object& _obj, int _obj_pos, const Database& _database, const Setting& _setting)
{
	// t_m计数加1
	Object::t_m_num++;

	// 创建两个新obj
	int o1size0 = _obj.size[0], o1size1 = _obj.size[1], o1size2 = _obj.size[2], o1size3 = _obj.size[3];
	int o2size0 = 0, o2size1 = 1, o2size2 = 0, o2size3 = 0;
	int o1type = -1, o2type = -1, o1dir = 0, o2dir = 0;
	double o1x = _obj.pos[0], o1y = _obj.pos[1], o1z = _obj.pos[2];
	double o2x = 0, o2y = 0, o2z = 0;

	//产生一个0-2pi之间的随机弧度值，确定被发射元素的坐标
	double angle1 = uni() * 2 * 3.1415926;
	double angle2 = uni() * 2 * 3.1415926;

	//球半径为母obj的半径加3
	double r = _obj.radius + 3;
	o2x = _obj.pos.at(0) + r * sin(angle1) * cos(angle2);
	o2y = _obj.pos.at(1) + r * sin(angle1) * sin(angle2);
	if (_setting.TM_sia2surface) {
		o2z = _obj.pos.at(2) - r * fabs(cos(angle1));
	}
	else {
		o2z = _obj.pos.at(2) + r * cos(angle1);
	}
	
	if (o1size1 > 0) {
		o1size1--;
	}
	else {
		o1size0++;
	}

	bool flag_o2 = check_boundary(o2x, o2y, o2z, _setting);

	Object* ptr_o1 = new Object(o1type, o1x, o1y, o1z, o1size0, o1size1, o1size2, o1size3, o1dir, _database, _setting);
	Object* ptr_o2 = new Object(o2type, o2x, o2y, o2z, o2size0, o2size1, o2size2, o2size3, o2dir, _database, _setting);

	// 是否输出发射事件, 发射事件传入新生成的两个团簇， 在内部函数中获得老团簇size。
	if (_setting.output_emit_reaction > 0) {
		output_react_1_2(_obj, *ptr_o1, *ptr_o2, _setting);
	}

	// 删除老团簇
	delete(_obj_ptr_list.at(_obj_pos));
	_obj_ptr_list.erase(_obj_ptr_list.begin() + _obj_pos);

	// 先加入o1,并判断反应
	_obj_ptr_list.push_back(ptr_o1);

	// 对t_m产生的obj进行反应判断， 此种情况为判断体系内所有可能发生的反应，并非只对一个团簇进行反应判断
	int pos1 = 0, pos2 = 0;
	while (check_dist(_obj_ptr_list, pos1, pos2, _setting))
	{
		carry_out_reaction(_obj_ptr_list, pos1, pos2, *_obj_ptr_list.at(pos1), *_obj_ptr_list.at(pos2), _database, _setting);
		pos2 = 0;
	}

	// 如果o2存活就加入o2，并判断反应
	if (flag_o2) {
		_obj_ptr_list.push_back(ptr_o2);
		//check_reaction，对吐出来的obj循环一轮即可，pos1是被吐出来的obj的索引
		int pos1 = _obj_ptr_list.size() - 1, pos2 = 0;
		if (check_dist_inner_loop(_obj_ptr_list, pos1, pos2, _setting)){
			carry_out_reaction(_obj_ptr_list, pos1, pos2, *_obj_ptr_list.at(pos1), *_obj_ptr_list.at(pos2), _database, _setting);
		}
	}
	else {
		delete(ptr_o2);
	}
}


std::string choose_cascade_to_insert(Setting& _set)
{
	double rand_num = uni();
	for (auto iter = _set.cascade_possibility.cbegin(); iter != _set.cascade_possibility.cend(); iter++)
	{
		if (*iter > rand_num)
		{
			int num = iter - _set.cascade_possibility.cbegin();
			std::ostringstream os;
			os << "cascade." << _set.cascade_energy.at(num);
			double x = uni();
			int n = (int)(x * _set.cascade_number.at(num)) + 1;
			os << '.' << n << ".txt";
			return os.str();
		}
	}
	return "fail to choose cascade_to_insert";
}


// 修改event中插入Be的深度分布，设置其数据为SRIM 100 eV数据
std::map < double, double> five_eVBe = {
	{0.00,  0.00},
	{1.01,  0.12},
	{2.01,  0.13},
	{3.01,  0.53},
	{4.01,  0.73},
	{5.01,  0.86},
	{6.01,  0.94},
	{7.01,  0.98},
	{8.01,  0.99},
	{9.01,  1.00},
};
std::map < double, double> one_hundred_eVBe = {
{0.00000 ,	0.00000	},
{0.00962 ,	1.01000	},
{0.02738 ,	2.01000	},
{0.05644 ,	3.01000	},
{0.09368 ,	4.01000	},
{0.13792 ,	5.01000	},
{0.18795 ,	6.01000	},
{0.24018 ,	7.01000	},
{0.29540 ,	8.01000	},
{0.35242 ,	9.01000	},
{0.41021 ,	10.01000},
{0.46678 ,	11.01000},
{0.52255 ,	12.01000},
{0.57609 ,	13.01000},
{0.62741 ,	14.01000},
{0.67579 ,	15.01000},
{0.71906 ,	16.01000},
{0.75701 ,	17.01000},
{0.79299 ,	18.01000},
{0.82438 ,	19.01000},
{0.85307 ,	20.01000},
{0.87765 ,	21.01000},
{0.89890 ,	22.01000},
{0.91684 ,	23.01000},
{0.93254 ,	24.01000},
{0.94503 ,	25.01000},
{0.95627 ,	26.01000},
{0.96474 ,	27.01000},
{0.97224 ,	28.01000},
{0.97859 ,	29.01000},
{0.98308 ,	30.01000},
{0.98674 ,	31.01000},
{0.98985 ,	32.01000},
{0.99229 ,	33.01000},
{0.99405 ,	34.01000},
{0.99555 ,	35.01000},
{0.99685 ,	36.01000},
{0.99754 ,	37.01000},
{0.99824 ,	38.01000},
{0.99870 ,	39.01000},
{0.99908 ,	40.01000},
{0.99933 ,	41.01000},
{0.99950 ,	42.01000},
{0.99969 ,	43.01000},
{0.99983 ,	44.01000},
{0.99987 ,	45.01000},
{0.99994 ,	46.01000},
{0.99994 ,	47.01000},
{0.99998 ,	48.01000},
{1.00000 ,	56.01000},
};
std::map < double, double> five_eVHe;
std::map < double, double> one_hundred_eVHe;

void insert_obj(int _insert_element, std::vector<Object*>& _obj_ptr_list, const Database& _database, const Setting& _setting) {
	int type = -1;  int dir = 0;
	int size0 = 0; int size1 = 0; int size2 = 0; int size3 = 0;
	double z = 0;
	double upper(0), lower(0);
	auto iter = one_hundred_eVBe.upper_bound(uni());

	switch (_insert_element) {
	case 0:	size0 = 1; break;
	case 1: size1 = 1; break;
	case 2: 
		size2 = 1;
		upper = iter->second; iter--; lower = iter->second;
		z = lower + (upper - lower) * uni();
		break;
	case 3: 
		size3 = 1; 
		Object::he_insert_num++;
		z = (0.16 / _setting.a0 * 10) - (1.65 / _setting.a0 * 10) * log(1.0 - uni());
		break;
	default: std::cout << "insert_obj中_insert_element异常：" << _insert_element << '\n'; break;
	}

	double x = uni() * _setting.box_max[0];
	double y = uni() * _setting.box_max[1];

	Object* ptr_obj = new Object(type, x, y, z, size0, size1, size2, size3, dir, _database, _setting);
	_obj_ptr_list.push_back(ptr_obj);

	//check_reaction，对刚插入的obj检查反应
	int pos1 = _obj_ptr_list.size() - 1, pos2 = 0;
	if (check_dist_inner_loop(_obj_ptr_list, pos1, pos2, _setting))
	{
		carry_out_reaction(_obj_ptr_list, pos1, pos2, *_obj_ptr_list.at(pos1), *_obj_ptr_list.at(pos2), _database, _setting);
	}
}