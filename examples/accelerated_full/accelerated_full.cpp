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

#include "builder.h"
#include "generator.h"
#include "tinyxml/tinyxml2.h"

#include <vector>





#include "generator.h"
#include "rta_ss.h"
#include "delay_bound.h"




const unsigned int UNIT = 1000;




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


  //////////
  // FPGA //
  //////////

  pElement = pRoot->FirstChildElement("FPGA");
  if (pElement == nullptr)
    XMLErrorQuit("Wrong \"FPGA\" field");




  pElement2 = pElement->FirstChildElement("area");
  if (pElement2 == nullptr)
    XMLErrorQuit("Wrong \"area\" field");
  if (pElement2->Attribute("test") && string("true").compare(pElement2->Attribute("test")) == 0) {
    double min, max, step;

    pElement3 = pElement2->FirstChildElement("min");
    if (pElement3 == nullptr)
      XMLErrorQuit("Wrong \"area/min\" field");
    min = stod(pElement3->GetText());

    pElement3 = pElement2->FirstChildElement("max");
    if (pElement3 == nullptr)
      XMLErrorQuit("Wrong \"area/max\" field");
    max = stod(pElement3->GetText());

    pElement3 = pElement2->FirstChildElement("step");
    if (pElement3 == nullptr)
      XMLErrorQuit("Wrong \"area/step\" field");
    step = stod(pElement3->GetText());

    for (double s = min; s <= max; s += step) {
      arch.A_TOT_list.push_back(s);
    }
  } else {
    arch.A_TOT_list.push_back(stod(pElement2->GetText()));
  }



  pElement2 = pElement->FirstChildElement("rho");
  if (pElement2 == nullptr)
    XMLErrorQuit("Wrong \"rho\" field");
  arch.RHO = stod(pElement2->GetText());

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
      arch.FRI_list.push_back(TB_PREEMPTIVE_B);
    if (!fs.compare("TB_NONPREEMPTIVE"))
      arch.FRI_list.push_back(TB_NONPREEMPTIVE_B);
    if (!fs.compare("FP_PREEMPTIVE"))
      arch.FRI_list.push_back(FP_PREEMPTIVE_B);
    if (!fs.compare("FP_NONPREEMPTIVE"))
      arch.FRI_list.push_back(FP_NONPREEMPTIVE_B);
    if (!fs.compare("FPGA_STATIC"))
      arch.FRI_list.push_back(FPGA_STATIC_B);

    pElement3 = pElement3->NextSiblingElement("val");
  }

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


  pElement2 = pElement->FirstChildElement("taskAppended");
  if (pElement2 == nullptr)
    XMLErrorQuit("Wrong \"taskAppended\" field");
  if (pElement2->Attribute("test") && string("true").compare(pElement2->Attribute("test")) == 0) {
    double min, max, step;

    pElement3 = pElement2->FirstChildElement("min");
    if (pElement3 == nullptr)
      XMLErrorQuit("Wrong \"taskAppended/min\" field");
    min = stod(pElement3->GetText());

    pElement3 = pElement2->FirstChildElement("max");
    if (pElement3 == nullptr)
      XMLErrorQuit("Wrong \"taskAppended/max\" field");
    max = stod(pElement3->GetText());

    pElement3 = pElement2->FirstChildElement("step");
    if (pElement3 == nullptr)
      XMLErrorQuit("Wrong \"taskAppended/step\" field");
    step = stod(pElement3->GetText());

    for (double s = min; s <= max; s += step) {
      arch.TASK_APPENDED_list.push_back(s);
    }
  } else {
    arch.TASK_APPENDED_list.push_back(stod(pElement2->GetText()));
  }


  pElement2 = pElement->FirstChildElement("U_HW_UB");
  if (pElement2 == nullptr)
    XMLErrorQuit("Wrong \"U_HW_UB\" field");
  arch.U_HW_UB = stod(pElement2->GetText());

  pElement2 = pElement->FirstChildElement("C_SW_MIN");
  if (pElement2 == nullptr)
    XMLErrorQuit("Wrong \"C_SW_MIN\" field");
  arch.C_SW_MIN = stod(pElement2->GetText());

  if (arch.PARTITION_NUM > 1) {

    pElement2 = pElement->FirstChildElement("periodBreak");
    if (pElement2 == nullptr)
      XMLErrorQuit("Wrong \"periodBreak\" field");
    pElement3 = pElement2->FirstChildElement("val");
    while (pElement3 != nullptr) {
      unsigned int periodBR;

      pElement3->QueryUnsignedText(&periodBR);
      arch.PERIOD_break_list.push_back(periodBR);

      pElement3 = pElement3->NextSiblingElement("val");
    }
  }

  pElement2 = pElement->FirstChildElement("periodBucket");
  if (pElement2 == nullptr)
    XMLErrorQuit("Wrong \"periodBucket\" field");
  pElement3 = pElement2->FirstChildElement("val");
  while (pElement3 != nullptr) {
    unsigned int periodBR;

    pElement3->QueryUnsignedText(&periodBR);

    arch.PERIOD_bucket.push_back(periodBR);

    pElement3 = pElement3->NextSiblingElement("val");
  }


  return arch;
}



void analysis();
int simulation(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  try {

    srand(time(NULL));

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


    //////////////////
    // Copy configuration to file
    //////////////////

    ofstream out_file;
    ifstream in_file;
    in_file.open(argv[1]);

    if (in_file.is_open()) {
      out_file.open(curDir + "info.xml");
      if (out_file.is_open()) {
        string line;
        while (getline(in_file, line)) {
          out_file << line << endl;
        }
        out_file.close();
      }
      in_file.close();
    }

    Environment * e = new Environment(&randVar);

    pid_t child, changed_child;
    vector<pid_t> children;
    int status;




    std::vector<unsigned int> app_N_list;
    for (unsigned int app_N=0; app_N<=10; app_N += 2) app_N_list.push_back(app_N);



    unsigned int experiment_number =
        arch.FRI_list.size() *
        arch.SPEEDUP_list.size() *
        arch.U_SW_list.size() *
        arch.U_HW_list.size() *
        arch.A_TOT_list.size() *
        arch.TASK_APPENDED_list.size() *
        arch.runs *
        app_N_list.size();


    runStarted(curDir, experiment_number);

    for (unsigned int i=0; i<arch.FRI_list.size(); ++i) {

      arch.FRI = arch.FRI_list.at(i);

      string friDir = curDir + "FRI_" + to_string(arch.FRI) + "/";
      boost::filesystem::create_directories(friDir);

      writeConfigurationToFile(friDir, arch);

      for (unsigned int a=0; a<arch.A_TOT_list.size(); ++a) {

        arch.A_TOT = arch.A_TOT_list.at(a);

        string areaDir = friDir + "AREA_" + to_string(arch.A_TOT) + "/";
        boost::filesystem::create_directories(areaDir);

        for (unsigned int taskApp=0; taskApp<arch.TASK_APPENDED_list.size(); ++taskApp) {

          arch.TASK_APPENDED = arch.TASK_APPENDED_list.at(taskApp);

          string taskAppDir = areaDir + "TASK_APPENDED_" + to_string(arch.TASK_APPENDED) + "/";
          boost::filesystem::create_directories(taskAppDir);

          for (unsigned int s=0; s<arch.SPEEDUP_list.size(); ++s) {

            arch.SPEEDUP = arch.SPEEDUP_list.at(s);

            string speedupDir = taskAppDir + "SPEEDUP_" + to_string(arch.SPEEDUP) + "/";
            boost::filesystem::create_directories(speedupDir);

            for (unsigned int sw=0; sw<arch.U_SW_list.size(); ++sw) {

              arch.U_SW = arch.U_SW_list.at(sw);

              string u_SW_Dir = speedupDir + "U_SW_" + to_string(arch.U_SW) + "/";
              boost::filesystem::create_directories(u_SW_Dir);

              for (unsigned int j=0; j<arch.U_HW_list.size(); ++j) {

                arch.U_HW = arch.U_HW_list.at(j);

                string valDir = u_SW_Dir + "U_HW_" + to_string(arch.U_HW) + "/";
                boost::filesystem::create_directories(valDir);

#if 1

                for (unsigned int i=0; i<arch.runs; ++i) {

                  string runDir = valDir + "/" + to_string(i) + "/";
                  boost::filesystem::create_directories(runDir);

                  Environment_details_t ed = generateEnvironment(arch);
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

#else

                Environment_details_t ed = generateEnvironment(arch);

                for (unsigned int app_Ni=0; app_Ni<app_N_list.size(); ++app_Ni) {
                  unsigned int app_N = app_N_list.at(app_Ni);


                  Environment_details_t ed_2;

                  extraTasks_details_t extra_details;
                  extra_details.N = app_N;
                  extra_details.U_SW = 0.1;
                  extra_details.U_HW = 0.1;

                  ed_2 = environmentAddTask(ed, arch, extra_details);


                  string app_NDir = valDir + "app_N_" + to_string(app_N)+ "/";
                  boost::filesystem::create_directories(app_NDir);

                  for (unsigned int i=0; i<arch.runs; ++i) {

                    string runDir = app_NDir + "/" + to_string(i) + "/";
                    boost::filesystem::create_directories(runDir);

                    e->build(ed_2);
                    e->environmentToFile(runDir);

                    if ((child = fork()) == 0) {
                      // Child process

                      FRED::FRED_config_t fct;

                      if (arch.FRI == TB_PREEMPTIVE_B)
                        fct = FRED::PREEMPTIVE_FRI;
                      if (arch.FRI == TB_NONPREEMPTIVE_B)
                        fct = FRED::NON_PREEMPTIVE_FRI;
                      if (arch.FRI == FPGA_STATIC_B)
                        fct = FRED::STATIC;

                      SS_taskset_t SStaskset = convertSimplifiedFRED_to_SStaskset(ed_2, fct);
                      //SStaskset[2] = convertSimplifiedFRED_to_SStaskset(ed, FRED::STATIC);

                      SS_Task_RTA analysis(SStaskset);

                      e->addAnalysisResults(analysis.isSchedulable());

#ifdef RUN_DURATION
                      SIMUL.run(RUN_DURATION);
#endif
#ifdef RUN_PERIOD_TIMES
                      SIMUL.run(max_T(ed_2) * RUN_PERIOD_TIMES);
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

#endif





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
