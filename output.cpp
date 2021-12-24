#include "output.h"


//��ʼ��output files
void initialize_output_file(std::string _filename)
{
	std::ostringstream oss;
	std::ofstream fout;
	fout.open(_filename);
	fout << "step\ttime\tid\ttype\tx\ty\tz\tsize1\tsize2\tdir\n";
	fout.close();
}


//��ʼ��"output_react.txt"
void initialize_react_file()
{
	std::ostringstream oss;
	std::ofstream fout;
	fout.open("output_react.txt");
	fout << "#step\ttime\tcpu_time\t" << "id\ttype\tsize[0]\tsize[1]\tsize[2]\tsize[3]\t" 
		<< "id\ttype\tsize[0]\tsize[1]\tsize[2]\tsize[3]\t" 
		<< "react\t"
		<< "id\ttype\tsize[0]\tsize[1]\tsize[2]\tsize[3]\t" << "id\ttype\tsize[0]\tsize[1]\tsize[2]\tsize[3]\n";
	fout.close();
}


//����Object��ȫ���𣬲���0����object
//��������object������Ӧ��Ϣд��"react.txt"
void output_react_2_0(const Object& _obj1, const Object& _obj2, const Setting& _set)
{
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< _obj1.id << '\t' << _obj1.type << '\t' 
		<< _obj1.size[0]<< '\t' << _obj1.size[1] << '\t' << _obj1.size[2] << '\t' << _obj1.size[3] << '\t'
		<< _obj2.id << '\t' << _obj2.type << '\t' 
		<< _obj2.size[0] << '\t' << _obj2.size[1] << '\t' << _obj2.size[2] << '\t' << _obj2.size[3] << '\t'
		<< "anneal\t-\t-\t-\t-\t-\t-\t-\t-\n";
	write_content("output_react.txt", oss, false);
}

// ����ϲ���Ӧ��obj1 + obj2 = obj3
void output_react_2_1(const Object& _obj1, const Object& _obj2, const Object& _obj3, const Setting& _set)
{
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< _obj1.id << '\t' << _obj1.type << '\t' 
		<< _obj1.size[0] << '\t' << _obj1.size[1] << '\t' << _obj1.size[2] << '\t' << _obj1.size[3] << '\t'
		<< _obj2.id << '\t' << _obj2.type << '\t' 
		<< _obj2.size[0] << '\t' << _obj2.size[1] << '\t' << _obj2.size[2] << '\t' << _obj2.size[3] << '\t'
		<< "react\t"
		<< _obj3.id << '\t' << _obj3.type << '\t' 
		<< _obj3.size[0] << '\t' << _obj3.size[1] << '\t' << _obj3.size[2] << '\t' << _obj3.size[3] << '\t'
		<< "-\t-\t-\t-\n";
	write_content("output_react.txt", oss, false);
}


//obj reach bound in absorb boundary condition
void output_react_1_0(const Object& _obj1, const Setting& _set)
{
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< _obj1.id << '\t' << _obj1.type << '\t' 
		<< _obj1.size[0] << '\t' << _obj1.size[1] << '\t' << _obj1.size[2] << '\t' << _obj1.size[3] << '\t'
		<< "-\t-\t-\t-\t"
		<< "out_bound\t"
		<< "-\t-\t-\t-\t-\t-\t-\t-\n";
	write_content("output_react.txt", oss, false);
}

//obj����������
void output_react_1_0_GB(const Object& _obj1, const Setting& _set)
{
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< _obj1.id << '\t' << _obj1.type << '\t'
		<< _obj1.size[0] << '\t' << _obj1.size[1] << '\t' << _obj1.size[2] << '\t' << _obj1.size[3] << '\t'
		<< "-\t-\t-\t-\t"
		<< "GB\t"
		<< "-\t-\t-\t-\t-\t-\t-\t-\n";
	write_content("output_react.txt", oss, false);
}

//obj��λ������
void output_react_1_0_dislocation(const Object& _obj1, const Setting& _set)
{
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< _obj1.id << '\t' << _obj1.type << '\t'
		<< _obj1.size[0] << '\t' << _obj1.size[1] << '\t' << _obj1.size[2] << '\t' << _obj1.size[3] << '\t'
		<< "-\t-\t-\t-\t"
		<< "dislocation\t"
		<< "-\t-\t-\t-\t-\t-\t-\t-\n";
	write_content("output_react.txt", oss, false);
}


//cascade insert
void output_react_0_0(const Setting& _set, std::string str)
{
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< "-\t-\t-\t-\t-\t-\t-\t-\t"
		<< str
		<< "\t-\t-\t-\t-\t-\t-\t-\t-\n";
	write_content("output_react.txt", oss, false);
}

// �ڹ̶���ʱ�̣�������object����Ϣ����dump����ʽ���
void output_dump(const std::vector<Object*>& _obj_ptr_list, const Setting& _set, bool _initialize = false)
{
	std::ostringstream oss;
	oss << "ITEM: TIMESTEP\n\t" << _set.step << "\nITEM: NUMBER OF ATOMS\n\t" << _obj_ptr_list.size() << "\nITEM: BOX BOUNDS\n\t"
		<< _set.box_min.at(0) << '\t' << _set.box_max.at(0) << "\n\t"
		<< _set.box_min.at(1) << '\t' << _set.box_max.at(1) << "\n\t"
		<< _set.box_min.at(2) << '\t' << _set.box_max.at(2) << '\n'
		<< "ITEM: ATOMS id type x y z V SIA fia1 fia2 radius time\n";
	for (auto iter = _obj_ptr_list.cbegin(); iter != _obj_ptr_list.cend(); iter++)
	{
		oss << '\t' << (*iter)->id << '\t' << (*iter)->type << '\t' <<
			(*iter)->pos.at(0) << '\t' << (*iter)->pos.at(1) << '\t' << (*iter)->pos.at(2) << '\t' <<
			(*iter)->size[0] << '\t' << (*iter)->size[1] << '\t' << (*iter)->size[2] << '\t' << (*iter)->size[3] 
			<< '\t' << (*iter)->radius << '\t' <<_set.time << "\n";
	}
	write_content("output_dump.dump", oss, _initialize);
}

//�ڹ̶�ʱ�̣����ÿһ��Object�ĳߴ���Ϣ��true�����ʼ������ļ�
void output_txt(const std::vector<Object*>& _obj_ptr_list, const Setting& _set, bool _initialize = false)
{
	std::ostringstream oss_vac, oss_sia, oss_be, oss_he, oss_vac_be, oss_vac_he, oss_vac_be_he, oss_be_he, oss_sia_he ;
	std::ostringstream oss_he_retention_rate, oss_he_avg_size;
	std::ostringstream oss_trap_mutation_annihilation;

	int he_cur_num = 0;
	int he_clu_cnt = 0;

	double duration = cal_duration();
	oss_vac << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_sia << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_be << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_he << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_vac_be << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_vac_he << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_sia_he << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_be_he << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	oss_vac_be_he << "step\t" << _set.step << "\ttime\t" << _set.time << "\ttemperature\t" << _set.temperature
		<< "\tcpu_time\t" << duration << std::endl;
	for (auto iter : _obj_ptr_list)
	{
		switch (iter->type)
		{
		case 1:
			oss_vac << iter->size[0] << std::endl;
			break;
		case 2:
			oss_sia << iter->size[1] << std::endl;
			break;
		case 3:
			oss_be << iter->size[2] << std::endl;
			break;
		case 4:
			oss_he << iter->size[3] << std::endl;
			break;
		case 6:
			oss_vac_be << iter->size[0] << '\t' << iter->size[2] << std::endl;
			break;
		case 7:
			oss_vac_he << iter->size[0] << '\t' << iter->size[3] << std::endl;
			break;
		case 9:
			oss_sia_he << iter->size[1] << '\t' << iter->size[3] << std::endl;
			break;
		case 10:
			oss_be_he << iter->size[2] << '\t' << iter->size[3] << std::endl;
			break;
		case 13:
			oss_vac_be_he << iter->size[0] << '\t' << iter->size[2] << '\t' << iter->size[3] << std::endl;
			break;
		}
		if (iter->size[3] > 0) {
			he_cur_num += iter->size[3];
			he_clu_cnt++;

		}
	}
	oss_he_retention_rate << _set.time  << '\t' << _set.time * _set.He_flux << '\t' << he_cur_num << '\t' << Object::he_insert_num << '\n';
	oss_he_avg_size << _set.time << '\t' << he_clu_cnt << '\t' << he_cur_num << '\n';
	oss_trap_mutation_annihilation << _set.time << '\t' << Object::annihilation_num << '\t' << Object::t_m_num << '\n';

	write_content("he_retention_rate.txt", oss_he_retention_rate, _initialize);
	write_content("he_avg_size.txt", oss_he_avg_size, _initialize);
	write_content("trap_mutation_annihilation.txt", oss_trap_mutation_annihilation, _initialize);
	write_content("time_vac.txt", oss_vac, _initialize);
	write_content("time_sia.txt", oss_sia, _initialize);
	write_content("time_be.txt", oss_be, _initialize);
	write_content("time_he.txt", oss_he, _initialize);
	write_content("time_vac_be.txt", oss_vac_be, _initialize);
	write_content("time_vac_he.txt", oss_vac_he, _initialize);
	write_content("time_sia_he.txt", oss_vac_he, _initialize);
	write_content("time_be_he.txt", oss_be_he, _initialize);
	write_content("time_vac_be_he.txt", oss_vac_be_he, _initialize);
}

void output_cascade(const std::vector<Object*>& _obj_ptr_list, const Setting& _set)
{
	std::ostringstream oss;
	oss << "#steps\t" << _set.step << "\ttime\t" << _set.time << "\tseed\t" << _set.seed << '\n' <<
		"#type 1~vac, 2~sia, 3~fia, 4~vac_fia, 5~sia_fia\n"
		"#no direction~0;	111~1;	-111~2;	1 - 11~3;	11 - 1~4\n" <<
		"#id	type	x	y	z	size1	size2	dir\n";
	for (auto iter : _obj_ptr_list)
	{
		Object& obj = *iter;
		oss << obj.id << '\t' << obj.type << '\t' <<
			obj.pos.at(0) << '\t' << obj.pos.at(1) << '\t' << obj.pos.at(2) << '\t' <<
			obj.size[0] << '\t' << obj.size[1] << '\t' << obj.size[2] << '\t' << obj.size[3]<< '\t' << obj.dir << std::endl;
	}
	write_content("output_cascade.txt", oss, true);
}

//�����ļ������ַ����������ɽ�������ӵ���Ӧ�ļ���
//����������Ϊtrue����Ӧ'w'ģʽ������������Ϊfalse����Ӧ'a'ģʽ
void write_content(std::string _filename, std::ostringstream& _oss, bool _initialize)
{
	std::ofstream out_file;
	// ����ģʽд�룬��һ�ε���ʱʹ��
	if (_initialize) {
		out_file.open(_filename);
	}
	// ׷��ģʽд��
	else {
		out_file.open(_filename, std::ios::out | std::ios::app);
	}
	out_file << _oss.str();
	out_file.close();
}

//���emit��Ӧ
void output_react_1_2(const Object& _obj1, const Object& _obj2, const Object& _obj3, const Setting& _set)
{
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< _obj1.id << '\t' << _obj1.type << '\t' << _obj1.size[0] << '\t' << _obj1.size[1] << '\t' << _obj1.size[2] << '\t' << _obj1.size[3] << '\t'
		<< "-\t-\t-\t-\temit\t"
		<< _obj2.id << '\t' << _obj2.type << '\t' << _obj2.size[0] << '\t' << _obj2.size[1] << '\t' << _obj2.size[2] << '\t' << _obj2.size[3] << '\t'
		<< _obj3.id << '\t' << _obj3.type << '\t' << _obj3.size[0] << '\t' << _obj3.size[1] << '\t' << _obj3.size[2] << '\t' << _obj3.size[3] << '\n';
	write_content("output_react.txt", oss, false);
}

void output_trap_mutation(const Object& _obj1, const Object& _obj2, const Object& _obj3, const Setting& _set) {
	double duration = cal_duration();
	std::ostringstream oss;
	oss << _set.step << '\t' << _set.time << '\t' << duration << '\t'
		<< _obj1.id << '\t' << _obj1.type << '\t' << _obj1.size[0] << '\t' << _obj1.size[1] << '\t' << _obj1.size[2] << '\t' << _obj1.size[3] << '\t'
		<< "-\t-\t-\t-\tt_m\t"
		<< _obj2.id << '\t' << _obj2.type << '\t' << _obj2.size[0] << '\t' << _obj2.size[1] << '\t' << _obj2.size[2] << '\t' << _obj2.size[3] << '\t'
		<< _obj3.id << '\t' << _obj3.type << '\t' << _obj3.size[0] << '\t' << _obj3.size[1] << '\t' << _obj3.size[2] << '\t' << _obj3.size[3] << '\n';
	write_content("output_react.txt", oss, false);
}
