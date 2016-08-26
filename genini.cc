// vim: set makeprg=ninja
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <set>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <dirent.h>
#include <fnmatch.h>


static int findInPath(const char *path, const char *pattern)
{
    int found =0;
    DIR *dirp=opendir(path);
    struct dirent entry;
    struct dirent *dp=&entry;
    while((dp = readdir(dirp)))
    {
        if((fnmatch(pattern, dp->d_name,0)) == 0)
        {
            found++;
        }
    }
    closedir(dirp);
    return found;
}

typedef std::map <std::string, std::string> conf_t;

int main(int argc, char * argv[]) {
    if (argc  <3) {
        std::cerr << "need two args" <<std::endl;
        return -1;
    }
    std::set<std::string> existing_agents;
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
    conf_t defaultConf;
    BOOST_FOREACH ( boost::property_tree::ptree::value_type & conf, commonConfig) {
        defaultConf[conf.first] = conf.second.data();
    }

    boost::property_tree::ptree rules= pt.get_child("Rules");
    BOOST_FOREACH ( boost::property_tree::ptree::value_type & vt, rules ) {
        std::string agentName = vt.second.get<std::string>("AgName");
        if (0 != existing_agents.count(agentName ))
            continue;
        boost::property_tree::ptree conds= vt.second.get_child("Conds");
        bool result =true;
        BOOST_FOREACH ( boost::property_tree::ptree::value_type & cond, conds) {
            std::string type = cond.second.get<std::string>("Type");
            if (0 == type.compare("file")) {
                std::string file= cond.second.get<std::string>("File");
                if ( !boost::filesystem::exists( file )  )
                {
                       result = false;
                       break;
                }
            }
            if (0 == type.compare("fnmatch")) {
                std::string path= cond.second.get<std::string>("Path");
                std::string pattern= cond.second.get<std::string>("Pattern");
                if ( 0 == findInPath(path.c_str(), pattern.c_str()))
                {
                       result = false;
                       break;
                }
            }
        }
        if (result) {
            existing_agents.insert(agentName);
            std::string conf= vt.second.get<std::string>("Conf");
            conf_t localConf = defaultConf;
            BOOST_FOREACH ( boost::property_tree::ptree::value_type & conf, commonConfig) {
                localConf[conf.first] = conf.second.data();
            }
            agentsSection<<  "[" << agent_id  << "-" << agentName << "]" <<  std::endl;
            BOOST_FOREACH ( conf_t::value_type val, localConf) {
                agentsSection<<  "\t" << val.first << "="<< val.second <<  std::endl;
            }
            agentsSection<<  std::endl;
            processSection<< "\t" << agentName << "=agent -i " << agent_id << " -s " << agentName << std::endl;
            agent_id++;
        }

    }
    outputfile << processSection.str()<< std::endl << agentsSection.str()<< std::endl;
    return 0;
}
