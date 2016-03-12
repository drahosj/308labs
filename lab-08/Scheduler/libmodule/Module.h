/*
 * module.hpp
 *
 *  Created on: Mar 8, 2016
 *      Author: vens
 */

#ifndef MODULE_HPP_
#define MODULE_HPP_

#include <string>

namespace module {

class Module {
public:
	Module(const std::string& name);
	virtual ~Module();

	void Remove();

private:
	std::string module_name;
	void * module_handle;
};

} /* namespace module */

#endif /* MODULE_HPP_ */
