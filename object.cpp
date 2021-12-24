#include "object.h"


// obj的静态变量赋值
size_t Object::ID(1U);
long Object::he_insert_num = 0;
long Object::t_m_num = 0;
long Object::annihilation_num = 0;

Object::Object(int _type, double x, double y, double z, int _size0, int _size1, int _size2, int _size3, int _dir,
	const Database& _database, const Setting& _setting) {
	type = _type; id = ID++; pos[0] = x; pos[1] = y; pos[2] = z; dir = _dir;
	size[0] = _size0; size[1] = _size1; size[2] = _size2; size[3] = _size3;
	refresh_obj(_database, _setting);
}


bool Object::refresh_obj(const Database& _database, const Setting& _setting){

	// 检查type, 如果type不在1-15内就刷新,刷新之后如果出现问题就返回false；
	if (type > 15 || type < 1) {
		if (!refresh_type()) {
			return false;
		}
	}
	
	if (type == 5 || type == 11 || type == 12 || type == 15) 
		std::cerr << "构造或刷新obj时 SIA 与 V  出现在同一个团簇\n";
	
	// 检查含有sia的团簇的方向，如果没有dir==0再给dir赋值。
	if (type == 2  && ( dir == 0 || dir == -1) ) {
		dir = (int)floor(uni() * 4) + 1;
	}


	//创建obj时先检查边界, 不考虑边界条件，无条件往盒子里移动
	check_pbc(_setting);
	refresh_property(_database, _setting);

	// 修改，不许团簇中包含空位或自间隙的团簇平移至临近位点。
	if (size[0] == 0 && size[1] == 0) {
		refresh_site(_database, _setting);
	}
	// 平移之后刷新格子
	refresh_grid_pos(_setting);
	
	refresh_radius(_database, _setting);
	cal_frequency(_database, _setting);
	return true;
}

// 根据size更新type, 出现0000返回false，否则返回true
bool Object::refresh_type() {
	if (size[0] > 0 && size[1] == 0 && size[2] == 0 && size[3] == 0) type = 1;
	else if (size[0] == 0 && size[1] > 0 && size[2] == 0 && size[3] == 0) type = 2;
	else if (size[0] == 0 && size[1] == 0 && size[2] > 0 && size[3] == 0) type = 3;
	else if (size[0] == 0 && size[1] == 0 && size[2] == 0 && size[3] > 0) type = 4;
	else if (size[0] > 0 && size[1] > 0 && size[2] == 0 && size[3] == 0) type = 5;
	else if (size[0] > 0 && size[1] == 0 && size[2] > 0 && size[3] == 0) type = 6;
	else if (size[0] > 0 && size[1] == 0 && size[2] == 0 && size[3] > 0) type = 7;
	else if (size[0] == 0 && size[1] > 0 && size[2] >  0 && size[3] == 0) type = 8;
	else if (size[0] == 0 && size[1] > 0 && size[2] == 0 && size[3] > 0) type = 9;
	else if (size[0] == 0 && size[1] == 0 && size[2] > 0 && size[3] > 0) type = 10;
	else if (size[0] > 0 && size[1] > 0 && size[2] > 0 && size[3] == 0) type = 11;
	else if (size[0] > 0 && size[1] > 0 && size[2] == 0 && size[3] > 0) type = 12;
	else if (size[0] > 0 && size[1] == 0 && size[2] > 0 && size[3] > 0) type = 13;
	else if (size[0] == 0 && size[1] > 0 && size[2] > 0 && size[3] > 0) type = 14;
	else if (size[0] > 0 && size[1] > 0 && size[2] > 0 && size[3] > 0) type = 15;
	else {
		std::cerr << "根据size更新obj的type时出现问题" << ", size[0][1][2][3]分别为" <<
			size[0] <<"  " << size[1] << "  " << size[2] << "  " << size[3] << std::endl;
		system("pause");
		return false;
	}  
	return true;
}

void Object::refresh_site(const Database& _database, const Setting& _setting) {
	// 根据位置 将该团簇放到该放的位置，  如果位置是TIS 就更新 ququertar2
	if (occupying_site == 0) { // bcc
		move2bcc(_setting);
	}
	else if (occupying_site == 1) { // tis
		move2tis(_setting);
		// 更新tis的quarter_half_integer_pos信息。
		for (int i = 0; i < 3; ++i) {
			double diff = pos[i] - floor(pos[i]);
			if (diff < 0.1) {
				tis_quarter_half_int_pos[2] = i;
				continue;
			}
			else if (diff > 0.4 && diff < 0.6) {
				tis_quarter_half_int_pos[1] = i;
				continue;
			}
			else {
				tis_quarter_half_int_pos[0] = i;
				continue;
			}
		}
	}
	else if (occupying_site == 2) { // ois
		move2ois(_setting);
	}
	else { // fcc or other
		std::cerr << "fcc is not supported in this code\n";
	}
	//
}

// 先读库里的数据，找不到就找默认， F, E, occupying_site,  energy_barrier 都已更新。
void Object::refresh_property(const Database& _database, const Setting& _setting) {

//如果没读到数据， 常用参数已赋值， 
	property_array = { 6e12, 10, 6e12, 10, 10, 6e12, 10, 10, 6e12, 10, 10, 6e12, 10, 10, 10, 0, 6e12, 10, 6e12, 10};
	
	// 库 为 database.pos_map[]
	// find 返回 迭代器， 为指针类型， 需要-> 调用
	auto iter = _database.pos_map.find(size);

	switch (type) {
	case 1: //*************************************************** V---
		if (iter != _database.pos_map.cend()) { // 中标，用map里的
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else { 
			// 参数没给 用默认的   
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation, 6~trap mutation
			energy_barrier[0] = 10;
			// lzz
			energy_barrier[1] = size[0] % 16 == 1 ? 1.18 + 1.66 : 2.63 + 1.66;
			energy_barrier[2] = 10;
			energy_barrier[3] = 10;
			energy_barrier[4] = 10;
			energy_barrier[5] = 10;
			occupying_site = 0;
		}
		break;
	case 2: // -W--
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			// 参数没给 用默认的   
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			// sia 更改 * 模式的迁移指前因子
			//ljl
			//property_array.at(0) = 1.9e12 * pow(size.at(1), 0.436);
			//energy_barrier.at(2) = 9.57867-3.19804*(pow(size.at(1)-1, 0.5) - pow(size.at(1), 0.5)) + log10(size.at(1)) - log10(size.at(1)-1);
			//hpw , 不考虑SIA团发射sia
			// energy_barrier[0] = 0.04;
			//property_array.at(0) = 5e12 * pow(size.at(1), -0.5);
			//energy_barrier.at(2) = 10
			// 
			//lzz
			energy_barrier[0] = 0.04;
			property_array.at(0) = 6e12 * pow(size[1], -0.5);
			energy_barrier.at(2) = 10;
			energy_barrier.at(1) = 10;
			energy_barrier[3] = 10;
			energy_barrier[4] = 10;
			energy_barrier[5] = 10;
			occupying_site = 0;
		}
		break;
	case 3://  --Be-
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			// 参数没给 用默认的   
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			property_array.at(0) = 6e10;
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) = 10;
			energy_barrier.at(3) = 1.08 + 3.8 * pow(size[2], 0.2);
			energy_barrier.at(4) = 10;
			energy_barrier.at(5) = 10;
			occupying_site = 2;
		}
		break;
	case 4://  ---He
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			property_array.at(0) = 6e10;
			energy_barrier[0] = 0.01*size[3] + 0.02;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) = 10;
			energy_barrier.at(3) = 10;
			energy_barrier.at(4) = 2.5;
			energy_barrier.at(5) = 10;
			occupying_site = 1;
		}
		break;
	case 5://  VW--
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
		}
		else {
			;
		}
		break;
	case 6://  V-Be-
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 1.66 + size[0] > size[2] ? 0.3 * pow(size[0]-size[2], 0.5) : 4.2 * pow(size[2]/size[0] , 0.5);
			energy_barrier.at(2) = 10;
			energy_barrier.at(3) = 1.08 + size[0] >= size[2] ? 6.5 : 2.5 * pow(size[2]-size[0], 1/3);
			energy_barrier.at(4) = 10;
			energy_barrier.at(5) = 10;
			occupying_site = 0;
		}
		break;
	case 7://  V--He
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) = 10;
			energy_barrier.at(3) = 10;
			// test G. Valles energy
			energy_barrier.at(4) = size[3] > size[0] ? 5.0 * pow((double)size[3] / size[0], -0.5) : 6.0;
			//energy_barrier.at(4) = size[3] > size[0] ? 5.0 * pow((double)size[3]/size[0], -0.5) : 6.0 ;
			energy_barrier.at(5) = 10;
			occupying_site = 0;
		}
		break;
	case 8://  -WBe-
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			// 发射SIA 设置为 0，Be 与  SIA 的结合能 设置为He
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) = 0.013 + 1.2 * size[1] + 0.2 * size[2];
			energy_barrier.at(3) = 1.08 + 0.3 * size[1] + 0.01 * size[2];
			energy_barrier.at(3) = 0;
			energy_barrier.at(4) = 10;
			energy_barrier.at(5) = 10;
			occupying_site = 0;
		}
		break;
	case 9://  -W-He
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) =  0.013 + 1.2 * size[1] + 0.2 * size[3];
			energy_barrier.at(3) = 10;
			energy_barrier.at(4) = 0.04 + 0.3 * size[1] + 0.01 * size[3];
			energy_barrier.at(5) = 10;
			occupying_site = 0;
		}
		break;
	case 10://  --BeHe
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) = 10;
			energy_barrier.at(3) = 2.0 + 1.6;
			energy_barrier.at(4) = 2.0 + 0.4;
			energy_barrier.at(5) = 10;
			occupying_site = 2;
		}
		break;
	case 11://  VWBe-
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
		}
		else {
			;
		}
		break;
	case 12://  VW-He
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
		}
		else {
			;
		}
		break;
	case 13://  V-BeHe
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) = 10;
			energy_barrier.at(3) = size[2] >= size[3] ? (2.5 + 1.08 ): (1.8 + 1.08);
			energy_barrier.at(4) = size[2] <= size[3] ? (2.2 + 0.04 ): (1.8 + 0.04);
			energy_barrier.at(5) = 10;
			occupying_site = 0;
		}
		break;
	case 14://  -WBeHe
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
			parameterize();
		}
		else {
			//energy_barrier; // 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation 
			energy_barrier[0] = 10;
			energy_barrier.at(1) = 10;
			energy_barrier.at(2) = 0.013 + 1.2 * size[1] + 0.2 * size[3] + 0.2 * size[2];
			energy_barrier.at(3) = 1.08 + 0.3 * size[1] + 0.01 * size[3] + 0.01 * size[2];
			energy_barrier.at(4) = 0.04 + 0.3 * size[1] + 0.01 * size[3] + 0.01 * size[2];
			energy_barrier.at(5) = 10;
			occupying_site = 0;
		}
		break;
	case 15://  VWBeHe
		if (iter != _database.pos_map.cend()) {
			for (unsigned int i = 0; i < iter->second.size(); ++i) property_array[i] = iter->second[i];
		}
		else {
			;
		}
		break;
	}
	
	// t_m energy_barrier 赋值, 如果库里没找到 或者值为-1就按规则赋值
	/*if ((iter == _database.pos_map.cend()) || (iter != _database.pos_map.cend() && iter->second[19] == -1)  ) {
		switch (size[0]) {
		case 0:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 6 ? -4 : 10;
			break;
		case 1:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 9 ? -4 : 10;
			break;
		case 2:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 12 ? -4 : 10;
			break;
		case 3:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 23 ? -4 : 10;
			break;
		case 4:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 25 ? -4 : 10;
			break;
		case 5:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 27 ? -4 : 10;
			break;
		case 6:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 29 ? -4 : 10;
			break;
		case 7:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 30 ? -4 : 10;
			break;
		default:
			energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 5 * pow(size[0] + 1, 0.86) ? -4 : 10;
			break;
		}
	}*/

	// t_m energy_barrier 赋值, 如果库里没找到 或者值为-1就按规则赋值
	if ((iter == _database.pos_map.cend()) || (iter != _database.pos_map.cend() && iter->second[19] == -1)) {
		energy_barrier[6] = size[3] + _setting.Be_trap_mutation * size[2] >= 5 * pow(size[0] + 1, 0.86) ? -4 : 10;
	}
}

void Object::parameterize() {
	energy_barrier[0] = property_array[1];
	energy_barrier[1] = property_array[3] + property_array[4];
	energy_barrier[2] = property_array[6] + property_array[7];
	energy_barrier[3] = property_array[9] + property_array[10];
	energy_barrier[4] = property_array[12] + property_array[13];
	radius = property_array[14];
	occupying_site = int(property_array[15]);
	energy_barrier[5] = property_array[17];
	energy_barrier[6] = property_array[19];
}

/*
0	F_mig		V---	1
1	E_mig		-W--	2
2	F1_emit		--Be-	3
3	E_bind1		---He	4
4	E_mig1		VW--	5 ##
5	F2_emit		V-Be-	6
6	E_bind2		V--He	7
7	E_mig2		-WBe-	8
8	F3_emit		-W-He	9
9	E_bind3		--BeHe	10
10	E_mig3		VWBe-	11 ##
11	F4_emit		VW-He	12 ##
12	E_bind4		V-BeHe	13
13	E_mig4		-WBeHe	14
14	radius		VWBeHe	15 ##
15	site
16	F_rot
17	E_rot
*/
// radius小于等于0的团簇均会改成程序默认参数， 此处已经对a0修改
void Object::refresh_radius(const Database& _database, const Setting& _setting) {
	//if (radius <= 0) {
	//	switch (type) {
	//		// V ,lzz
	//	case 1:	radius = pow((3 * size[0] / 8.0 / PI), 1 / 3.0) + 0.34;
	//		break;
	//		// SIA, lzz
	//	case 2:radius = pow((size[1] / pow(3, 0.5)/ PI), 0.5) + 0.4813;
	//		break;
	//		// Be
	//	case 3: radius = pow((3 * size[2] / 8.0 / PI), 1 / 3.0) + 0.34;
	//		break;
	//		// He
	//	case 4:radius = pow((3 * size[3] / 8.0 / PI), 1 / 3.0) + 0.34;
	//		break;
	//		// VW--
	//	case 5: std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
	//		break;
	//		// V-Be-  's radius is the same as V---
	//	case 6:radius = pow((3 * size[0] / 8.0 / PI), 1 / 3.0) + 0.34;
	//		break;
	//		// V--He 's radius is the same as V---
	//	case 7:radius = pow((3 * size[0] / 8.0 / PI), 1 / 3.0) + 0.34;
	//		break;
	//		// -WBe- 's radius is the same as SIA
	//	case 8:radius = pow((size[1] / pow(3, 0.5) / PI), 0.5) + 0.4813;
	//		break;
	//		// -W-He radius is the same as SIA
	//	case 9:radius = pow((size[1] / pow(3, 0.5) / PI), 0.5) + 0.4813;
	//		break;
	//		// BeHe radius is the same as V---
	//	case 10:radius = pow((3 * (size[2]+size[3]) / 8.0 / PI), 1 / 3.0) + 0.34;
	//		break;
	//		// VWBe 
	//	case 11:std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
	//		break;
	//		// VW-He 
	//	case 12:std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
	//		break;
	//		// V-BeHe 's radius is the same as V's
	//	case 13:radius = pow((3 * size[0] / 8.0 / PI), 1 / 3.0) + 0.34;
	//		break;
	//		//-WBeHe 's radius is the same as SIA's
	//	case 14:radius = pow((size[1] / pow(3, 0.5) / PI), 0.5) + 0.4813;
	//		break;
	//		// VWBeHe
	//	case 15:std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
	//		break;
	//	default:std::cerr << type << "~type error,  and error in refresh_radius.\n";
	//		break;
	//	}
	//}
	if (radius <= 0) {
		switch (type) {
			// V
		case 1:
			radius = (pow((3.0 * size[0] / 8.0 / PI), 1 / 3.0) +  0.34) * _setting.a0;
			//radius = _setting.a0 * sqrt(3.0) / 4 - pow(3.0* pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0)	+ pow(3.0 * size.at(0) * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0);
			break;
			// SIA
		case 2: radius = 1.15 * _setting.a0 * sqrt(3.0) / 4 - 1.15 * pow(3.0* pow(_setting.a0, 3.0) / 8 / PI, 1.0 / 3.0) + 1.15 * pow(3.0 * size.at(1) * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0);
			break;
			// Be
		case 3: radius = 3.0+ pow(3.0 * size.at(2) * pow(_setting.a0, 3) / 40 / PI, 1.0 / 3.0) - pow(3.0* pow(_setting.a0, 3) / 40 / PI, 1.0 / 3.0);
			break;
			// He
		case 4:radius = 3.0+ pow(3.0 * size.at(3) * pow(_setting.a0, 3) / 40 / PI, 1.0 / 3.0) - pow(3.0* pow(_setting.a0, 3) / 40 / PI, 1.0 / 3.0);
			break;
			// VW--
		case 5: std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
			break;
			// V-Be-  's radius is the same as V---
		case 6:
			radius = (pow((3.0 * size[0] / 8.0 / PI), 1 / 3.0) + 0.34) * _setting.a0;
			//radius = _setting.a0 * sqrt(3.0) / 4 - pow(3.0* pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0)+ pow(3.0 * size.at(0) * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0);
			break;
			// V--He 's radius is the same as V---
		case 7:
			radius = (pow((3.0 * size[0] / 8.0 / PI), 1 / 3.0) + 0.34) * _setting.a0;
			//radius = _setting.a0 * sqrt(3.0) / 4 - pow(3.0* pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0) + pow(3.0 * size.at(0) * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0);
			break;
			// -WBe- 's radius is the same as SIA
		case 8:
			radius = 1.15 * _setting.a0 * sqrt(3.0) / 4 - 1.15 * pow(3.0* pow(_setting.a0, 3.0) / 8 / PI, 1.0 / 3.0)
				+ 1.15 * pow(3.0 * size.at(1) * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0);
			break;
			// -W-He
		case 9:
			radius = 1.15 * _setting.a0 * sqrt(3.0) / 4 - 1.15 * pow(3.0* pow(_setting.a0, 3.0) / 8 / PI, 1.0 / 3.0)
				+ 1.15 * pow(3.0 * size.at(1) * pow(_setting.a0, 3.0) / 8 / PI, 1.0 / 3.0);
			break;
			// BeHe
		case 10:
			radius = 3.0+ pow(3.0 * ((double)size.at(3) + size[2]) * pow(_setting.a0, 3.0) / 40 / PI, 1.0 / 3.0) - pow(3.0* pow(_setting.a0, 3.0) / 40 / PI, 1.0 / 3.0);
			break;
			// VWBe
		case 11:
			std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
			break;
			// VW-He
		case 12:
			std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
			break;
			// V-BeHe 's radius is the same as V's
		case 13:
			radius = (pow((3.0 * size[0] / 8.0 / PI), 1 / 3.0) + 0.34) * _setting.a0;
			// radius = _setting.a0 * sqrt(3.0) / 4 - pow(3.0 * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0)+ pow(3.0 * size.at(0) * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0);
			break;
			//-WBeHe 's radius is the same as SIA's
		case 14:
			radius = 1.15 * _setting.a0 * sqrt(3.0) / 4 - 1.15 * pow(3.0* pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0)
				+ 1.15 * pow(3.0 * size.at(1) * pow(_setting.a0, 3) / 8 / PI, 1.0 / 3.0);
			break;
			// VWBeHe
		case 15:
			std::cerr << "SIA and V occupy in the same obj, and error in refresh_radius.\n";
			break;
		default:
			std::cerr << type << "~type error,  and error in refresh_radius.\n";
			break;
		}
	}
	radius = radius / _setting.a0;
}

void Object::move2bcc(const Setting& _setting) {

	// 将pos移动到原点附近的暂时数组
	std::array<double, 3> displace = { double(floor(pos[0])), double(floor(pos[1])), double(floor(pos[2])) };

	// 移动之后obj的位置
	pos[0] -= displace[0]; pos[1] -= displace[1]; pos[2] -= displace[2];

	std::array<double, 9> dist;
	for (int i = 0; i < 9; ++i) {
		dist[i] = cal_distance(pos, _setting.bcc_site[i]);
	}
	// 计算出距离数组， 找到最小i
	int min_pos = 0;
	double min_dist = dist[0];

	for (int i = 0; i < 9; ++i) {
		if (min_dist > dist[i]) {
			min_pos = i;
			min_dist = dist[i];
		}
	}
	// 把找到距离最小的bcc移动回去
	for (int i = 0; i < 3; ++i) {
		pos[i] = _setting.bcc_site.at(min_pos).at(i) + displace[i];
	}
}

void Object::move2tis(const Setting& _setting) {
	// 将pos移动到原点附近的暂时数组
	std::array<double, 3> displace = { double(floor(pos[0])), double(floor(pos[1])) ,  double(floor(pos[2])) };

	// 移动之后obj的位置
	pos[0] -= displace[0]; pos[1] -= displace[1]; pos[2] -= displace[2];

	std::array<double, 24> dist;
	for (int i = 0; i < 24; ++i) {
		dist[i] = cal_distance(pos, _setting.tis_site[i]);
	}
	// 计算出距离数组， 找到最小i
	int min_pos = 0;
	double min_dist = dist[0];
	for (int i = 0; i < 24; ++i) {
		if (min_dist > dist[i]) {
			min_pos = i;
			min_dist = dist[i];
		}
	}
	// 把找到距离最小的tis移动回去
	for (int i = 0; i < 3; ++i) {
		pos[i] = _setting.tis_site.at(min_pos).at(i)+ displace[i];
	}
}

void Object::move2ois(const Setting& _setting) {
	// 将pos移动到原点附近的暂时数组
	std::array<double, 3> displace = { double(floor(pos[0])), double(floor(pos[1])) ,  double(floor(pos[2])) };

	// 移动之后obj的位置
	pos[0] -= displace[0]; pos[1] -= displace[1]; pos[2] -= displace[2];

	std::array<double, 6> dist;
	for (int i = 0; i < 6; ++i) {
		dist[i] = cal_distance(pos, _setting.ois_site[i]);
	}
	// 计算出距离数组， 找到最小i
	int min_pos = 0;
	double min_dist = dist[0];
	for (int i = 0; i < 6; ++i) {
		if (min_dist > dist[i]) {
			min_pos = i;
			min_dist = dist[i];
		}
	}
	// 把找到距离最小的ois移动回去
	for (int i = 0; i < 3; ++i) {
		pos[i] = _setting.ois_site.at(min_pos).at(i)+displace[i];
	}
}

	/*
	0	F_mig		V---	1
	1	E_mig		-W--	2
	2	F1_emit		--Be-	3
	3	E_bind1		---He	4
	4	E_mig1 		VW--	5 ##
	5	F2_emit		V-Be-	6
	6	E_bind2		V--He	7
	7	E_mig2		-WBe-	8
	8	F3_emit		-W-He	9
	9	E_bind3		--BeHe	10
	10	E_mig3		VWBe-	11 ##
	11	F4_emit		VW-He	12 ##
	12	E_bind4		V-BeHe	13
	13	E_mig4		-WBeHe	14
	14	radius		VWBeHe	15 ##
	15	site
	16	F_rot
	17	E_rot
	*/
void Object::cal_frequency(const Database& _database, const Setting& _set) {
	// 0~mig, 1~emit_1, 2~emit_2, 3~emit_3, 4~emit_4, 5~rotation, 6~tm
	if (energy_barrier.at(0) < _set.max_energy) {
		frequency[0] = property_array[0] * exp(-1 * energy_barrier[0] / _set.kb / _set.temperature);
	}
	if (energy_barrier.at(1) < _set.max_energy) {
		frequency[1] = property_array[2] * exp(-1 * energy_barrier[1] / _set.kb / _set.temperature);
	}
	if (energy_barrier.at(2) < _set.max_energy) {
		frequency[2] = property_array[5] * exp(-1 * energy_barrier[2] / _set.kb / _set.temperature);
	}
	if (energy_barrier.at(3) < _set.max_energy) {
		frequency[3] = property_array[8] * exp(-1 * energy_barrier[3] / _set.kb / _set.temperature);
	}
	if (energy_barrier.at(4) < _set.max_energy) {
		frequency[4] = property_array[11] * exp(-1 * energy_barrier[4] / _set.kb / _set.temperature);
	}
	if (energy_barrier.at(5) < _set.max_energy) {
		frequency[5] = property_array[16] * exp(-1 * energy_barrier[5] / _set.kb / _set.temperature);
	}	
	if (energy_barrier.at(6) < _set.max_energy) {
		frequency[6] = property_array[18] * exp(-1 * energy_barrier[6] / _set.kb / _set.temperature);
	}
}


// 普通函数
int size2type(std::array<int, 4> size) {
	int type = -1;
	if (size[0] > 0 && size[1] == 0 && size[2] == 0 && size[3] == 0) type = 1;
	else if (size[0] == 0 && size[1] > 0 && size[2] == 0 && size[3] == 0) type = 2;
	else if (size[0] == 0 && size[1] == 0 && size[2] > 0 && size[3] == 0) type = 3;
	else if (size[0] == 0 && size[1] == 0 && size[2] == 0 && size[3] > 0) type = 4;
	else if (size[0] > 0 && size[1] > 0 && size[2] == 0 && size[3] == 0) type = 5;
	else if (size[0] > 0 && size[1] == 0 && size[2] > 0 && size[3] == 0) type = 6;
	else if (size[0] > 0 && size[1] == 0 && size[2] == 0 && size[3] > 0) type = 7;
	else if (size[0] == 0 && size[1] > 0 && size[2] > 0 && size[3] == 0) type = 8;
	else if (size[0] == 0 && size[1] > 0 && size[2] == 0 && size[3] > 0) type = 9;
	else if (size[0] == 0 && size[1] == 0 && size[2] > 0 && size[3] > 0) type = 10;
	else if (size[0] > 0 && size[1] > 0 && size[2] > 0 && size[3] == 0) type = 11;
	else if (size[0] > 0 && size[1] > 0 && size[2] == 0 && size[3] > 0) type = 12;
	else if (size[0] > 0 && size[1] == 0 && size[2] > 0 && size[3] > 0) type = 13;
	else if (size[0] == 0 && size[1] > 0 && size[2] > 0 && size[3] > 0) type = 14;
	else if (size[0] > 0 && size[1] > 0 && size[2] > 0 && size[3] > 0) type = 15;
	else {
		std::cerr << "根据size获取type时出现问题" << ", size[0][1][2][3]分别为" <<
			size[0] << "  " << size[1] << "  " << size[2] << "  " << size[3] << std::endl;
		system("pause");
	} 
	return type;
}


double cal_distance(std::array<double, 3> a, std::array<double, 3> b) {
	return pow(pow(a[0] - b[0], 2) + pow(a[1] - b[1], 2) + pow(a[2] - b[2], 2), 0.5);
}


// 读inicas.txt，每一行生成一个object（动态分配内存），将指向它的指针存在obj_ptr_list中
void read_cascade(std::string _filename, std::vector<Object*>& _obj_ptr_list, const Database& _database,  Setting& _set)
{
	std::vector<std::string> total_file;
	if (_set.cas_map.find(_filename) == _set.cas_map.end()) {
		std::ifstream inFile(_filename);
		if (inFile.is_open())
		{
			std::string str;
			while (std::getline(inFile, str))
			{
				if (!str.empty() && str.at(0) != '#') //skip the empty line and the line started with '#'
				{
					total_file.push_back(str);
				}
			}
		}
		inFile.close();
	}
	else {
		for (auto temp_s : _set.cas_map.at(_filename)) {
			total_file.emplace_back(temp_s);
		}
	}
	//预先产生一个平移位移，方便后续插入cascade的时候随机平移
	double dx = uni() * _set.box_length.at(0);
	double dy = uni() * _set.box_length.at(1);
	double dz = uni() * _set.box_length.at(2);
	//预先产生三个角度，方便后续插入cascade的时候随机旋转
	double alpha = uni() * (2 * 3.14159265359);
	double beta = uni() * (2 * 3.14159265359);
	double gamma = uni() * (2 * 3.14159265359);
	int num = int(uni() * 3);
	std::array<std::array<double, 3>, 3> rx = { 1.0, 0.0, 0.0, 0.0 };
	//预先产生三个旋转矩阵	
	std::array<std::array<double, 3>, 3> Rx = { 1.0, 0.0, 0.0, 0.0, cos(alpha), -sin(alpha), 0.0, sin(alpha), cos(alpha) }; //绕x轴旋转alpha
	std::array<std::array<double, 3>, 3> Ry = { cos(beta), 0.0, sin(beta), 0.0, 1.0, 0.0, -sin(beta), 0.0, cos(beta) };     //绕y轴旋转beta
	std::array<std::array<double, 3>, 3> Rz = { cos(gamma), -sin(gamma), 0.0, sin(gamma), cos(gamma), 0.0, 0.0, 0.0, 1.0 }; //绕z轴旋转gamma

	for(std::string str : total_file){
		std::istringstream ss(str);
		int temp, type, size0, size1, size2,size3, dir;
		double x, y, z;
		ss >> temp >> type >> x >> y >> z >> size0 >> size1 >> size2 >> size3 >> dir;

		// 周期性边界条件
		if (_set.boundary_condition == 0 || _set.boundary_condition == 3) {
			if (_set.step > 0 && _set.rand_translate_cascade)
			{
				x += dx;
				y += dy;
				z += dz;
			}
			if (_set.step > 0 && _set.rand_rotate_cascade > 0)
			{
				switch (num)
				{
				case 0:
					multiply(Rx, x, y, z);
					multiply(Ry, x, y, z);
					multiply(Rz, x, y, z);
					break;
				case 1:
					multiply(Ry, x, y, z);
					multiply(Rz, x, y, z);
					multiply(Rx, x, y, z);
					break;
				case 2:
					multiply(Rz, x, y, z);
					multiply(Rx, x, y, z);
					multiply(Ry, x, y, z);
					break;
				default:
					std::cout << "There is a problem in read_cascade function\n";
					break;
				}
				if (type == 2) {
					dir = (int)(uni() * 4) + 1;
				}
			}
		}
		else if (_set.boundary_condition == 1) {
			// zfree计算的是表面， xy可以平移，不能旋转， z轴可以旋转不能平移。
			if (_set.step > 0 && _set.rand_translate_cascade)
			{
				x += dx;
				y += dy;
			}
			if (_set.step > 0 && _set.rand_rotate_cascade > 0)
			{
				multiply(Rz, x, y, z);
				if (type == 2) {
					dir = (int)(uni() * 4) + 1;
				}
			}
		}
		Object* ptr_obj = new Object(type, x, y, z, size0, size1,size2, size3, dir, _database, _set);
		_obj_ptr_list.push_back(ptr_obj);
	}
	//if (_set.cascade_insert_time > 1e-7) {
	//	std::cout << "The " << _filename << " is successfully read.\n";
	//}
}

//旋转矩阵左乘空间坐标
void multiply(std::array<std::array<double, 3>, 3>& R, double& x, double& y, double& z)
{
	double x_new = R[0][0] * x + R[0][1] * y + R[0][2] * z;
	double y_new = R[1][0] * x + R[1][1] * y + R[1][2] * z;
	double z_new = R[2][0] * x + R[2][1] * y + R[2][2] * z;

	x = x_new;
	y = y_new;
	z = z_new;
}