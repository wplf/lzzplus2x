#include"event.h"
#include <cassert>



//  -1~insert_cascade, 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation, 6~trap_mutation, 7~He_insert_3, 8~Be_insert_2
double build_event_list(std::vector<Object*>& _obj_ptr_list, std::vector<Event>& _event_list, const Setting& _setting)
{
	_event_list.clear();
	double rate_sum = 0;
	
	// ��������obj�п��ܷ������¼�
	// ����obj
	for (auto iter = _obj_ptr_list.cbegin(); iter != _obj_ptr_list.cend(); iter++) {
		Event temp_event;
		// ��������frequency
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
	
	// �ⲿcascade���ڲ���������obj
	if (_setting.rate_cascade > 0) {
		Event temp_event;
		rate_sum += _setting.rate_cascade;
		temp_event.rate = rate_sum;
		//����cascade����¼���Pos = -1, which_event = -1
		temp_event.obj_pos = -1;
		temp_event.event_type = -1;
		_event_list.push_back(temp_event);
	}
	if (_setting.He_insert_rate > 0) {
		Event temp_event;
		rate_sum += _setting.He_insert_rate;
		temp_event.rate = rate_sum;
		//He_insert��Pos = 0, which_event = 7
		temp_event.obj_pos = 0;
		temp_event.event_type = 7;
		_event_list.push_back(temp_event);
	}
	if (_setting.Be_insert_rate > 0) {
		Event temp_event;
		rate_sum += _setting.Be_insert_rate;
		temp_event.rate = rate_sum;
		//Be_insert��Pos = 0, which_event = 8
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
	//binary search ���ֲ���
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

//����object��obj_ptr_list�е�λ�ã��Լ���Ҫִ�еķ�Ӧ��ִ�и÷�Ӧ
//�����Ǩ���¼���Ǩ��֮��Ӧ�����ײ�������ײ��_flag_refresh_event_list����Ϊtrue��׼���ؽ�event_list
//���ַ����¼�������Ҫ�ؽ�event_list
void carry_out_event(int _which_event, int _obj_pos, std::vector<Object*>& _obj_ptr_list, bool& _flag_refresh_event_list,
	const Database& _database,  const Setting& _set)
{
	switch (_which_event)
	{
	case 0:
	{
		//ִ��Ǩ�Ʋ���
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
	// mig�ڲ��Ѿ�ʵ�ֲ�ͬ������Ŵ�mig
	_obj_ptr_list.at(_obj_pos)->mig(_setting);

	//ִ����Ǩ���¼����Ƿ���Ҫcheck_reaction
	bool flag_check_reaction = true;

	// ���ÿ������ж��Ƿ�sink���� 1. �����뾶 2. ����(sink strength), 3 λ���ܶ�
	// ��������գ���Ϊfalse
	bool check_sink_strength = true;


	// �ڲ�Ϊ�ȼ��߽���������ˢ�¸��ӣ��ټ���Ƿ���Ҫ��Ӧ��
	// �����Ա߽������� �˴�Ҳ�ɽ��߽�������װ��ĳ�������У���û��Ҫ
	if (_setting.boundary_condition == 0)
	{
		_obj_ptr_list.at(_obj_pos)->check_pbc(_setting);
		// ˢ������
		_obj_ptr_list.at(_obj_pos)->refresh_grid_pos(_setting);

		// ����
		if (_setting.grain_radius > 0)
		{
			//���λ�Ƴ��������뾶
			if (_obj_ptr_list.at(_obj_pos)->check_grain_radius(_setting)) {
				if (_setting.output_GB_absorption > 0) {
					output_react_1_0_GB(*(_obj_ptr_list.at(_obj_pos)), _setting);
				}
				delete _obj_ptr_list.at(_obj_pos);
				_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
				//Ǩ�Ƶ�obj��ɱ���ˣ�����Ҫ����event list
				_flag_refresh_event_list = true;
				//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ���check_reaction
				flag_check_reaction = false;
			}
		}
		//ͨ��sink strength���ж�Obj�Ƿ񴩹�����
		if (_setting.ss_grain_radius > 0 && check_sink_strength)
		{
			int type = _obj_ptr_list.at(_obj_pos)->type;
			//���Obj�ǿ�λ����ô��һ�����������Ƿ�С��p_vac_meet_GB�����С�ڣ�kill��obj
			if (type == 1)
			{
				if (uni() < _setting.p_vac_meet_GB)
				{
					if (_setting.output_GB_absorption > 0) {
						output_react_1_0_GB(*(_obj_ptr_list.at(_obj_pos)), _setting);
					}
					delete _obj_ptr_list.at(_obj_pos);
					_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
					//Ǩ�Ƶ�obj��ɱ���ˣ�����Ҫ����event list
					_flag_refresh_event_list = true;
					//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ���check_reaction
					flag_check_reaction = false;
					//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ������ж��Ƿ�����sink����
					check_sink_strength = false;
				}
			}
			//���Obj��sia����ô��һ�����������Ƿ�С��p_sia_meet_GB�����С�ڣ�kill��obj
			else if (type == 2)
			{
				if (uni() < _setting.p_sia_meet_GB)
				{
					if (_setting.output_GB_absorption > 0) {
						output_react_1_0_GB(*(_obj_ptr_list.at(_obj_pos)), _setting);
					}
					delete _obj_ptr_list.at(_obj_pos);
					_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
					//Ǩ�Ƶ�obj��ɱ���ˣ�����Ҫ����event list
					_flag_refresh_event_list = true;
					//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ���check_reaction
					flag_check_reaction = false;
					//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ������ж��Ƿ�����sink����
					check_sink_strength = false;
				}
			}
		}
		// λ���ܶ�
		if (_setting.dislocation_density > 0 && check_sink_strength)
		{
			if (uni() < _obj_ptr_list.at(_obj_pos)->p_meet_dislocation)
			{
				if (_setting.output_disloaction_absorption > 0) {
					output_react_1_0_dislocation(*(_obj_ptr_list.at(_obj_pos)), _setting);
				}
				delete _obj_ptr_list.at(_obj_pos);
				_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
				//Ǩ�Ƶ�obj��ɱ���ˣ�����Ҫ����event list
				_flag_refresh_event_list = true;
				//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ���check_reaction
				flag_check_reaction = false;
				//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ������ж��Ƿ�����sink����
				check_sink_strength = false;
			}
		}

		//�����Ա߽����� Ǩ����֮�󣬿���������Obj�Ƿ�ײ��
		if (flag_check_reaction)
		{
			int flag1_pos2 = 0;
			bool flag1 = check_dist_0free_inner_loop(_obj_ptr_list, _obj_pos, flag1_pos2, _setting);
			if (flag1) {
				carry_out_reaction(_obj_ptr_list, _obj_pos, flag1_pos2, *_obj_ptr_list.at(_obj_pos), *_obj_ptr_list.at(flag1_pos2), _database, _setting);
				//���ִ���˷�Ӧ��������obj�����仯�������Ҫ����event_list
				_flag_refresh_event_list = true;
			}
		}
	}
	//��������ձ߽�����
	else if (_setting.boundary_condition == 3) {
		//���ײ���߽��ˣ��Ͱ�objȥ�������Ҳ���check_reaction
		if (_obj_ptr_list.at(_obj_pos)->check_3free(_setting) ) {
			if ( (_setting.output_out_bound > 0) || (_setting.output_big_clu_out_bound > 0 && 
				_obj_ptr_list.at(_obj_pos)->size[0] + _obj_ptr_list.at(_obj_pos)->size[1] + _obj_ptr_list.at(_obj_pos)->size[2] + _obj_ptr_list.at(_obj_pos)->size[3] > _setting.big_clu_definition
				) ) {
				output_react_1_0(*(_obj_ptr_list.at(_obj_pos)), _setting);
			}
			delete _obj_ptr_list.at(_obj_pos);
			_obj_ptr_list.erase(std::begin(_obj_ptr_list) + _obj_pos);
			//Ǩ�Ƶ�obj��ɱ���ˣ�����Ҫ����event list
			_flag_refresh_event_list = true;
			//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ���check_reaction
			flag_check_reaction = false;
		}
		//���û��ײ���߽磬��Ӧ�ø�������Object���ڵ�����
		else {
			_obj_ptr_list.at(_obj_pos)->refresh_grid_pos(_setting);
		}

		//Ǩ����֮�󣬿���������Obj�Ƿ�ײ��
		if (flag_check_reaction)
		{
			int pos2 = 0;
			if (check_dist_3free_inner_loop(_obj_ptr_list, _obj_pos, pos2, _setting))
			{
				carry_out_reaction(_obj_ptr_list, _obj_pos, pos2, *_obj_ptr_list.at(_obj_pos), *_obj_ptr_list.at(pos2),
					_database, _setting);
				//���ִ���˷�Ӧ��������obj�����仯�������Ҫ����event_list
				_flag_refresh_event_list = true;
			}
		}
	}
	//�����XY���ڣ�Z����
	else if (_setting.boundary_condition == 1)
	{
		// �ȼ��߽磬��ˢ�±�obj������
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
			//Ǩ�Ƶ�obj��ɱ���ˣ�����Ҫ����event list
			_flag_refresh_event_list = true;
			//Ǩ�Ƶ��Ǹ�Obj��ɱ���ˣ����Բ���check_reaction
			flag_check_reaction = false;
		}
		else{
			_obj_ptr_list.at(_obj_pos)->refresh_grid_pos(_setting);
		}
		// Ǩ����ɼ�鷴Ӧ
		if (flag_check_reaction)
		{
			int pos2 = 0;
			if (check_dist_zfree_inner_loop(_obj_ptr_list, _obj_pos, pos2, _setting))
			{
				carry_out_reaction(_obj_ptr_list, _obj_pos, pos2, *_obj_ptr_list.at(_obj_pos), *_obj_ptr_list.at(pos2),
					_database, _setting);
				//���ִ���˷�Ӧ��������obj�����仯�������Ҫ����event_list
				_flag_refresh_event_list = true;
			}
		}
	}
}

// emit ֮��ԭ�Ŵػ��С�����Բ����ж�ԭ�Ŵصķ�Ӧ
void carry_out_emit(int _emit_element, std::vector<Object*>& _obj_ptr_list, Object& _obj, int _obj_pos, const Database& _database, const Setting& _setting)
{
	//����
	// ����������obj
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
	else { std::cerr << "����Ԫ����Ԫ�س���\n"; }

	//����һ��0-2pi֮����������ֵ��ȷ��������Ԫ�ص�����
	double angle1 = uni() * 2 * 3.1415926;
	double angle2 = uni() * 2 * 3.1415926;

	//��뾶Ϊĸobj�İ뾶��3
	double r = _obj.radius + 3;
	o2x = o1x + r * sin(angle1) * cos(angle2);
	o2y = o1y + r * sin(angle1) * sin(angle2);
	o2z = o1z + r * cos(angle1);

	bool flag_o2 = check_boundary(o2x, o2y, o2z, _setting);

	Object* ptr_o1 = new Object(o1type, o1x, o1y, o1z, o1size0, o1size1, o1size2, o1size3, o1dir, _database, _setting);
	Object* ptr_o2 = new Object(o2type, o2x, o2y, o2z, o2size0, o2size1, o2size2, o2size3, o2dir, _database, _setting);

	// �Ƿ���������¼�, �����¼����������ɵ������Ŵأ� ���ڲ������л�����Ŵ�size��
	if (_setting.output_emit_reaction > 0) {
		output_react_1_2(_obj, *ptr_o1, *ptr_o2, _setting);
	}

	// ɾ�����Ŵ�
	delete(_obj_ptr_list.at(_obj_pos));
	_obj_ptr_list.erase(_obj_ptr_list.begin() + _obj_pos);

	// �ȼ���o1
	_obj_ptr_list.push_back(ptr_o1);

	// ���o2���ͼ���o2
	if (flag_o2) {
		_obj_ptr_list.push_back(ptr_o2);
		//check_reaction�����³�����objѭ��һ�ּ��ɣ�pos1�Ǳ��³�����obj������
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

// t_m ֮��ԭ�Ŵػ���������Ҫ�ж�ԭ�Ŵصķ�Ӧ
void carry_out_tm(std::vector<Object*>& _obj_ptr_list, Object& _obj, int _obj_pos, const Database& _database, const Setting& _setting)
{
	// t_m������1
	Object::t_m_num++;

	// ����������obj
	int o1size0 = _obj.size[0], o1size1 = _obj.size[1], o1size2 = _obj.size[2], o1size3 = _obj.size[3];
	int o2size0 = 0, o2size1 = 1, o2size2 = 0, o2size3 = 0;
	int o1type = -1, o2type = -1, o1dir = 0, o2dir = 0;
	double o1x = _obj.pos[0], o1y = _obj.pos[1], o1z = _obj.pos[2];
	double o2x = 0, o2y = 0, o2z = 0;

	//����һ��0-2pi֮����������ֵ��ȷ��������Ԫ�ص�����
	double angle1 = uni() * 2 * 3.1415926;
	double angle2 = uni() * 2 * 3.1415926;

	//��뾶Ϊĸobj�İ뾶��3
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

	// �Ƿ���������¼�, �����¼����������ɵ������Ŵأ� ���ڲ������л�����Ŵ�size��
	if (_setting.output_emit_reaction > 0) {
		output_react_1_2(_obj, *ptr_o1, *ptr_o2, _setting);
	}

	// ɾ�����Ŵ�
	delete(_obj_ptr_list.at(_obj_pos));
	_obj_ptr_list.erase(_obj_ptr_list.begin() + _obj_pos);

	// �ȼ���o1,���жϷ�Ӧ
	_obj_ptr_list.push_back(ptr_o1);

	// ��t_m������obj���з�Ӧ�жϣ� �������Ϊ�ж���ϵ�����п��ܷ����ķ�Ӧ������ֻ��һ���Ŵؽ��з�Ӧ�ж�
	int pos1 = 0, pos2 = 0;
	while (check_dist(_obj_ptr_list, pos1, pos2, _setting))
	{
		carry_out_reaction(_obj_ptr_list, pos1, pos2, *_obj_ptr_list.at(pos1), *_obj_ptr_list.at(pos2), _database, _setting);
		pos2 = 0;
	}

	// ���o2���ͼ���o2�����жϷ�Ӧ
	if (flag_o2) {
		_obj_ptr_list.push_back(ptr_o2);
		//check_reaction�����³�����objѭ��һ�ּ��ɣ�pos1�Ǳ��³�����obj������
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


// �޸�event�в���Be����ȷֲ�������������ΪSRIM 100 eV����
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
	default: std::cout << "insert_obj��_insert_element�쳣��" << _insert_element << '\n'; break;
	}

	double x = uni() * _setting.box_max[0];
	double y = uni() * _setting.box_max[1];

	Object* ptr_obj = new Object(type, x, y, z, size0, size1, size2, size3, dir, _database, _setting);
	_obj_ptr_list.push_back(ptr_obj);

	//check_reaction���Ըղ����obj��鷴Ӧ
	int pos1 = _obj_ptr_list.size() - 1, pos2 = 0;
	if (check_dist_inner_loop(_obj_ptr_list, pos1, pos2, _setting))
	{
		carry_out_reaction(_obj_ptr_list, pos1, pos2, *_obj_ptr_list.at(pos1), *_obj_ptr_list.at(pos2), _database, _setting);
	}
}