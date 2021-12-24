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
	//��ʼ������ļ�
	initialize_react_file();
	//��ȡ���ã����������á����󣬳�ʼ�������
	Setting settings("input_setting.txt");
	//���input_setting��û��ָ����������ӣ�����ϵͳ��ǰʱ��Ϊ���������
	if (settings.seed <= 0){
		settings.seed = (unsigned)time(NULL);
	}
	std::cout << "The seed is " << settings.seed << std::endl;
	suni(settings.seed);

	// ��ʼ��database
	Database* database = new Database("input_posb.txt");

	// ����obj_ptr���б� �������Ľ�����Ϊobj��ָ�룬��Ҫ��->�������Ժͷ��� 
	std::vector<Object*> obj_ptr_list;

	read_cascade("inicas.txt", obj_ptr_list , *database, settings);
	

	// ����ʼ������, ����ϣ�������ϼ��t_m
	int pos1 = 0, pos2 = 0;
	while (check_dist(obj_ptr_list, pos1, pos2, settings))
	{
		carry_out_reaction(obj_ptr_list, pos1, pos2, *obj_ptr_list.at(pos1), *obj_ptr_list.at(pos2), *database,  settings);
		pos2 = 0;
	}
	//��ʼ��ѭ��֮ǰ���һ��
	output_dump(obj_ptr_list, settings, true);
	output_txt(obj_ptr_list, settings, true);
	output_cascade(obj_ptr_list, settings);

	// start the main loop
	std::cout << "start main loop\n";

	bool flag_refresh_event_list = true;
	double rate_sum = 0; 
	std::vector<Event> event_list;

	//��ѭ��
	while (settings.check_end()) {
		// ͨ��flag_refresh_event_list �ж� obj_ptr_list �Ƿ�ı䣬 ����ı伴����event_list
		if (flag_refresh_event_list) {
			rate_sum = build_event_list(obj_ptr_list, event_list, settings);
			flag_refresh_event_list = false;
		}
		// �¼��б���ڣ� ��ȡ�¼��� ʱ�����ӣ� �������ӡ�
		if (event_list.size() != 0) {
			// �ҵ��¼�
			int event_pos = find_event(event_list, rate_sum);

			//����鵽����cascade�����¼�
			if (event_list.at(event_pos).event_type == -1)
			{
				//����cascade
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
			//��������鵽����object���¼� or He_insert
			else
			{
				//ִ���¼�
				//���ܵ��¼���Ǩ�ơ���ת�����䡢cascade����
				//�������Ǩ�ƻ��߷��䣬�����Ƿ�ײ�ϱ��obj
				int obj_pos = event_list.at(event_pos).obj_pos;	//��obj��obj_ptr_list�е�λ��
				int which_event = event_list.at(event_pos).event_type;
				carry_out_event(which_event, obj_pos, obj_ptr_list, flag_refresh_event_list, *database, settings);
			}
			//����ʱ��
			settings.increase_time_step(rate_sum);
		} 
		else { // rate_sum==0 û���¼���
			settings.time += settings.default_dt;
			settings.step += 1;
		}

		// step����output
		if (settings.output_per_n_step > 0 && settings.step % settings.output_per_n_step == 0) {
			output_dump(obj_ptr_list, settings, false);
			output_txt(obj_ptr_list, settings, false);
			output_cascade(obj_ptr_list, settings);
		}

		//����Ƿ���Ҫ���
		if (settings.check_output())
		{ 
			output_dump(obj_ptr_list, settings, false);
			output_txt(obj_ptr_list, settings, false);
			output_cascade(obj_ptr_list, settings);
		}
	}
	
	//�����ʱ��Ϊ������׼�������һ��ִ����֮��ʱ�䳬��max_time�����ʱ������max_time
	if (settings.stop_criteria == 1 && settings.time > settings.max_time) {
		settings.time = settings.max_time;
	}

	//��ֹ��ѭ��֮�������һ��
	output_dump(obj_ptr_list, settings, false);
	output_txt(obj_ptr_list, settings, false);
	output_cascade(obj_ptr_list, settings);

	//�ͷ�����obj���ڴ�
	for (auto iter : obj_ptr_list) {
		delete iter;
	}
	//�ͷ�����database���ڴ�
	delete database;

	std::cout << "work is done" << std::endl;
	system("pause");

	return 0;
}

