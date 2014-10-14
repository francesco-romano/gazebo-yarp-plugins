/*
 * Copyright (C) 2007-2013 Istituto Italiano di Tecnologia ADVR & iCub Facility
 * Authors: Enrico Mingo, Alessio Rocchi, Mirko Ferrati, Silvio Traversaro and Alessandro Settimi
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#ifndef GAZEBOYARP_HANDLER_HH
#define GAZEBOYARP_HANDLER_HH

#include <map>
#include <yarp/os/Semaphore.h>
#include <boost/shared_ptr.hpp>

namespace gazebo
{
    namespace sensors {
        class Sensor;
        typedef boost::shared_ptr<Sensor> SensorPtr;
    }
    namespace physics {
        class Model;
        typedef boost::shared_ptr<Model> ModelPtr;
    }
}

namespace sdf {
    class Element;
    typedef boost::shared_ptr<Element> ElementPtr;
}

namespace yarp {
    namespace os {
        class Property;
    }
}

namespace GazeboYarpPlugins {

/** \class Handler
 *
 * Singleton object class. You can use this class to save robots and sensors and retrieve it in other part of the application
 */
class Handler
{
public:
    /** Returns the singleton intance of Handler class and creates it if it does not exist.
     * \return the singleton handler
     */
    static Handler* getHandler();

    /** \brief Adds a new modelPointer to the "database".
     *
     * If it already exists and the pointer are the same return success,
     * if pointers doesn't match returns error.
     * \param _model the model to be added to the internal database
     * \return true if successfully added, or the model already exists. False otherwise.
     */
    bool setRobot(gazebo::physics::Model* _model);

    /** Returns the pointer to the model matching the robot name
     * \param robotName robot name to be looked for
     * \return the model matching the passed name
     */
    gazebo::physics::Model* getRobot(const std::string& robotName) const;

    /** \brief Removes a robot from the internal database
     *  \param robotName the name of the robot to be removed
     */
    void removeRobot(const std::string& robotName);

    /** \brief Adds a new sensorPointer to the "database".
     *
     *  If the sensor already exists and the pointer are the same return success, if pointers doesn't match returns error.
     * \param _sensor the sensor to be added to the internal database
     * \return true if successfully added, or the model already exists. False otherwise.
     */
    bool setSensor(gazebo::sensors::Sensor* _sensor);

    /** Returns the  pointer to the sensor matching the sensor name
     * \param sensorScopedName sensor name to be looked for
     * \return the sensor matching the passed name
     */
    gazebo::sensors::Sensor* getSensor(const std::string& sensorScopedName) const;

    /** \brief Removes a sensor from the internal database
     *  \param sensorScopedName the name of the sensor to be removed
     */
    void removeSensor(const std::string& sensorName);

    /** Destructor
     */
    ~Handler();

private:

    template <class T>
    class ReferenceCountingObject
    {
        T m_object;
        unsigned short m_count;
    public:
        ReferenceCountingObject(T object): m_object(object), m_count(1) {}

        T object() const { return m_object; }
        unsigned short count() const { return m_count; }
        void incrementCount() { m_count++; }
        void decrementCount() { m_count--; }
    };

    typedef ReferenceCountingObject<gazebo::physics::Model*> ReferenceCountingModel;
    typedef ReferenceCountingObject<gazebo::sensors::Sensor*> ReferenceCountingSensor;

    typedef std::map<std::string, ReferenceCountingModel> RobotsMap;
    typedef std::map<std::string, ReferenceCountingSensor> SensorsMap;

    // singleton stuff
    static yarp::os::Semaphore s_mutex;
    static Handler* s_handle;

    Handler();
    RobotsMap m_robotMap;      // map of known robots
    SensorsMap m_sensorsMap;    // map of known sensors

    bool findRobotName(sdf::ElementPtr sdf, std::string* robotName);

};

/**
 * Add some Gazebo specific "enviromental variables" that it is possible
 * to expand in .ini configuration file loaded using the ${variable} syntax,
 * for writing configuration files where port names and other parameters
 * depend on Gazebo names.
 *
 * This function add some model related "enviromental variables",
 * copying their values from some ModelPtr methods :
 *
 * |  Yarp parameter name       | ModelPtr method    |
 * |:--------------------------:|:------------------:|
 * | gazeboYarpPluginsRobotName | model->GetName()   |
 *
 * @return true if all went well, false otherwise
 */
bool addGazeboEnviromentalVariablesModel(gazebo::physics::ModelPtr _parent,
                                         sdf::ElementPtr _sdf,
                                         yarp::os::Property & plugin_parameters);

/**
 * Add some Gazebo specific "enviromental variables" that it is possible
 * to expand in .ini configuration file loaded using the ${variable} syntax,
 * for writing configuration files where port names and other parameters
 * depend on Gazebo names.
 *
 * This function add some Sensor related "enviromental variables",
 * copying their values from some SensorPtr methods :
 *
 * |  Yarp parameter name        | SensorPtr method    |
 * |:---------------------------:|:-------------------:|
 * | gazeboYarpPluginsSensorName | sensor->GetName()   |
 *
 * @return true if all went well, false otherwise
 */
bool addGazeboEnviromentalVariablesSensor(gazebo::sensors::SensorPtr _sensor,
                                          sdf::ElementPtr _sdf,
                                          yarp::os::Property & plugin_parameters);



}

#endif  // GAZEBOYARP_HANDLER_HH
