// vim: set makeprg=ninja
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

int main(int argc, char * argv[]) {

    if (argc > 3)
        std::cerr << "need two args" <<std::endl;
    int agent_id=1;
    const char * rulefile =argv[1];
    const char * output =argv[2];
    std::stringstream processSection;
    std::stringstream agentsSection;
    processSection << "[Agents]" << std::endl;

    boost::property_tree::ptree pt;
    read_json(rulefile, pt);
    std::ofstream outputfile (output);
    boost::property_tree::ptree commonConfig= pt.get_child("Conf");
    boost::property_tree::ptree rules= pt.get_child("Rules");
    BOOST_FOREACH ( boost::property_tree::ptree::value_type & vt, rules ) {
        std::string agentName = vt.second.get<std::string>("AgentName");
        boost::property_tree::ptree conds= pt.get_child("Cond");
        bool result =true;
        BOOST_FOREACH ( boost::property_tree::ptree::value_type & cond, conds) {
            std::string type = cond.second.get<std::string>("Type");
            if (type.compare("file")) {
                std::string file= cond.second.get<std::string>("File");
                if ( !boost::filesystem::exists( file )  )
                {
                       result = false;
                       break;
                }
            }
        }
        if (result) {
            std::string conf= vt.second.get<std::string>("Conf");
            agentsSection<<  "[" << agent_id  << "-" << agentName << "]" <<  std::endl;
            agentsSection<<  "\t" << conf << std::endl<< std::endl;
            processSection<< "\t" << agentName << "=agent -i " << agent_id << " -s " << agentName << std::endl;
        }

    }
    outputfile << processSection.str()<< std::endl << agentsSection.str()<< std::endl;
    return 0;
}
