/* Copyright 2011 Cambridge Hydronautics Ltd.
 *
 * Cambridge Hydronautics Ltd. licenses this software to the CAUV student
 * society for all purposes other than publication of this source code.
 * 
 * See license.txt for details.
 * 
 * Please direct queries to the officers of Cambridge Hydronautics:
 *     James Crosby    james@camhydro.co.uk
 *     Andy Pritchard   andy@camhydro.co.uk
 *     Leszek Swirski leszek@camhydro.co.uk
 *     Hugo Vincent     hugo@camhydro.co.uk
 */

#ifndef __CAUV_GLOBAL_H__
#define __CAUV_GLOBAL_H__

#include <string>

namespace cauv{

class cauv_global
{
	public:
		static cauv_global& current();

        static void print_logo(
            const char* colour_start,
            const char* colour_end,
            const char* shape_start,
            const char* shape_end,
            const std::string& module_name
        );
		static void print_module_header(const std::string& module_name);
	
    private:
		static cauv_global* m_current;
		cauv_global();

	protected:
		
};

} // namespace cauv

#endif//__CAUV_GLOBAL_H__
