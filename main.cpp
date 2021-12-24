#include <vector> 
#include <string> 
#include <fstream> 
#include <iostream> 

#include"database.h"
#include"event.h"
#include"object.h"
#include"setting.h"
#include"output.h"
#include"random.h"
#include"reaction.h"



int main()
{
	//初始化输出文件
	initialize_react_file();
	//读取设置，建立“设置”对象，初始化随机数
	Setting settings("input_setting.txt");
	//如果input_setting中没有指定随机数种子，则以系统当前时间为随机数种子
	if (settings.seed <= 0){
		settings.seed = (unsigned)time(NULL);
	}
	std::cout << "The seed is " << settings.seed << std::endl;
	suni(settings.seed);

	// 初始化database
	Database* database = new Database("input_posb.txt");

	// 储存obj_ptr的列表， 迭代器的解引用为obj的指针，需要用->调用属性和方法 
	std::vector<Object*> obj_ptr_list;

	read_cascade("inicas.txt", obj_ptr_list , *database, settings);
	

	// 检查初始结合情况, 检查结合，检查完结合检查t_m
	int pos1 = 0, pos2 = 0;
	while (check_dist(obj_ptr_list, pos1, pos2, settings))
	{
		carry_out_reaction(obj_ptr_list, pos1, pos2, *obj_ptr_list.at(pos1), *obj_ptr_list.at(pos2), *database,  settings);
		pos2 = 0;
	}
	//开始主循环之前输出一次
	output_dump(obj_ptr_list, settings, true);
	output_txt(obj_ptr_list, settings, true);
	output_cascade(obj_ptr_list, settings);

	// start the main loop
	std::cout << "start main loop\n";

	bool flag_refresh_event_list = true;
	double rate_sum = 0; 
	std::vector<Event> event_list;

	//主循环
	while (settings.check_end()) {
		// 通过flag_refresh_event_list 判断 obj_ptr_list 是否改变， 如果改变即更新event_list
		if (flag_refresh_event_list) {
			rate_sum = build_event_list(obj_ptr_list, event_list, settings);
			flag_refresh_event_list = false;
		}
		// 事件列表存在， 抽取事件， 时间增加， 步数增加。
		if (event_list.size() != 0) {
			// 找到事件
			int event_pos = find_event(event_list, rate_sum);

			//如果抽到的是cascade入射事件
			if (event_list.at(event_pos).event_type == -1)
			{
				//插入cascade
				std::string cascade_filename = choose_cascade_to_insert(settings);
				if (settings.output_cascade_injection > 0) {
					output_react_0_0(settings, cascade_filename);
				}
				read_cascade(cascade_filename, obj_ptr_list, *database, settings);
				int pos1 = 0, pos2 = 0;
				while (check_dist(obj_ptr_list, pos1, pos2, settings))
				{
					carry_out_reaction(obj_ptr_list, pos1, pos2, *obj_ptr_list.at(pos1), *obj_ptr_list.at(pos2), *database, settings);
					pos2 = 0;
				}
				flag_refresh_event_list = true;
			}
			//否则如果抽到的是object的事件 or He_insert
			else
			{
				//执行事件
				//可能的事件：迁移、旋转、发射、cascade入射
				//如果抽中迁移或者发射，则检查是否撞上别的obj
				int obj_pos = event_list.at(event_pos).obj_pos;	//该obj在obj_ptr_list中的位置
				int which_event = event_list.at(event_pos).event_type;
				carry_out_event(which_event, obj_pos, obj_ptr_list, flag_refresh_event_list, *database, settings);
			}
			//增加时间
			settings.increase_time_step(rate_sum);
		} 
		else { // rate_sum==0 没有事件，
			settings.time += settings.default_dt;
			settings.step += 1;
		}

		// step——output
		if (settings.output_per_n_step > 0 && settings.step % settings.output_per_n_step == 0) {
			output_dump(obj_ptr_list, settings, false);
			output_txt(obj_ptr_list, settings, false);
			output_cascade(obj_ptr_list, settings);
		}

		//检查是否需要输出
		if (settings.check_output())
		{ 
			output_dump(obj_ptr_list, settings, false);
			output_txt(obj_ptr_list, settings, false);
			output_cascade(obj_ptr_list, settings);
		}
	}
	
	//如果是时间为截至标准，且最后一步执行完之后，时间超过max_time，则把时间拉回max_time
	if (settings.stop_criteria == 1 && settings.time > settings.max_time) {
		settings.time = settings.max_time;
	}

	//终止主循环之后再输出一次
	output_dump(obj_ptr_list, settings, false);
	output_txt(obj_ptr_list, settings, false);
	output_cascade(obj_ptr_list, settings);

	//释放所有obj的内存
	for (auto iter : obj_ptr_list) {
		delete iter;
	}
	//释放所有database的内存
	delete database;

	std::cout << "work is done" << std::endl;
	system("pause");

	return 0;
}

