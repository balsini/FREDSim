/*
  In this example, a simple system is simulated, consisting of two
  real-time tasks scheduled by EDF on a single processor.
*/
#include <kernel.hpp>
#include <task.hpp>
#include <fpsched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <ps_trace.hpp>
#include <acceleratedtask.hpp>
#include <instr.hpp>
#include <fcfsresmanager.hpp>
#include <resource.hpp>
#include <basestat.hpp>
#include <fpgakernel.hpp>

#include <iostream>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <boost/filesystem.hpp>
#include <unistd.h>
#include <sys/wait.h>

#include <randomvar.hpp>

#include "generator.h"
#include "constant.h"
#include "tinyxml/tinyxml2.h"

#include <vector>

using namespace MetaSim;
using namespace RTSim;
using namespace tinyxml2;

//#define RUN_DURATION    (500 * 1000)
#define RUN_PERIOD_TIMES 1000

const string dirRootName = "results/";


void XMLErrorQuit(const string &err_msg)
{
    cerr << err_msg << endl;
    exit(1);
}


overallArchitecture_t parseArchitectureXML(const string &path)
{
    overallArchitecture_t arch;

    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(path.c_str());

    if (eResult != XML_SUCCESS)
        XMLErrorQuit(to_string(eResult));

    XMLElement *pElement, *pElement2, *pElement3;
    XMLNode *pRoot = xmlDoc.FirstChild();

    if (pRoot == nullptr)
        XMLErrorQuit("XML Error, no root");

    /*
  if (string("FPGA").compare(pRoot->ToElement()->Attribute("type")) != 0)
    XMLErrorQuit("Wrong simulation type");
  arch.name = pRoot->ToElement()->Attribute("name");
*/

    if (pRoot->ToElement()->Attribute("proc") == 0)
        arch.processes = 1;
    else
        arch.processes = stol(pRoot->ToElement()->Attribute("proc"));

    if (pRoot->ToElement()->Attribute("runs") == 0)
        arch.runs = 1;
    else
        arch.runs = stol(pRoot->ToElement()->Attribute("runs"));

    ///////////
    // TASKS //
    ///////////

    pElement = pRoot->FirstChildElement("tasks");
    if (pElement == nullptr)
        XMLErrorQuit("Wrong \"tasks\" field");

    pElement2 = pElement->FirstChildElement("max_K");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"max_K\" field");
    arch.TASK_MAX_K = stod(pElement2->GetText());

    pElement2 = pElement->FirstChildElement("period");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"period\" field");

    pElement3 = pElement2->FirstChildElement("min");
    if (pElement3 == nullptr)
        XMLErrorQuit("Wrong \"period/min\" field");
    arch.PERIOD_MIN = stoi(pElement3->GetText());

    pElement3 = pElement2->FirstChildElement("max");
    if (pElement3 == nullptr)
        XMLErrorQuit("Wrong \"period/max\" field");
    arch.PERIOD_MAX = stoi(pElement3->GetText());

    pElement2 = pElement->FirstChildElement("U_SW");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"U_SW\" field");
    if (pElement2->Attribute("test") && string("true").compare(pElement2->Attribute("test")) == 0) {
        double min, max, step;

        pElement3 = pElement2->FirstChildElement("min");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/min\" field");
        min = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("max");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/max\" field");
        max = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("step");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/step\" field");
        step = stod(pElement3->GetText());

        for (double s = min; s <= max; s += step) {
            arch.U_SW_list.push_back(s);
        }
    } else {
        arch.U_SW_list.push_back(stod(pElement2->GetText()));
    }

    pElement2 = pElement->FirstChildElement("U_HW");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"U_HW\" field");
    if (pElement2->Attribute("test") && string("true").compare(pElement2->Attribute("test")) == 0) {
        double min, max, step;

        pElement3 = pElement2->FirstChildElement("min");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/min\" field");
        min = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("max");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/max\" field");
        max = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("step");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/step\" field");
        step = stod(pElement3->GetText());

        for (double s = min; s <= max; s += step) {
            arch.U_HW_list.push_back(s);
        }
    } else {
        arch.U_HW_list.push_back(stod(pElement2->GetText()));
    }

    pElement2 = pElement->FirstChildElement("U_HW_UB");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"U_HW_UB\" field");
    arch.U_HW_UB = stod(pElement2->GetText());

    pElement2 = pElement->FirstChildElement("C_SW_MIN");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"C_SW_MIN\" field");
    arch.C_SW_MIN = stod(pElement2->GetText());

    //////////
    // FPGA //
    //////////

    pElement = pRoot->FirstChildElement("FPGA");
    if (pElement == nullptr)
        XMLErrorQuit("Wrong \"FPGA\" field");

    pElement2 = pElement->FirstChildElement("area");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"area\" field");
    arch.A_TOT = stol(pElement2->GetText());

    pElement2 = pElement->FirstChildElement("speedup");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"rho\" field");
    if (pElement2->Attribute("test") && string("true").compare(pElement2->Attribute("test")) == 0) {
        double min, max, step;

        pElement3 = pElement2->FirstChildElement("min");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"rho/min\" field");
        min = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("max");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"rho/max\" field");
        max = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("step");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"rho/step\" field");
        step = stod(pElement3->GetText());

        for (double s = min; s <= max; s += step) {
            arch.RHO_list.push_back(s);
        }
    } else {
        arch.RHO_list.push_back(stod(pElement2->GetText()));
    }




    pElement2 = pElement->FirstChildElement("partitions");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"partitions\" field");
    arch.PARTITION_NUM = stod(pElement2->GetText());

    pElement2 = pElement->FirstChildElement("slots");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"slots\" field");
    pElement3 = pElement2->FirstChildElement("min");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"min\" field");
    arch.SLOT_NUM_MIN = stol(pElement3->GetText());
    pElement3 = pElement2->FirstChildElement("max");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"max\" field");
    arch.SLOT_NUM_MAX = stol(pElement3->GetText());

    pElement2 = pElement->FirstChildElement("speedup");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"speedup\" field");
    if (pElement2->Attribute("test") && string("true").compare(pElement2->Attribute("test")) == 0) {
        double min, max, step;

        pElement3 = pElement2->FirstChildElement("min");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/min\" field");
        min = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("max");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/max\" field");
        max = stod(pElement3->GetText());

        pElement3 = pElement2->FirstChildElement("step");
        if (pElement3 == nullptr)
            XMLErrorQuit("Wrong \"speedup/step\" field");
        step = stod(pElement3->GetText());

        for (double s = min; s <= max; s += step) {
            arch.SPEEDUP_list.push_back(s);
        }
    } else {
        arch.SPEEDUP_list.push_back(stod(pElement2->GetText()));
    }


    pElement2 = pElement->FirstChildElement("FRI");
    if (pElement2 == nullptr)
        XMLErrorQuit("Wrong \"FRI\" field");
    pElement3 = pElement2->FirstChildElement("val");
    while (pElement3 != nullptr) {
        string fs(pElement3->GetText());

        if (!fs.compare("TB_PREEMPTIVE"))
            arch.FRI_list.push_back(TB_PREEMPTIVE);
        if (!fs.compare("TB_NONPREEMPTIVE"))
            arch.FRI_list.push_back(TB_NONPREEMPTIVE);
        if (!fs.compare("FP_PREEMPTIVE"))
            arch.FRI_list.push_back(FP_PREEMPTIVE);
        if (!fs.compare("FP_NONPREEMPTIVE"))
            arch.FRI_list.push_back(FP_NONPREEMPTIVE);

        pElement3 = pElement3->NextSiblingElement("val");
    }

    return arch;
}


int main(int argc, char * argv[])
{
    try {

        if (argc != 2) {
            cout << "Please, provide the simulation file" << endl;
            return -1;
        }

        // Initialize timer and folder
        time_t rawtime;
        struct tm * timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);


        ///////////////////

        // Create experiment folder

        string curDir = dirRootName + asctime(timeinfo);
        curDir.at(curDir.length() - 1) = '/';
        replace(curDir.begin(), curDir.end(), ' ', '_');
        replace(curDir.begin(), curDir.end(), ':', '_');
        boost::filesystem::create_directories(curDir.c_str());

        //////////////////


        RandomGen randVar(time(NULL));

        overallArchitecture_t arch = parseArchitectureXML(argv[1]);

        Environment * e = new Environment(&randVar);

        pid_t child, changed_child;
        vector<pid_t> children;
        int status;

        unsigned int experiment_number = arch.FRI_list.size() *
                arch.SPEEDUP_list.size() *
                arch.U_SW_list.size() *
                arch.U_HW_list.size() *
                arch.RHO_list.size() *
                arch.runs;

        runStarted(curDir, experiment_number);

        for (unsigned int i=0; i<arch.FRI_list.size(); ++i) {

            arch.FRI = arch.FRI_list.at(i);

            string friDir = curDir + "FRI_" + to_string(arch.FRI) + "/";
            boost::filesystem::create_directories(friDir);

            writeConfigurationToFile(friDir, arch);

            for (unsigned int s=0; s<arch.SPEEDUP_list.size(); ++s) {

                arch.SPEEDUP = arch.SPEEDUP_list.at(s);

                string speedupDir = friDir + "SPEEDUP_" + to_string(arch.SPEEDUP) + "/";
                boost::filesystem::create_directories(speedupDir);

                for (unsigned int sw=0; sw<arch.U_SW_list.size(); ++sw) {

                    arch.U_SW = arch.U_SW_list.at(sw);

                    string u_SW_Dir = speedupDir + "U_SW_" + to_string(arch.U_SW) + "/";
                    boost::filesystem::create_directories(u_SW_Dir);

                    for (unsigned int j=0; j<arch.U_HW_list.size(); ++j) {

                        arch.U_HW = arch.U_HW_list.at(j);

                        string valDir = u_SW_Dir + "U_HW_" + to_string(arch.U_HW);
                        boost::filesystem::create_directories(valDir);


                        for (unsigned int r=0; r<arch.RHO_list.size(); ++r) {

                            arch.RHO = arch.RHO_list.at(r);

                            string rhoDir = valDir + "RHO_" + to_string(arch.RHO);
                            boost::filesystem::create_directories(rhoDir);

                            for (unsigned int i=0; i<arch.runs; ++i) {

                                string runDir = rhoDir + "/" + to_string(i) + "/";
                                boost::filesystem::create_directories(runDir);

                                Environment_details_t ed = generateEnvironment(arch, &randVar);
                                e->build(ed);
                                e->environmentToFile(runDir);

                                if ((child = fork()) == 0) {
                                    // Child process

#ifdef RUN_DURATION
                                    SIMUL.run(RUN_DURATION);
#endif
#ifdef RUN_PERIOD_TIMES
                                    SIMUL.run(max_T(ed) * RUN_PERIOD_TIMES);
#endif

                                    e->resultsToFile(runDir);

                                    delete e;
                                    return 0;
                                } else {
                                    children.push_back(child);

                                    while (children.size() >= arch.processes)
                                    {
                                        changed_child = wait(&status);
                                        if (changed_child > 0)
                                        {
                                            children.erase(find(children.begin(),
                                                                children.end(),
                                                                changed_child));

                                            runCompleted(curDir);
                                        } else {
                                            cout << "wait returned " << changed_child << endl;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        while (children.size() > 0)
        {
            changed_child = wait(&status);
            if (changed_child > 0)
            {
                children.erase(find(children.begin(),
                                    children.end(),
                                    changed_child));
                runCompleted(curDir);
            } else {
                cout << "wait returned " << changed_child << endl;
            }
        }

        delete e;

        //for (int N_task=2; i<50; i++) {

        //for (int N_SLOT=0.2; i<0.5; i++) {
        //  updateStruttura(&strutturaParametriTaskEArchitettura);

        // Crea la cartella

        //  for (SIM_RUNS) {
        //    generateEnvironment(strutturaParametriTaskEArchitettura);
        //SIMUL.run(50);
        //   getResults();
        // }
        //}

    } catch (BaseExc &e) {
        cout << e.what() << endl;
    }
}
