/*
 * Copyright (C) 2007-2013 Istituto Italiano di Tecnologia ADVR & iCub Facility
 * Authors: Enrico Mingo, Alessio Rocchi, Mirko Ferrati, Silvio Traversaro, Alberto Cardellino and Alessandro Settimi
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#include "Handler.hh"

#include <gazebo/physics/Entity.hh>
#include <gazebo/sensors/sensors.hh>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/Wrapper.h>
#include <yarp/dev/GenericSensorInterfaces.h>
using namespace gazebo;

namespace GazeboYarpPlugins {

Handler* Handler::s_handle = NULL;
boost::mutex robotmap_mtx;

void close_thread(yarp::dev::PolyDriver* driver)
{
    driver->close();
}

void close_thread_imu(yarp::dev::PolyDriver* driver,std::string m_sensorName)
{
    yarp::dev::IGenericSensor* iDeviceDriver;
    driver->view(iDeviceDriver);
    iDeviceDriver->calibrate(-1,1);//HACK: This will avoid gazebo Update from calling the IMU update in the next step, in order to avoid a segfault

    driver->close();//this will block until gazebo next step Update is done
    GazeboYarpPlugins::Handler::getHandler()->removeSensor(m_sensorName);
}

void detach_thread(yarp::dev::IMultipleWrapper* wrapper,yarp::dev::PolyDriver* driver)
{
    wrapper->detachAll();
    driver->close();
}

yarp::os::Semaphore& Handler::mutex()
{
    static yarp::os::Semaphore s_mutex(1);
    return s_mutex;
}
    
Handler::Handler()
{
    m_robotMap.clear();
    m_sensorsMap.clear();
}

Handler* Handler::getHandler()
{
    mutex().wait();
    if (!s_handle) {
        std::cout << "Calling GazeboYarpPlugins::Handler Constructor" << std::endl;
        s_handle = new Handler();
        if (!s_handle)
            std::cout << "Error while calling GazeboYarpPluginHandler constructor" << std::endl;
    }
    mutex().post();

    return s_handle;
}

void Handler::asyncCloseThisPolydriver(yarp::dev::PolyDriver* driver)
{
    boost::thread* serverThread= new boost::thread(boost::bind(&close_thread,driver));
}

void Handler::asyncCloseThisPolydriver(yarp::dev::PolyDriver* driver, std::string m_sensor_name)
{
    boost::thread* serverThread= new boost::thread(boost::bind(&close_thread_imu,driver,m_sensor_name));
}

void Handler::asyncDetachAll(yarp::dev::IMultipleWrapper* wrapper, yarp::dev::PolyDriver* driver)
{
    boost::thread* serverThread= new boost::thread(boost::bind(&detach_thread,wrapper,driver));
}

bool Handler::setRobot(gazebo::physics::Model* _model)
{
    bool ret = false;
    std::string scopedRobotName = _model->GetScopedName();
    std::cout << "GazeboYarpPlugins::Handler: Inserting Robot : " << scopedRobotName << std::endl;
    robotmap_mtx.lock();
    RobotsMap::iterator robot = m_robotMap.find(scopedRobotName);
    if (robot != m_robotMap.end()) {
        //robot already exists. Increment reference counting
        robot->second.incrementCount();
        std::cout << "Robot already registered, pointers match." << std::endl;
        ret = true;
    }
    else {
        //robot does not exists. Add to map
        ReferenceCountingModel model(_model);
        if (!m_robotMap.insert(std::pair<std::string, ReferenceCountingModel>(scopedRobotName, model)).second) {
            std::cout << "Error in GazeboYarpPlugins::Handler while inserting a new sensor pointer!" << std::endl;
            std::cout << " The name of the sensor is already present but the pointer does not match with the one already registered!" << std::endl;
            std::cout << " This should not happen, as the scoped name should be unique in Gazebo. Fatal error." << std::endl;
            ret = false;
        } else {
            ret = true;
            std::cout << "Singleton: Added a new robot " << scopedRobotName << "." << std::endl;
        }
    }
    robotmap_mtx.unlock();
    return ret;
}

gazebo::physics::Model* Handler::getRobot(const std::string& robotName) const
{
    gazebo::physics::Model* tmp = NULL;
    std::cout << "Looking for robot : " << robotName << std::endl;
    robotmap_mtx.lock();
    RobotsMap::const_iterator robot = m_robotMap.find(robotName);
    if (robot != m_robotMap.end()) {
        std::cout << "Robot " << robotName << " was happily found!" << std::endl;
        tmp = robot->second.object();
    }
    else {
        std::cout << "Robot was not found: " << robotName << std::endl;
        tmp = NULL;
    }
    robotmap_mtx.unlock();
    return tmp;
}

void Handler::removeRobot(const std::string& robotName)
{
    robotmap_mtx.lock();
    RobotsMap::iterator robot = m_robotMap.find(robotName);
    if (robot != m_robotMap.end()) {
        robot->second.decrementCount();
        if (!robot->second.count()) {
            std::cout << "Removing robot " << robotName << std::endl;
            m_robotMap.erase(robot);
        }
    } else {
        std::cout << "Could not remove robot " << robotName << ". Robot was not found" << std::endl;
    }
    robotmap_mtx.unlock();
}

bool Handler::setSensor(gazebo::sensors::Sensor* _sensor)
{
    bool ret = false;
    std::string scopedSensorName = _sensor->GetScopedName();
    std::cout << "GazeboYarpPlugins::Handler: Inserting Sensor : " << scopedSensorName << std::endl;
    
    SensorsMap::iterator sensor = m_sensorsMap.find(scopedSensorName);
    if (sensor != m_sensorsMap.end()) {
        //sensor already exists. Increment reference counting
        sensor->second.incrementCount();
        std::cout << "Sensor already registered, pointers match." << std::endl;
        ret = true;
    } else {
        //sensor does not exists. Add to map
        ReferenceCountingSensor countedSensor(_sensor);
        if (!m_sensorsMap.insert(std::pair<std::string, ReferenceCountingSensor>(scopedSensorName, countedSensor)).second) {
            std::cout << "Error in GazeboYarpPlugins::Handler while inserting a new sensor pointer!" << std::endl;
            std::cout << " The name of the sensor is already present but the pointer does not match with the one already registered!" << std::endl;
            std::cout << " This should not happen, as the scoped name should be unique in Gazebo. Fatal error." << std::endl;
            ret = false;
        } else {
            ret = true;
            std::cout << "Singleton: Added a new sensor " << scopedSensorName << "." << std::endl;
        }
    }
    return ret;
}
    
// return the sensor pointer given the sensor scoped namespac
gazebo::sensors::Sensor* Handler::getSensor(const std::string& sensorScopedName) const
{
    gazebo::sensors::Sensor* tmp = NULL;
    std::cout << "Looking for sensor : " << sensorScopedName << std::endl;
    
    SensorsMap::const_iterator sensor = m_sensorsMap.find(sensorScopedName);
    if (sensor != m_sensorsMap.end()) {
        std::cout << "Sensor " << sensorScopedName << " was happily found!" << std::endl;
        tmp = sensor->second.object();
    } else {
        std::cout << "Sensor was not found: " << sensorScopedName << std::endl;
        tmp = NULL;
    }
    return tmp;
}

void Handler::removeSensor(const std::string& sensorName)
{
    robotmap_mtx.lock();
    SensorsMap::iterator sensor = m_sensorsMap.find(sensorName);
    if (sensor != m_sensorsMap.end()) {
        sensor->second.decrementCount();
        if (!sensor->second.count()) {
            std::cout << "Removing sensor " << sensorName << std::endl;
            m_sensorsMap.erase(sensor);
        }
    } else {
        std::cout << "Could not remove sensor " << sensorName << ". Sensor was not found" << std::endl;
    }
    robotmap_mtx.unlock();
}
}
