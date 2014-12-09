/*
 * pWTStatusView
 *        File: WTStatusView.cpp
 *  Created on: Sep 11, 2013
 *      Author: Josh Leighton
 */

#include <iterator>
#include <sstream>
#include "MBUtils.h"
#include "WTStatusView.h"
#include <Wt/WCssStyleSheet>

using namespace Wt;
using namespace std;

boost::mutex data_mutex;
std::vector<std::string> vnames;
std::map<std::string, double> report_ages;
std::map<std::string, ProtoNodeReport> data;

const int NUM_ROWS = 14;

const int AGE_ROW = 2;
const int TYPE_ROW = 1;
const int VOLTAGE_ROW = 3;
const int BATT_PERCENT_ROW = 4;
const int GPS_QUALITY_ROW = 5;
const int ACOMMS_DRIVER_STATUS_ROW = 6;
const int HELM_STATE_ROW = 7;
const int ACTIVE_BEHAVIORS_ROW = 8;
const int RADIO_STATE_ROW = 9;
const int NSF_POWER_LEVEL_ROW = 10;
const int THRUST_LIMIT_ROW = 11;
const int CPU_MEM_ROW = 12;
const int ERRORS_ROW = 13;

//---------------------------------------------------------
// Constructor

StatusViewApplication::StatusViewApplication(const WEnvironment& env) :
        WApplication(env) {
    setTitle("Status View");

    styleSheet().addRule(".StatusView table, td, th",
            "border: 2px solid #DDD; margin-top: 20px; margin-bottom: 20px; padding: 4px 5px;");
    styleSheet().addRule(".StatusView .table-bordered",
            "border-left: 4px; border-radius: 4px; table-layout: fixed;");
    styleSheet().addRule(".StatusView .col0",
            "font-weight: bold; text-align: left");
    styleSheet().addRule(".StatusView .center", "text-align: center");
    styleSheet().addRule(".StatusView .green", "background-color: #58FA58;");
    styleSheet().addRule(".StatusView .blue", "background-color: #00FFFF;");
    styleSheet().addRule(".StatusView .yellow", "background-color: #F7FE2E;");
    styleSheet().addRule(".StatusView .red", "background-color: #FE2E2E;");
    styleSheet().addRule(".StatusView .greyout", "background-color: #E4E4E4;");

    reDraw(0);
    current_num_vehicles = -1;
    iterations = 0;

    WTimer *timer = new WTimer();
    timer->setInterval(1000);
    timer->timeout().connect(this, &StatusViewApplication::update);
    timer->start();
}

void StatusViewApplication::reDraw(int num_vehicles) {
    root()->clear();
    tableTexts.clear();

    container_ = new WContainerWidget();
    container_->setStyleClass("StatusView");
    table = new WTable(container_);
    table->setHeaderCount(1);
    table->setStyleClass("table table-bordered");
    table->columnAt(0)->setWidth(100);
    for (int i = 1; i <= num_vehicles; i++) {
        table->columnAt(i)->setWidth(150);
    }

    for (int column = 0; column < num_vehicles + 1; column++) {
        for (int row = 0; row < NUM_ROWS; row++) {
            WText *t = new WText();
            if (column == 0 && row != 0)
                t->setStyleClass("col0");
            else if (column != 0)
                t->setStyleClass("center");
            tableTexts[pair<int, int>(row, column)] = t;
            table->elementAt(row, column)->addWidget(t);
//            grid_->addWidget(t, row, column);
        }
    }

    tableTexts[pair<int, int>(AGE_ROW, 0)]->setText("Age");
    tableTexts[pair<int, int>(TYPE_ROW, 0)]->setText("Type");
    tableTexts[pair<int, int>(VOLTAGE_ROW, 0)]->setText("Voltage");
    tableTexts[pair<int, int>(BATT_PERCENT_ROW, 0)]->setText("Batt. %");
    tableTexts[pair<int, int>(GPS_QUALITY_ROW, 0)]->setText("GPS Quality");
    tableTexts[pair<int, int>(ACOMMS_DRIVER_STATUS_ROW, 0)]->setText(
            "Acomms Status");
    tableTexts[pair<int, int>(HELM_STATE_ROW, 0)]->setText("Helm State");
    tableTexts[pair<int, int>(ACTIVE_BEHAVIORS_ROW, 0)]->setText(
            "Active Behaviors");
    tableTexts[pair<int, int>(RADIO_STATE_ROW, 0)]->setText("Radio Mode");
    tableTexts[pair<int, int>(NSF_POWER_LEVEL_ROW, 0)]->setText(
            "NSF Power Level");
    tableTexts[pair<int, int>(THRUST_LIMIT_ROW, 0)]->setText("Thrust");
    tableTexts[pair<int, int>(CPU_MEM_ROW, 0)]->setText("cpu% / mem%");
    tableTexts[pair<int, int>(ERRORS_ROW, 0)]->setText("Other Errors");

    for (int col = 0; col < num_vehicles; col++) {
        tableTexts[pair<int, int>(0, col + 1)]->setText(vnames[col]);
    }

    root()->addWidget(container_);
}

void StatusViewApplication::update() {
    boost::mutex::scoped_lock lock(data_mutex);
    if (data.size() != current_num_vehicles) {
        current_num_vehicles = data.size();
        reDraw(current_num_vehicles);
    }

//    reDraw(current_num_vehicles);

    std::stringstream ss;
    ss << iterations;
    tableTexts[pair<int, int>(0, 0)]->setText(ss.str());
    iterations++;

    // loop over all vehicles
    for (int col = 0; col < current_num_vehicles; col++) {
        string vname = vnames[col];
        ProtoNodeReport_PlatformTypeEnum type = data[vname].platform_type();

        // ---------------- age -------------------
        ss.str("");
        double age = (MOOSTime() - data[vname].time_stamp());
        ss << fixed << setprecision(2) << age;
        tableTexts[pair<int, int>(AGE_ROW, col + 1)]->setText(ss.str());
        if (age < 3)
            table->elementAt(AGE_ROW, col + 1)->setStyleClass("green center");
        else if (age > 10)
            table->elementAt(AGE_ROW, col + 1)->setStyleClass("red center");
        else
            table->elementAt(AGE_ROW, col + 1)->setStyleClass("yellow center");

        // ---------------- type -------------------
        ss.str("");
        switch (type) {
        case ProtoNodeReport_PlatformTypeEnum_KAYAK:
            tableTexts[pair<int, int>(TYPE_ROW, col + 1)]->setText("kayak");
            break;
        case ProtoNodeReport_PlatformTypeEnum_REMUS:
            tableTexts[pair<int, int>(TYPE_ROW, col + 1)]->setText("remus");
            break;
        case ProtoNodeReport_PlatformTypeEnum_NSF:
            tableTexts[pair<int, int>(TYPE_ROW, col + 1)]->setText("nsf");
            break;
        case ProtoNodeReport_PlatformTypeEnum_ICARUS:
            tableTexts[pair<int, int>(TYPE_ROW, col + 1)]->setText("icarus");
            break;
        }
        table->elementAt(TYPE_ROW, col + 1)->setStyleClass("center");

        // ---------------- voltage -------------------
        switch (type) {
        case ProtoNodeReport_PlatformTypeEnum_NSF:
            if (data[vname].nsf_power() == ProtoNodeReport_NSFPowerEnum_OKAY) {
                tableTexts[pair<int, int>(VOLTAGE_ROW, col + 1)]->setText(
                        "okay");
                table->elementAt(VOLTAGE_ROW, col + 1)->setStyleClass(
                        "green center");
            } else {
                tableTexts[pair<int, int>(VOLTAGE_ROW, col + 1)]->setText(
                        "low");
                table->elementAt(VOLTAGE_ROW, col + 1)->setStyleClass(
                        "yellow center");
            }
            break;

        case ProtoNodeReport_PlatformTypeEnum_KAYAK:
        case ProtoNodeReport_PlatformTypeEnum_ICARUS:
            ss.str("");
            ss << fixed << setprecision(1);
            double volts = data[vname].voltage();
            ss << volts;
            tableTexts[pair<int, int>(VOLTAGE_ROW, col + 1)]->setText(ss.str());
            if (volts >= 12.2)
                table->elementAt(VOLTAGE_ROW, col + 1)->setStyleClass(
                        "green center");
            else if (volts >= 11.7)
                table->elementAt(VOLTAGE_ROW, col + 1)->setStyleClass(
                        "yellow center");
            else
                table->elementAt(VOLTAGE_ROW, col + 1)->setStyleClass(
                        "red center");
            break;
        }

        // ---------------- helm state -------------------
        string helm_state;
        switch (type) {
        case ProtoNodeReport_PlatformTypeEnum_KAYAK:
            switch (data[vname].helm_state()) {
            case ProtoNodeReport_HelmStateEnum_DRIVE:
                helm_state = "DRIVE";
                table->elementAt(HELM_STATE_ROW, col + 1)->setStyleClass(
                        "green center");
                break;
            case ProtoNodeReport_HelmStateEnum_PARK:
                helm_state = "PARK";
                table->elementAt(HELM_STATE_ROW, col + 1)->setStyleClass(
                        "yellow center");
                break;
            case ProtoNodeReport_HelmStateEnum_MISSING:
                helm_state = "MISSING";
                table->elementAt(HELM_STATE_ROW, col + 1)->setStyleClass(
                        "red center");
                break;
            case ProtoNodeReport_HelmStateEnum_MALCONFIG:
                helm_state = "MALCONFIG";
                table->elementAt(HELM_STATE_ROW, col + 1)->setStyleClass(
                        "red center");
                break;
            default:
                helm_state = "";
                break;
            }
            tableTexts[pair<int, int>(HELM_STATE_ROW, col + 1)]->setText(
                    helm_state);
            break;

        default:
            table->elementAt(HELM_STATE_ROW, col + 1)->setStyleClass("greyout");
            break;
        }

        // ---------------- active behaviors -------------------
        ss.str("");
        for (int i = 0; i < data[vname].active_behaviors_size(); i++) {
            ss << data[vname].active_behaviors(i);
            if (i < data[vname].active_behaviors_size() - 1)
                ss << endl;
        }
        tableTexts[pair<int, int>(ACTIVE_BEHAVIORS_ROW, col + 1)]->setText(
                ss.str());
        table->elementAt(ACTIVE_BEHAVIORS_ROW, col + 1)->setStyleClass(
                "center");

        // ---------------- acomms driver status -------------------
        string acomms_running;
        switch (data[vname].acomms_status()) {
        case ProtoNodeReport_AcommsStatusEnum_READY:
            acomms_running = "ready";
            table->elementAt(ACOMMS_DRIVER_STATUS_ROW, col + 1)->setStyleClass(
                    "green center");
            break;
        case ProtoNodeReport_AcommsStatusEnum_TRANSMITTING:
            acomms_running = "transmitting";
            table->elementAt(ACOMMS_DRIVER_STATUS_ROW, col + 1)->setStyleClass(
                    "blue center");
            break;
        case ProtoNodeReport_AcommsStatusEnum_RECEIVING:
            acomms_running = "receiving";
            table->elementAt(ACOMMS_DRIVER_STATUS_ROW, col + 1)->setStyleClass(
                    "blue center");
            break;
        case ProtoNodeReport_AcommsStatusEnum_NOT_RUNNING:
            acomms_running = "not running";
            table->elementAt(ACOMMS_DRIVER_STATUS_ROW, col + 1)->setStyleClass(
                    "yellow center");
            break;
        case ProtoNodeReport_AcommsStatusEnum_OFFLINE:
            acomms_running = "offline";
            table->elementAt(ACOMMS_DRIVER_STATUS_ROW, col + 1)->setStyleClass(
                    "red center");
            break;
        default:
            break;
        }
        tableTexts[pair<int, int>(ACOMMS_DRIVER_STATUS_ROW, col + 1)]->setText(
                acomms_running);

        // ---------------- gps quality -------------------
        string gps_quality;
        switch (data[vname].gps_quality()) {
        case ProtoNodeReport_GPSQualityEnum_FIX:
            gps_quality = "rtk fix";
            table->elementAt(GPS_QUALITY_ROW, col + 1)->setStyleClass(
                    "green center");
            break;
        case ProtoNodeReport_GPSQualityEnum_FLOAT:
            gps_quality = "rtk float";
            table->elementAt(GPS_QUALITY_ROW, col + 1)->setStyleClass(
                    "yellow center");
            break;
        case ProtoNodeReport_GPSQualityEnum_SINGLE:
            gps_quality = "rtk single";
            table->elementAt(GPS_QUALITY_ROW, col + 1)->setStyleClass(
                    "yellow center");
            break;
        case ProtoNodeReport_GPSQualityEnum_INTERNAL:
            gps_quality = "internal";
            table->elementAt(GPS_QUALITY_ROW, col + 1)->setStyleClass(
                    "yellow center");
            break;
        case ProtoNodeReport_GPSQualityEnum_NO_GPS:
            gps_quality = "unavailable";
            table->elementAt(GPS_QUALITY_ROW, col + 1)->setStyleClass(
                    "red center");
            break;
        case ProtoNodeReport_GPSQualityEnum_NO_MANAGER:
            gps_quality = "no manager";
            table->elementAt(GPS_QUALITY_ROW, col + 1)->setStyleClass(
                    "red center");
            break;
        case ProtoNodeReport_GPSQualityEnum_EXPERIMENT:
            gps_quality = "experiment";
            table->elementAt(GPS_QUALITY_ROW, col + 1)->setStyleClass(
                    "blue center");
            break;
        default:
            break;
        }
        tableTexts[pair<int, int>(GPS_QUALITY_ROW, col + 1)]->setText(
                gps_quality);

        // ---------------- radio mode -------------------
        if (type == ProtoNodeReport_PlatformTypeEnum_KAYAK) {
            string radio_mode;
            switch (data[vname].radio_state()) {
            case ProtoNodeReport_RadioStateEnum_BULLET_LOCKED:
                radio_mode = "bullet";
                table->elementAt(RADIO_STATE_ROW, col + 1)->setStyleClass(
                        "green center");
                break;
            case ProtoNodeReport_RadioStateEnum_BULLET_UNLOCKED:
                radio_mode = "bullet";
                table->elementAt(RADIO_STATE_ROW, col + 1)->setStyleClass(
                        "yellow center");
                break;
            case ProtoNodeReport_RadioStateEnum_FREEWAVE_LOCKED:
                radio_mode = "freewave";
                table->elementAt(RADIO_STATE_ROW, col + 1)->setStyleClass(
                        "green center");
                break;
            case ProtoNodeReport_RadioStateEnum_FREEWAVE_UNLOCKED:
                radio_mode = "freewave";
                table->elementAt(RADIO_STATE_ROW, col + 1)->setStyleClass(
                        "yellow center");
                break;
            default:
                break;
            }
            tableTexts[pair<int, int>(RADIO_STATE_ROW, col + 1)]->setText(
                    radio_mode);
        } else {
            table->elementAt(RADIO_STATE_ROW, col + 1)->setStyleClass(
                    "greyout");
        }

        // ---------------- nsf power level -------------------
        if (type == ProtoNodeReport_PlatformTypeEnum_NSF) {
            stringstream nsf_power_level;
            nsf_power_level << data[vname].nsf_power_level();
            tableTexts[pair<int, int>(NSF_POWER_LEVEL_ROW, col + 1)]->setText(
                    nsf_power_level.str());
            table->elementAt(NSF_POWER_LEVEL_ROW, col + 1)->setStyleClass(
                    "center");
        } else {
            table->elementAt(NSF_POWER_LEVEL_ROW, col + 1)->setStyleClass(
                    "greyout");
        }

        // ---------------- thrust limit -------------------
        if (type == ProtoNodeReport_PlatformTypeEnum_KAYAK) {
            stringstream kayak_thrust;
            kayak_thrust << data[vname].thrust() <<
                    "/" << data[vname].thrust_limit();
            tableTexts[pair<int, int>(THRUST_LIMIT_ROW, col + 1)]->setText(
                    kayak_thrust.str());
            if (data[vname].thrust_limit() == 1000) {
                table->elementAt(THRUST_LIMIT_ROW, col + 1)->setStyleClass(
                        "green center");
            } else if (data[vname].thrust_limit() == 0) {
                table->elementAt(THRUST_LIMIT_ROW, col + 1)->setStyleClass(
                        "red center");
            } else {
                table->elementAt(THRUST_LIMIT_ROW, col + 1)->setStyleClass(
                        "yellow center");
            }
        } else {
            table->elementAt(THRUST_LIMIT_ROW, col + 1)->setStyleClass(
                    "greyout");
        }

        // ---------------- resource monitor -------------------
        stringstream cpu_mem_usage;
        int cpu = data[vname].cpu_percent_use();
        int mem = data[vname].mem_percent_use();
        cpu_mem_usage << cpu << "% / " << mem << "%";
        tableTexts[pair<int, int>(CPU_MEM_ROW, col + 1)]->setText(
                cpu_mem_usage.str());
        if (cpu > 90 || mem > 90) {
            table->elementAt(CPU_MEM_ROW, col + 1)->setStyleClass("red center");
        } else if (cpu > 75 || mem > 75) {
            table->elementAt(CPU_MEM_ROW, col + 1)->setStyleClass(
                    "yellow center");
        } else {
            table->elementAt(CPU_MEM_ROW, col + 1)->setStyleClass(
                    "green center");
        }

        // ---------------- other errors -------------------
        if (data[vname].errors_size() == 0) {
            tableTexts[pair<int, int>(ERRORS_ROW, col + 1)]->setText("none");
            table->elementAt(ERRORS_ROW, col + 1)->setStyleClass(
                    "green center");
        } else {
            string errors;
            for (int i = 0; i < data[vname].errors_size(); i++) {
                switch (data[vname].errors(i)) {
                case ProtoNodeReport_ErrorEnum_NoCompassData:
                    errors += "No compass data\n";
                    break;
                }
            }
            tableTexts[pair<int, int>(ERRORS_ROW, col + 1)]->setText(errors);
            table->elementAt(ERRORS_ROW, col + 1)->setStyleClass("red center");
        }
    }
}

WTStatusView::WTStatusView() {
    wtThread = boost::thread(boost::bind(&WTStatusView::startWT, this));
}

//---------------------------------------------------------
// Destructor

WTStatusView::~WTStatusView() {
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool WTStatusView::OnNewMail(MOOSMSG_LIST &NewMail) {
    boost::mutex::scoped_lock lock(data_mutex);
    MOOSMSG_LIST::iterator p;

    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        std::string key = msg.GetKey();
        if (key == "PROTO_REPORT") {
            ProtoNodeReport pnr;
            if (pnr.ParseFromString(msg.GetString())) {
                data[pnr.vehicle_name()] = pnr;
//                report_ages[pnr.name()] = MOOSTime()-pnr.time();
                if (find(vnames.begin(), vnames.end(), pnr.vehicle_name())
                        == vnames.end())
                    vnames.push_back(pnr.vehicle_name());

                std::cout << pnr.DebugString() << std::endl;
            }
        }

    }

    return (true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool WTStatusView::OnConnectToServer() {
    // register for variables here
    // possibly look at the mission file?
    // m_MissionReader.GetConfigurationParam("Name", <string>);
    // m_Comms.Register("VARNAME", 0);

    RegisterVariables();
    return (true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool WTStatusView::Iterate() {
    return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool WTStatusView::OnStartUp() {
    return (true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void WTStatusView::RegisterVariables() {
    // m_Comms.Register("FOOBAR", 0);
    m_Comms.Register("PROTO_REPORT", 0);
}

WApplication *createApplication(const WEnvironment& env) {
    /*
     * You could read information from the environment to decide whether
     * the user has permission to start a new application
     */
    return new StatusViewApplication(env);
}

void WTStatusView::startWT() {
    char title[] = "Status View";
    char docroot[] = "--docroot=.";
    char http_address[] = "--http-address=0.0.0.0";
    char http_port[] = "--http-port=8080";
    char *myArgv[] = { title, docroot, http_address, http_port };

    WRun(4, myArgv, &createApplication);
}
