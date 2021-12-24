#include "setting.h"



// 输入文件名，调用read_setting函数，生成setting_map，并用setting_map初始化得到一个setting对象
Setting::Setting(std::string filename)
{
	read_setting(filename);
	temperature = setting_map["temperature"];
	
	box_min.at(0) = int(setting_map["x_min"]);
	box_min.at(1) = int(setting_map["y_min"]);
	box_min.at(2) = int(setting_map["z_min"]);
	box_max.at(0) = int(setting_map["x_max"]);
	box_max.at(1) = int(setting_map["y_max"]);
	box_max.at(2) = int(setting_map["z_max"]);
	box_grid = int(setting_map["box_grid"]);
	for (int i = 0; i < 3; i++){box_grid_num[i] = (int)ceil((box_max[i] - box_min[i]) / (box_grid)) - 1;}
	for (int i = 0; i < 3; i++) { box_length[i] = box_max.at(i) - box_min.at(i); }

	Be_trap_mutation = setting_map["Be_trap_mutation"];
	TM_sia2surface = (int)setting_map["TM_sia2surface"];

	He_insert = setting_map["He_insert"];
	He_flux = setting_map["He_flux"];
	Be_insert = setting_map["Be_insert"];
	Be_flux = setting_map["Be_flux"];

	max_step = (unsigned long long)setting_map["max_step"];
	max_time = setting_map["max_time"];
	a0 = setting_map["a0"];
	kb = setting_map["kb"];
	hp = setting_map["hp"];

	stop_criteria = static_cast<int>(setting_map["stop_criteria"]);
	boundary_condition = static_cast<int>(setting_map["boundary_condition"]);


	output_cascade_injection = setting_map["output_cascade_injection"];
	output_combine_reaction = setting_map["output_combine_reaction"];
	output_annihilation_reaction = setting_map["output_annihilation_reaction"];
	output_emit_reaction = setting_map["output_emit_reaction"];
	output_out_bound = setting_map["output_out_bound"];
	output_GB_absorption = setting_map["output_GB_absorption"];
	output_disloaction_absorption = setting_map["output_disloaction_absorption"];
	output_big_clu_out_bound = setting_map["output_big_clu_out_bound"];


	big_clu_definition = setting_map["big_clu_definition"];
	output_trap_mutation = setting_map["output_trap_mutation"];

	total_output = int(setting_map["total_output"]);
	output_per_n_step = int(setting_map["output_per_n_step"]);
	n_output = 1;


	cascade_interval = setting_map["cascade_interval"];
	rand_translate_cascade = (int)setting_map["rand_translate_cascade"];
	rand_rotate_cascade = (int)setting_map["rand_rotate_cascade"];

	mig_degeneration = (int)setting_map["mig_degeneration"];
	max_energy = setting_map["max_energy"];
	grain_radius = setting_map["grain_radius"];
	grain_radius_square = pow(grain_radius, 2);
	default_dt = setting_map["default_dt"];
	ss_grain_radius = setting_map["ss_grain_radius"];
	dislocation_density = setting_map["dislocation_density"];
	dislocation_radius = setting_map["dislocation_radius"];
	seed = setting_map["seed"];

	cal_sink_strength();
	cal_cascade_insert_rate();
	cal_He_insert();
	cal_Be_insert();

	//下一次插入cascade的时间
	cascade_insert_time = cascade_interval;
	//程序运行的百分比进度
	progress_percentage = 1;

	step = 0;
	time = 0.0;
	bcc_site = {
		0.0, 0.0, 0.0,
		0.5, 0.5, 0.5,
		1.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		1.0, 1.0, 0.0,
		1.0, 0.0, 1.0,
		0.0, 1.0, 1.0,
		1.0, 1.0, 1.0 };
	tis_site = {
		0.25, 0.50, 0.0,   0.25, 0.50, 1.0,
		0.50, 0.25, 0.0,   0.50, 0.25, 1.0,
		0.75, 0.50, 0.0,   0.75, 0.50, 1.0,
		0.50, 0.75, 0.0,   0.50, 0.75, 1.0,

		0.25, 0.0, 0.50,   0.25, 1.0, 0.50,
		0.50, 0.0, 0.25,   0.50, 1.0, 0.25,
		0.75, 0.0, 0.50,   0.75, 1.0, 0.50,
		0.50, 0.0, 0.75,   0.50, 1.0, 0.75,

		0.0, 0.50, 0.25,   1.0, 0.50, 0.25,
		0.0, 0.25, 0.50,   1.0, 0.25, 0.50,
		0.0, 0.50, 0.75,   1.0, 0.50, 0.75,
		0.0, 0.75, 0.50,   1.0, 0.75, 0.50, };
	ois_site = {
		0.0, 0.5, 0.5,		1.0, 0.5, 0.5,
		0.5, 0.0, 0.5,		0.5, 1.0, 0.5,
		0.5, 0.5, 0.0,		0.5, 0.5, 1.0,
	};

	read_cascade_possibility();
	if (rate_cascade > 0) {
		read_cascade_map();
	}
}

// 输入文件名，读取文件中所有非空，非#开头的行，以第一个关键字为键，第二个关键字为值，构造setting map
void Setting::read_setting(std::string _filename)
{
	std::fstream inFile(_filename);
	if (inFile.is_open())
	{
		std::string str;
		while (std::getline(inFile, str))
		{
			if (!str.empty() && str.at(0) != '#')	//skip the empty line and the line started with '#'
			{
				std::istringstream ss(str);
				std::string temp;
				ss >> temp;
				ss >> setting_map[temp];
			}
		}
		std::cout << _filename << " is succssefully read.\n";
		inFile.close();
	}
	else {
		std::cout << "Fail to open " << _filename << std::endl;
	}
}

//读取cascade_possibility文件，记录待插入cascade的信息
void Setting::read_cascade_possibility()
{
	std::string str;
	std::ifstream inFile("input_cascade_possibility.txt");
	if (inFile.is_open()) {
		while (std::getline(inFile, str))
		{
			if (str.empty() || str.at(0) == '#') { continue; }	//skip the empty line and the line started with '#'
			std::istringstream ss(str);
			double e, p;
			int n;
			ss >> e >> p >> n;
			cascade_energy.push_back(e);
			cascade_possibility.push_back(p);
			cascade_number.push_back(n);
		}
		std::cout << "input_cascade_possibility.txt is successfully read\n";
		inFile.close();
	}
	else {
		std::cout << "Fail to open input_cascade_possibility.txt\n";
	}
}

void Setting::read_cascade_map() {
	for (int i = 0; i < cascade_energy.size(); i++) {
		double energy = cascade_energy.at(i);
		int n = cascade_number[i];
		for (int j = 0; j < n; j++) {
			std::ostringstream os;
			os << "cascade." << energy << '.' << j + 1 << ".txt";
			std::string filename = os.str();

			std::ifstream inFile(filename);
			if (inFile.is_open())
			{
				std::string str;
				while (std::getline(inFile, str))
				{
					if (!str.empty() && str.at(0) != '#') //skip the empty line and the line started with '#'
					{
						cas_map[filename].push_back(str);
					}
				}
				inFile.close();
				std::cout << "The " << filename << " is successfully read in cas_map.\n";
			}
			else {
				std::cout << "Fail to open " << filename << std::endl;
			}
		}
	}
}

void Setting::cal_sink_strength()
{
	if (ss_grain_radius > 0)
	{
		p_sia_meet_GB = 5.625 * a0 * a0 / ss_grain_radius / ss_grain_radius;
		p_vac_meet_GB = 1.8 * a0 * a0 / ss_grain_radius / ss_grain_radius;
		std::cout << "P_sia_meet_GB is " << p_sia_meet_GB << std::endl;
		std::cout << "P_vac_meet_GB is " << p_vac_meet_GB << std::endl;
	}
}

void Setting::cal_cascade_insert_rate()
{
	if (cascade_interval > 0)
	{
		rate_cascade = 1.0 / cascade_interval;
	}
	else {
		rate_cascade = -1.0;
	}
		
	std::cout << "The cascade insert rate is: " << rate_cascade << " per second." << std::endl;
}

void Setting::cal_He_insert() {
	// 根据表面积计算每秒入射rate， 并在build_event_list中判断，如果这个数不为0就加入这个事件。
	if (He_insert > 0) {
		double area = box_length[0] * box_length[1] * a0 * a0 * 1e-20;
		He_insert_rate = He_flux * area;
	}
	else {
		He_insert_rate = -1;
	}
	std::cout << "The He insert rate is: " << He_insert_rate << " per second." << std::endl;
}

void Setting::cal_Be_insert() {
	// 根据表面积计算每秒入射rate， 并在build_event_list中判断，如果这个数>0就加入这个事件。
	if (Be_insert > 0) {
		double area = box_length[0] * box_length[1] * a0 * a0 * 1e-20;
		Be_insert_rate = Be_flux * area;
	}
	else {
		Be_insert_rate = -1;
	}
	std::cout << "The Be insert rate is: " << Be_insert_rate << " per second." << std::endl;
}