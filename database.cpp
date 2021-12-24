#include "database.h"

// #type	size	size	size	size	
// F_mig	E_mig	F1	E_bind1	E_mig1	F2	E_bind2	E_mig2	F3	E_bind3	E_mig3	F4	E_bind4	E_mig4	radius	site	F_rot	E_rot	Ftm	Etm
Database::Database(std::string  _posb) {
	std::fstream inFile(_posb);
	if (inFile.good()) {
		std::string str;
		while (std::getline(inFile, str)) {
			if (!str.empty() && str.at(0) != '#') {
				std::array<int, 4> _size;
				std::array<double, 20> _property;
				std::istringstream ss(str);
				int temp = 0;
				ss >> temp;
				ss >> _size[0] >> _size[1] >> _size[2] >> _size[3] >> _property[0] >> _property[1] >> _property[2] >>
					_property[3] >> _property[4] >> _property[5] >> _property[6] >> _property[7] >> _property[8] >>
					_property[9] >> _property[10] >> _property[11] >> _property[12] >> _property[13] >> _property[14] >>
					_property[15] >>_property[16] >> _property[17] >> _property[18] >> _property[19];
				pos_map.insert(std::make_pair(_size, _property));
			}
		}
		std::cout << _posb << " is successfully read" << std::endl;
	}
	else {
		std::cerr << _posb << " is failed to open\n";
	}
}








